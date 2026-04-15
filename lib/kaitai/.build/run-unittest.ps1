<#
.DESCRIPTION
Runs unit tests on Windows
#>

# Standard boilerplate
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$PSDefaultParameterValues['*:ErrorAction'] = 'Stop'

# Go to repo root
$repoRoot = (Resolve-Path "$PSScriptRoot\..").Path
Push-Location $repoRoot

try {
    cd build

    # Use ctest
    #ctest -C Debug --output-on-failure

    # Run gtest-generated binary directly, produces more detailed output
    #
    # NOTE: `$args` is a built-in PowerShell variable that contains all command-line arguments
    # passed to the script (see https://learn.microsoft.com/en-us/powershell/module/microsoft.powershell.core/about/about_automatic_variables?view=powershell-7.5#args).
    # We use [splatting](https://learn.microsoft.com/en-us/powershell/module/microsoft.powershell.core/about/about_splatting?view=powershell-7.5)
    # to pass all received arguments to the test runner.
    ./tests/Debug/unittest.exe @args
} finally {
    Pop-Location
}
