$ErrorActionPreference = 'Stop'

function Get-CommandPath {
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

    throw ("Missing required command. Tried: {0}" -f ($Names -join ', '))
}

function Resolve-PolyspaceRoot {
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

$paths = [ordered]@{
    git = Get-CommandPath -Names @('git.exe', 'git')
    gpp = Get-CommandPath -Names @('g++.exe', 'g++')
    make = Get-CommandPath -Names @('mingw32-make.exe', 'mingw32-make', 'make.exe', 'make')
    polyspace_as_you_code = Get-CommandPath -Names @('polyspace-as-you-code.exe', 'polyspace-as-you-code')
    polyspace_configure = Get-CommandPath -Names @('polyspace-configure.exe', 'polyspace-configure')
    polyspace_results_export = Get-CommandPath -Names @('polyspace-results-export.exe', 'polyspace-results-export')
}

$polyspaceRoot = Resolve-PolyspaceRoot
$pstunitRuntime = Join-Path $polyspaceRoot 'polyspace\pstest\pstunit\src\pstunit.c'
$pstunitInclude = Join-Path $polyspaceRoot 'polyspace\pstest\pstunit\include\pstunit.h'

foreach ($requiredPath in @($pstunitRuntime, $pstunitInclude)) {
    if (-not (Test-Path $requiredPath)) {
        throw "Missing required Polyspace runtime asset: $requiredPath"
    }
}

Write-Host '[runner] Toolchain detected:'
foreach ($entry in $paths.GetEnumerator()) {
    Write-Host ("[runner]   {0}: {1}" -f $entry.Key, $entry.Value)
}
Write-Host ("[runner]   polyspace_root: {0}" -f $polyspaceRoot)
Write-Host '[runner] Prerequisite validation passed.'
