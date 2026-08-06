$ErrorActionPreference = 'Stop'

function Get-RequiredCommandPath {
    param(
        [Parameter(Mandatory = $true)]
        [string[]] $Names
    )

    foreach ($name in $Names) {
        $command = Get-Command $name -ErrorAction SilentlyContinue
        if ($null -ne $command) {
            return $command.Source
        }
    }

    throw ("Required command not found. Tried: {0}" -f ($Names -join ', '))
}

function Get-PolyspaceRoot {
    if ($env:POLYSPACE_ROOT) {
        $candidate = $env:POLYSPACE_ROOT.TrimEnd('\')
        if (Test-Path (Join-Path $candidate 'polyspace\bin\polyspace-as-you-code.exe')) {
            return $candidate
        }
    }

    $payc = Get-Command 'polyspace-as-you-code.exe' -ErrorAction Stop
    $binDir = Split-Path -Parent $payc.Source
    $polyspaceDir = Split-Path -Parent $binDir
    return Split-Path -Parent $polyspaceDir
}

function Invoke-External {
    param(
        [Parameter(Mandatory = $true)]
        [string] $Description,

        [Parameter(Mandatory = $true)]
        [string] $FilePath,

        [Parameter()]
        [string[]] $Arguments = @()
    )

    Write-Host "[gate] $Description"
    & $FilePath @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "$Description failed with exit code $LASTEXITCODE."
    }
}

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Split-Path -Parent $scriptDir
Set-Location $repoRoot

$checkersFile = Join-Path $repoRoot 'media.xml'
if (-not (Test-Path $checkersFile)) {
    throw "Missing Polyspace checker configuration: $checkersFile"
}

$polyspaceRoot = Get-PolyspaceRoot
$polyspaceConfigure = Join-Path $polyspaceRoot 'polyspace\bin\polyspace-configure.exe'
$polyspaceAsYouCode = Join-Path $polyspaceRoot 'polyspace\bin\polyspace-as-you-code.exe'
$polyspaceResultsExport = Join-Path $polyspaceRoot 'polyspace\bin\polyspace-results-export.exe'
$pstunitIncludeDir = Join-Path $polyspaceRoot 'polyspace\pstest\pstunit\include'
$pstunitRuntime = Join-Path $polyspaceRoot 'polyspace\pstest\pstunit\src\pstunit.c'
$gpp = Get-RequiredCommandPath -Names @('g++.exe', 'g++')
$makeExe = Get-RequiredCommandPath -Names @('mingw32-make.exe', 'mingw32-make', 'make.exe', 'make')

$testSource = Join-Path $repoRoot 'car_media_polyspace_demo_pstunit.cpp'
if (-not (Test-Path $testSource)) {
    throw "Missing PSTUnit test source: $testSource"
}

$scratchRoot = Join-Path ([System.IO.Path]::GetTempPath()) ('Aug10Demo_merge_gate_' + [System.Guid]::NewGuid().ToString('N'))
$buildOptionsFile = Join-Path $scratchRoot 'polyspace_build_options.txt'
$testExe = Join-Path $scratchRoot 'car_media_polyspace_demo_pstunit.exe'
New-Item -ItemType Directory -Path $scratchRoot -Force | Out-Null

try {
    $buildCommand = '"' + $makeExe + '" -C "' + $repoRoot + '" all'

    Invoke-External `
        -Description 'Generating Polyspace build options' `
        -FilePath $polyspaceConfigure `
        -Arguments @(
            '-output-options-file',
            $buildOptionsFile,
            '-allow-overwrite',
            '-no-sources',
            'cmd.exe',
            '/c',
            $buildCommand
        )

    Invoke-External `
        -Description 'Building PSTUnit executable' `
        -FilePath $gpp `
        -Arguments @(
            '-std=c++17',
            '-Wall',
            '-Wextra',
            '-O2',
            $testSource,
            $pstunitRuntime,
            "-I$pstunitIncludeDir",
            '-o',
            $testExe
        )

    Invoke-External `
        -Description 'Running PSTUnit tests' `
        -FilePath $testExe

    $sourceFiles = Get-ChildItem -Path $repoRoot -Filter '*.cpp' -File |
        Where-Object { $_.Name -notlike '*_pstunit.cpp' } |
        Sort-Object Name

    if ($sourceFiles.Count -eq 0) {
        throw 'No production C++ source files were found.'
    }

    $violations = @()

    foreach ($sourceFile in $sourceFiles) {
        $resultDir = Join-Path $scratchRoot $sourceFile.BaseName
        $sarifFile = Join-Path $scratchRoot ($sourceFile.BaseName + '.sarif')

        New-Item -ItemType Directory -Path $resultDir -Force | Out-Null

        Invoke-External `
            -Description ("Running Polyspace on " + $sourceFile.Name) `
            -FilePath $polyspaceAsYouCode `
            -Arguments @(
                '-sources',
                $sourceFile.FullName,
                '-checkers-activation-file',
                $checkersFile,
                '-options-file',
                $buildOptionsFile,
                '-results-dir',
                $resultDir
            )

        Invoke-External `
            -Description ("Exporting SARIF for " + $sourceFile.Name) `
            -FilePath $polyspaceResultsExport `
            -Arguments @(
                '-format',
                'json-sarif',
                '-results-dir',
                $resultDir,
                '-output-name',
                $sarifFile
            )

        $sarif = Get-Content -Path $sarifFile -Raw | ConvertFrom-Json
        $results = @()

        foreach ($run in @($sarif.runs)) {
            if ($null -ne $run -and $null -ne $run.results) {
                $results += @($run.results)
            }
        }

        if ($results.Count -gt 0) {
            $examples = $results |
                Select-Object -First 5 |
                ForEach-Object {
                    $location = $_.locations[0].physicalLocation
                    '{0}:{1} {2}' -f $sourceFile.Name, $location.region.startLine, $_.ruleId
                }

            $violations += [PSCustomObject]@{
                Source = $sourceFile.Name
                Count = $results.Count
                Examples = $examples
            }
        }
    }

    if ($violations.Count -gt 0) {
        Write-Host '[gate] Static-analysis violations detected:'
        foreach ($violation in $violations) {
            Write-Host ("[gate]   {0}: {1} finding(s)" -f $violation.Source, $violation.Count)
            foreach ($example in $violation.Examples) {
                Write-Host ("[gate]     " + $example)
            }
        }

        throw 'Merge gate failed: all MISRA C++ and SEI CERT C++ findings must be resolved.'
    }

    Write-Host '[gate] Merge gate passed.'
}
finally {
    if (Test-Path $scratchRoot) {
        Remove-Item -LiteralPath $scratchRoot -Recurse -Force
    }
}
