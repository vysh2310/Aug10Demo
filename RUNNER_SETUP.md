# Self-Hosted Runner Setup

This repository's PR gate requires a Windows self-hosted GitHub Actions runner with the default labels `self-hosted`, `windows`, and `X64`.

## Required software

- Git
- `g++`
- `mingw32-make`
- Polyspace R2026a or later with:
  - `polyspace-as-you-code.exe`
  - `polyspace-configure.exe`
  - `polyspace-results-export.exe`
  - `polyspace\pstest\pstunit\src\pstunit.c`

On the current development machine, the expected tool locations are:

```text
git.exe                      C:\Program Files\Git\cmd\git.exe
g++.exe                      C:\ProgramData\MATLAB\SupportPackages\R2026a\3P.instrset\mingw_w64.instrset\bin\g++.exe
mingw32-make.exe             C:\ProgramData\MATLAB\SupportPackages\R2026a\3P.instrset\mingw_w64.instrset\bin\mingw32-make.exe
polyspace-as-you-code.exe    C:\Program Files\Polyspace\R2026a\polyspace\bin\polyspace-as-you-code.exe
polyspace-configure.exe      C:\Program Files\Polyspace\R2026a\polyspace\bin\polyspace-configure.exe
polyspace-results-export.exe C:\Program Files\Polyspace\R2026a\polyspace\bin\polyspace-results-export.exe
```

## Environment

Ensure the runner account has these tools on `PATH`.

Optional but recommended:

```powershell
[System.Environment]::SetEnvironmentVariable('POLYSPACE_ROOT', 'C:\Program Files\Polyspace\R2026a', 'Machine')
```

## Register the runner

In GitHub:

1. Open the repository.
2. Go to `Settings > Actions > Runners`.
3. Click `New self-hosted runner`.
4. Choose `Windows`.
5. Download and extract the runner.
6. Run the GitHub-provided `config.cmd` command.
7. You do not need a custom label for the current workflow. The default labels are enough.
8. Install and start the runner service with the GitHub-provided `run.cmd` or service commands.

## Validate the runner

From the repository root, run:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\validate-runner-prereqs.ps1
```

The PR workflow `Main Merge Gate` will only target runners labeled:

```text
self-hosted, windows, X64
```
