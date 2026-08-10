<#
.SYNOPSIS
    Run the Mixxx behave UI tests on Windows and stream the output.

.DESCRIPTION
    Thin, correctly-quoted wrapper around Start-InteractiveProcessOnWindows.ps1.

    Start-InteractiveProcessOnWindows.ps1 must run the child inside the
    interactive desktop session (a GUI app cannot start from an SSH session),
    so it needs the whole test command as a single -Arguments string. Building
    that string from cmd.exe or bash mangles the single quotes behave needs for
    scenario names, so this script is the supported entry point.

    This script and its companion Start-InteractiveProcessOnWindows.ps1 were
    drafted autonomously by an AI agent and are offered for a human reviewer
    to check, amend and submit.

.PARAMETER Feature
    Feature file(s) to run, relative to the repo root.
    Default: the library settings feature used while bring-up debugging.

.PARAMETER Name
    Scenario name(s) to run (behave -n). Repeat the parameter or use a
    comma separated list. Omit to run the whole feature file.

.PARAMETER Retry
    Behave autoretry attempts per scenario.

.PARAMETER Record
    Record a video of the run into src/test/behave/artifacts.

.PARAMETER Label
    Suffix for the artifact filenames, so parallel runs do not clobber
    each other's behave-output-<label>.txt.

.PARAMETER RepoRoot
    Repository root. Defaults to the parent of this script's tools\ directory.

.PARAMETER FfmpegBin
    ffmpeg executable forwarded to the child as MIXXX_FFMPEG_BIN. Only used
    with -Record. Discovered automatically when omitted; pass an empty string
    to disable the lookup entirely.

.EXAMPLE
    .\tools\Run-BehaveScenariosOnWindows.ps1

.EXAMPLE
    .\tools\Run-BehaveScenariosOnWindows.ps1 -Name 'An empty music directory can be removed'

.EXAMPLE
    .\tools\Run-BehaveScenariosOnWindows.ps1 -Feature features/settings/library.feature -Name 'a','b'
#>
[CmdletBinding()]
param(
    [string[]]$Feature = @("src\test\behave\features\settings\library.feature"),

    [string[]]$Name = @(),

    [int]$Retry = 3,

    [switch]$Record,

    [string]$Label = "",

    [string]$RepoRoot = "",

    [string]$FfmpegBin = "",

    [int]$TimeoutSeconds = 1800
)

$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($RepoRoot)) {
    # This script lives in tools\, so the repository root is its parent. This
    # avoids hardcoding a drive letter, which differs per machine.
    $RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
}

$launcher = Join-Path $RepoRoot "tools\Start-InteractiveProcessOnWindows.ps1"

if (-not (Test-Path $launcher)) {
    throw "Start-InteractiveProcessOnWindows.ps1 not found at $launcher"
}

$python = Join-Path $RepoRoot "src\test\behave\.venv\Scripts\python.exe"

if (-not (Test-Path $python)) {
    throw "Behave venv python not found at $python"
}

$binary = Join-Path $RepoRoot "build\mixxx-test.exe"

if (-not (Test-Path $binary)) {
    throw "mixxx-test binary not found at $binary. Build it before running the UI tests."
}

# The scheduled task does not inherit the caller's environment, so anything the
# behave runner needs has to be forwarded explicitly. This is a no-op for a
# non-recording run.
function Resolve-Ffmpeg {
    if ($FfmpegBin -eq "") {
        return ""
    }

    if (Test-Path $FfmpegBin) {
        return (Resolve-Path $FfmpegBin).Path
    }

    $onPath = Get-Command "ffmpeg.exe" -ErrorAction SilentlyContinue

    if ($onPath) {
        return $onPath.Source
    }

    $buildenv = Get-ChildItem `
        -Path (Join-Path $RepoRoot "buildenv") `
        -Filter "ffmpeg.exe" `
        -Recurse `
        -ErrorAction SilentlyContinue |
        Select-Object -First 1

    if ($buildenv) {
        return $buildenv.FullName
    }

    Write-Warning "ffmpeg not found; -Record will fail. Pass -FfmpegBin to set it explicitly."

    return ""
}

$childEnvironment = @{}

if ($Record) {
    $ffmpeg = Resolve-Ffmpeg

    if ($ffmpeg) {
        $childEnvironment["MIXXX_FFMPEG_BIN"] = $ffmpeg
    }
}

# Build the runner argv as a real array first, then join it. Joining is safe
# because the launcher's wrapper re-quotes each element; the single quotes
# here are what survive into the child command line for scenario names.
$runnerArgs = @(
    "src\test\behave\mixxx_test_runner.py"
    "--no-capture"
    "--retry", $Retry
    "--binary", "build\mixxx-test.exe"
)

if (![string]::IsNullOrWhiteSpace($Label)) {
    $runnerArgs += "--label"
    $runnerArgs += $Label
}

if ($Record) {
    $runnerArgs += "--record"
}

$runnerArgs += $Feature

foreach ($scenarioName in $Name) {
    $runnerArgs += @("-n", "'$scenarioName'")
}

$arguments = $runnerArgs -join " "

Write-Host "Arguments: $arguments"

& $launcher `
    -Program $python `
    -Arguments $arguments `
    -WorkingDirectory $RepoRoot `
    -WaitMode Exit `
    -Environment $childEnvironment `
    -TimeoutSeconds $TimeoutSeconds

exit $LASTEXITCODE
