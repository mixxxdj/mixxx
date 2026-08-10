<#
.SYNOPSIS
    Launch a program in the currently logged-in Windows desktop session.

.DESCRIPTION
    Designed for use over SSH on Windows 10/11.

    Uses an interactive scheduled task to launch GUI applications
    in the physical user's desktop session.

    This script and its companion Run-BehaveScenariosOnWindows.ps1 were drafted
    autonomously by an AI agent and are offered for a human reviewer
    to check, amend and submit.

.PARAMETER Environment
    Environment variables to export into the child process, as a hashtable of
    environment, so anything the child needs must be passed explicitly.
    Defaults to QSG_RHI_BACKEND=opengl, PYTHONUNBUFFERED=1 and
    PYTHONIOENCODING=utf-8. Caller entries override those defaults.


.EXAMPLES

    # Launch GUI application and return once started
    .\tools\Start-InteractiveProcessOnWindows.ps1 `
        -Program "notepad.exe"

    # Launch application with arguments
    .\tools\Start-InteractiveProcessOnWindows.ps1 `
        -Program "D:\dev\mixxx\build\mixxx.exe" `
        -Arguments '--qml --settings-path "D:\dev\mixxx\build\mixxx_dev"' `
        -WorkingDirectory "D:\dev\mixxx"

    # Wait until application exits
    .\tools\Start-InteractiveProcessOnWindows.ps1 `
        -Program "D:\dev\mixxx\build\mixxx.exe" `
        -Arguments '--qml --settings-path "D:\dev\mixxx\build\mixxx_dev"' `
        -WorkingDirectory "D:\dev\mixxx" `
        -WaitMode Exit

    # Pass an environment variable through to the child
    .\tools\Start-InteractiveProcessOnWindows.ps1 `
        -Program "D:\dev\mixxx\build\mixxx.exe" `
        -WaitMode Exit `
        -Environment @{ MIXXX_FFMPEG_BIN = "C:\tools\ffmpeg.exe" }

    # Specify startup timeout
    .\tools\Start-InteractiveProcessOnWindows.ps1 `
        -Program "notepad.exe" `
        -WaitMode Start `
        -TimeoutSeconds 30
#>

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true, Position = 0)]
    [string]$Program,

    [Parameter(Position = 1)]
    [string]$Arguments = "",

    [string]$WorkingDirectory = "",

    [ValidateSet("Start", "Exit")]
    [string]$WaitMode = "Start",

    [ValidateRange(0, 86400)]
    [int]$TimeoutSeconds = 1800,

    [ValidateRange(50, 5000)]
    [int]$PollMilliseconds = 100,

    [hashtable]$Environment = @{}
)

$ErrorActionPreference = "Stop"

# ============================================================================
# Configuration
# ============================================================================

$taskName = "SSH-Interactive-Launcher-$([guid]::NewGuid().ToString())"

$tempDir = Join-Path `
    $env:TEMP `
    "SSHLauncher\$taskName"

New-Item `
    -ItemType Directory `
    -Path $tempDir `
    -Force | Out-Null

$stdoutFile = Join-Path $tempDir "stdout.log"
$stderrFile = Join-Path $tempDir "stderr.log"
$pidFile = Join-Path $tempDir "pid.txt"
$exitCodeFile = Join-Path $tempDir "exitcode.txt"
$doneFile = Join-Path $tempDir "done"
$wrapperFile = Join-Path $tempDir "wrapper.ps1"

# ============================================================================
# Helper Functions
# ============================================================================

function Write-OutputLine {
    param(
        [string]$Message
    )

    Write-Host $Message
}

function Escape-PowerShellString {
    param(
        [string]$Value
    )

    if ($null -eq $Value) {
        return ""
    }

    return $Value.Replace("'", "`"")
}

function Read-NewOutput {
    param(
        [string]$Path,

        [ref]$Offset,

        [switch]$ErrorStream
    )

    if (-not (Test-Path $Path)) {
        return
    }

    try {
        $bytes = [System.IO.File]::ReadAllBytes($Path)

        if ($bytes.Length -le $Offset.Value) {
            return
        }

        $newBytes = $bytes[$Offset.Value..($bytes.Length - 1)]

        $Offset.Value = $bytes.Length

        $text = [System.Text.Encoding]::UTF8.GetString($newBytes)

        if ($ErrorStream) {
            [Console]::Error.Write($text)
        }
        else {
            Write-Host -NoNewline $text
        }
    }
    catch {
        # The file may be temporarily locked while the child process writes.
    }
}

function Cleanup {
    Write-Verbose "Cleaning up task $taskName"

    try {
        Unregister-ScheduledTask `
            -TaskName $taskName `
            -Confirm:$false `
            -ErrorAction SilentlyContinue
    }
    catch {
    }

    try {
        if (Test-Path $tempDir) {
            Remove-Item `
                -Path $tempDir `
                -Recurse `
                -Force `
                -ErrorAction SilentlyContinue
        }
    }
    catch {
    }
}

# ============================================================================
# Main Execution
# ============================================================================

try {

    # ========================================================================
    # Validate Program
    # ========================================================================

    if ([string]::IsNullOrWhiteSpace($Program)) {
        throw "Program cannot be empty."
    }

    # Resolve relative program paths where possible
    if (-not [System.IO.Path]::IsPathRooted($Program)) {

        $command = Get-Command `
            $Program `
            -ErrorAction SilentlyContinue

        if ($command) {
            $Program = $command.Source
        }
        else {
            $candidatePath = Join-Path `
                (Get-Location) `
                $Program

            if (Test-Path $candidatePath) {
                $Program = (Resolve-Path $candidatePath).Path
            }
        }
    }

    Write-Verbose "Program: $Program"
    Write-Verbose "Arguments: $Arguments"
    Write-Verbose "Working Directory: $WorkingDirectory"
    Write-Verbose "Wait Mode: $WaitMode"

    # ========================================================================
    # Detect Interactive Desktop User
    # ========================================================================

    $computerSystem = Get-CimInstance Win32_ComputerSystem

    $userAccount = $computerSystem.UserName

    if ([string]::IsNullOrWhiteSpace($userAccount)) {
        throw "No interactive desktop user is currently logged in."
    }

    Write-Verbose "Interactive user: $userAccount"

    # ========================================================================
    # Determine Working Directory
    # ========================================================================

    if ([string]::IsNullOrWhiteSpace($WorkingDirectory)) {

        # Default to the directory containing the executable if possible.
        if ([System.IO.Path]::IsPathRooted($Program)) {

            $programDirectory = Split-Path `
                -Path $Program `
                -Parent

            if ($programDirectory -and (Test-Path $programDirectory)) {
                $WorkingDirectory = $programDirectory
            }
        }

        # Fallback to user's current directory.
        if ([string]::IsNullOrWhiteSpace($WorkingDirectory)) {
            $WorkingDirectory = (Get-Location).Path
        }
    }

    if (-not (Test-Path $WorkingDirectory)) {
        throw "Working directory does not exist: $WorkingDirectory"
    }

    Write-Verbose "Using working directory: $WorkingDirectory"

    # ========================================================================
    # Create Wrapper Script
    # ========================================================================

    $escapedProgram = Escape-PowerShellString $Program
    $escapedArguments = Escape-PowerShellString $Arguments
    $escapedWorkingDirectory = Escape-PowerShellString $WorkingDirectory

    $escapedStdoutFile = Escape-PowerShellString $stdoutFile
    $escapedStderrFile = Escape-PowerShellString $stderrFile
    $escapedPidFile = Escape-PowerShellString $pidFile
    $escapedExitCodeFile = Escape-PowerShellString $exitCodeFile
    $escapedDoneFile = Escape-PowerShellString $doneFile

    # The scheduled task runs in the interactive user's session, which does NOT
    # inherit the caller's environment. Anything the child needs has to be
    # written into the wrapper script explicitly. Caller-supplied -Environment
    # entries win over the defaults below.
    $childEnvironment = @{
        # Qt Scene Graph RHI defaults to D3D11 on Windows, which spix cannot
        # introspect reliably. OpenGL keeps the QML item tree observable.
        "QSG_RHI_BACKEND" = "opengl"
        # A redirected stdout makes CPython block-buffer instead of line-buffer,
        # so without this nothing reaches the log until the child exits.
        "PYTHONUNBUFFERED" = "1"
        # A redirected stdout otherwise makes CPython encode with the machine's
        # ANSI codepage, so non-ASCII output is mangled whenever the reading
        # side assumes UTF-8. Pinning the encoding keeps logs comparable
        # between machines. Does not affect the child's C++/Qt output.
        "PYTHONIOENCODING" = "utf-8"
    }

    foreach ($name in $Environment.Keys) {
        $childEnvironment[$name] = [string]$Environment[$name]
    }

    $wrapperEnvironmentLines = @(
        foreach ($name in ($childEnvironment.Keys | Sort-Object)) {
            $escapedValue = Escape-PowerShellString $childEnvironment[$name]
            "    `$env:$name = '$escapedValue'"
        }
    )

    $wrapperLines = @(
        '$ErrorActionPreference = "Stop"'
        ""
        "`$program = '$escapedProgram'"
        "`$arguments = '$escapedArguments'"
        "`$workingDirectory = '$escapedWorkingDirectory'"
        ""
        "`$stdoutFile = '$escapedStdoutFile'"
        "`$stderrFile = '$escapedStderrFile'"
        "`$pidFile = '$escapedPidFile'"
        "`$exitCodeFile = '$escapedExitCodeFile'"
        "`$doneFile = '$escapedDoneFile'"
        ""
        "try {"
        "    New-Item -ItemType Directory -Path `$workingDirectory -Force | Out-Null"
        "    New-Item -ItemType File -Path `$stdoutFile -Force | Out-Null"
        "    New-Item -ItemType File -Path `$stderrFile -Force | Out-Null"
        ""
    )

    $wrapperLines += $wrapperEnvironmentLines

    $wrapperLines += @(
        ""
        "    if ([string]::IsNullOrWhiteSpace(`$arguments)) {"
        "        `$process = Start-Process -FilePath `$program -WorkingDirectory `$workingDirectory -PassThru -RedirectStandardOutput `$stdoutFile -RedirectStandardError `$stderrFile"
        "    }"
        "    else {"
        "        `$process = Start-Process -FilePath `$program -ArgumentList `$arguments -WorkingDirectory `$workingDirectory -PassThru -RedirectStandardOutput `$stdoutFile -RedirectStandardError `$stderrFile"
        "    }"
        ""
        "    `$null = `$process.Handle"
        ""
        "    `$process.Id | Set-Content -Path `$pidFile -Encoding UTF8"
        ""
        "    `$process.WaitForExit()"
        "    `$process.Refresh()"
        "    `$process.ExitCode | Set-Content -Path `$exitCodeFile -Encoding UTF8"
        "}"
        "catch {"
        "    `$_.Exception.ToString() | Set-Content -Path `$stderrFile -Encoding UTF8"
        "    1 | Set-Content -Path `$exitCodeFile -Encoding UTF8"
        "}"
        "finally {"
        "    New-Item -ItemType File -Path `$doneFile -Force | Out-Null"
        "}"
    )

    Set-Content `
        -Path $wrapperFile `
        -Value $wrapperLines `
        -Encoding UTF8

    Write-Verbose "Wrapper created: $wrapperFile"

    # ========================================================================
    # Create Scheduled Task
    # ========================================================================

    $actionArguments = "-NoProfile -ExecutionPolicy Bypass -WindowStyle Hidden -File `"$wrapperFile`""

    $action = New-ScheduledTaskAction `
        -Execute "powershell.exe" `
        -Argument $actionArguments

    $principal = New-ScheduledTaskPrincipal `
        -UserId $userAccount `
        -LogonType Interactive `
        -RunLevel Limited

    $taskSettings = New-ScheduledTaskSettingsSet `
        -AllowStartIfOnBatteries `
        -DontStopIfGoingOnBatteries `
        -ExecutionTimeLimit (New-TimeSpan -Days 1)

    Register-ScheduledTask `
        -TaskName $taskName `
        -Action $action `
        -Principal $principal `
        -Settings $taskSettings `
        -Force | Out-Null

    Write-Verbose "Scheduled task registered: $taskName"

    # ========================================================================
    # Start Task
    # ========================================================================

    Write-OutputLine "Starting: $Program"

    Start-ScheduledTask `
        -TaskName $taskName

    $startTime = Get-Date

    $childPid = $null

    $stdoutOffset = 0
    $stderrOffset = 0

    # ========================================================================
    # Wait for PID
    # ========================================================================

    while (-not (Test-Path $pidFile)) {

        # Stream any early output
        Read-NewOutput `
            -Path $stdoutFile `
            -Offset ([ref]$stdoutOffset)

        Read-NewOutput `
            -Path $stderrFile `
            -Offset ([ref]$stderrOffset) `
            -ErrorStream

        # Detect wrapper failure
        if (Test-Path $doneFile) {

            $errorMessage = ""

            if (Test-Path $stderrFile) {
                $errorMessage = Get-Content `
                    -Path $stderrFile `
                    -Raw
            }

            if ([string]::IsNullOrWhiteSpace($errorMessage)) {
                $errorMessage = "Application wrapper exited before creating PID file."
            }

            throw $errorMessage
        }

        # Timeout check
        if ($TimeoutSeconds -gt 0) {

            $elapsedSeconds = (
                (Get-Date) - $startTime
            ).TotalSeconds

            if ($elapsedSeconds -ge $TimeoutSeconds) {

                Write-Error "Timed out waiting for application to start."

                if (Test-Path $stderrFile) {
                    $diagnostics = Get-Content `
                        -Path $stderrFile `
                        -Raw

                    if (-not [string]::IsNullOrWhiteSpace($diagnostics)) {
                        Write-Error "Startup diagnostics:"
                        Write-Error $diagnostics
                    }
                }

                throw "Timed out waiting for application to start."
            }
        }

        Start-Sleep -Milliseconds $PollMilliseconds
    }

    # ========================================================================
    # Process Started
    # ========================================================================

    $childPid = [int](Get-Content $pidFile)

    Write-OutputLine "Started with PID: $childPid"

    # ========================================================================
    # WaitMode = Start
    # ========================================================================

    if ($WaitMode -eq "Start") {

        # Allow immediate startup failures to surface
        Start-Sleep -Milliseconds 300

        Read-NewOutput `
            -Path $stdoutFile `
            -Offset ([ref]$stdoutOffset)

        Read-NewOutput `
            -Path $stderrFile `
            -Offset ([ref]$stderrOffset) `
            -ErrorStream

        # Check if process exited immediately
        if (Test-Path $doneFile) {

            $exitCode = 1

            if (Test-Path $exitCodeFile) {
                $exitCode = [int](Get-Content $exitCodeFile)
            }

            Write-OutputLine "Process exited immediately with code $exitCode"

            exit $exitCode
        }

        Write-Verbose "Detached successfully."

        exit 0
    }

    # ========================================================================
    # WaitMode = Exit
    # ========================================================================

    Write-OutputLine "Waiting for process $childPid to exit..."

    while (-not (Test-Path $doneFile)) {

        # Stream stdout/stderr
        Read-NewOutput `
            -Path $stdoutFile `
            -Offset ([ref]$stdoutOffset)

        Read-NewOutput `
            -Path $stderrFile `
            -Offset ([ref]$stderrOffset) `
            -ErrorStream

        # Timeout check
        if ($TimeoutSeconds -gt 0) {

            $elapsedSeconds = (
                (Get-Date) - $startTime
            ).TotalSeconds

            if ($elapsedSeconds -ge $TimeoutSeconds) {

                Write-OutputLine "Timeout reached."

                try {
                    Stop-Process `
                        -Id $childPid `
                        -Force `
                        -ErrorAction SilentlyContinue
                }
                catch {
                }

                exit 124
            }
        }

        Start-Sleep -Milliseconds $PollMilliseconds
    }

    # ========================================================================
    # Final Output Flush
    # ========================================================================

    Read-NewOutput `
        -Path $stdoutFile `
        -Offset ([ref]$stdoutOffset)

    Read-NewOutput `
        -Path $stderrFile `
        -Offset ([ref]$stderrOffset) `
        -ErrorStream

    # ========================================================================
    # Retrieve Exit Code
    # ========================================================================

    $exitCode = 0

    if (Test-Path $exitCodeFile) {
        $exitCodeText = (Get-Content -Path $exitCodeFile -Raw).Trim()
    }
    else {
        $exitCodeText = ""
    }

    if ([string]::IsNullOrWhiteSpace($exitCodeText)) {
        # The wrapper never reported an exit code. Do not report success,
        # otherwise a crashed or hung child would look like a clean run.
        Write-OutputLine "WARNING: no exit code was reported by the wrapper."
        $exitCode = 3
    }
    else {
        $exitCode = [int]$exitCodeText
    }

    Write-OutputLine "Process exited with code: $exitCode"

    exit $exitCode
}
finally {

    # Always clean up, including errors and Ctrl+C where possible.
    Cleanup

    Write-Verbose "Temporary files retained at: $tempDir"
    Write-Verbose "Scheduled task retained: $taskName"
}
