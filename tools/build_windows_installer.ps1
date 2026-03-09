Param(
    [string]$BuildDir = "build",
    [ValidateSet("x64", "arm64")]
    [string]$Platform = "x64",
    [string]$WixPath = ""
)

$ErrorActionPreference = "Stop"

$Platform = $Platform.Trim().ToLowerInvariant()

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
Set-Location $repoRoot

if ([string]::IsNullOrWhiteSpace($WixPath)) {
    $WixPath = Join-Path $repoRoot "buildenv\wix311"
}
$WixPath = $WixPath.Trim()

if (-not (Test-Path $WixPath)) {
    Write-Warning "WiX path '$WixPath' was not found. Packaging will fail unless WIX is set elsewhere."
}

Write-Host "[1/5] Configuring Windows dependency environment..."
$cmdConfigure = @(
    "pushd $repoRoot",
    "call C:\BuildTools\Common7\Tools\VsDevCmd.bat -arch=$Platform -host_arch=$Platform",
    "set ""PLATFORM=$Platform""",
    "set ""BUILDENV_RELEASE=TRUE""",
    "set ""WIX=$WixPath""",
    "call tools\windows_buildenv.bat setup",
    "cmake -S . -B $BuildDir -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBULK=ON -DHSS1394=ON -DLOCALECOMPARE=ON -DMAD=ON -DMEDIAFOUNDATION=ON -DMODPLUG=ON -DQT6=ON -DQML=ON -DWAVPACK=ON -DVCPKG_TARGET_TRIPLET=$Platform-windows-release",
    "cmake --build $BuildDir --config RelWithDebInfo",
    "popd"
) -join " && "

cmd.exe /d /s /c $cmdConfigure
if ($LASTEXITCODE -ne 0) {
    throw "Build failed."
}

Write-Host "[2/5] Building installer package with CPack/WiX..."
$wixArch = if ($Platform -eq "arm64") { "ARM64" } else { "x64" }
$cmdPackage = @(
    "pushd $repoRoot\\$BuildDir",
    "call C:\BuildTools\Common7\Tools\VsDevCmd.bat -arch=$Platform -host_arch=$Platform",
    "set ""WIX=$WixPath""",
    "cpack -G WIX -D CPACK_WIX_ARCHITECTURE=$wixArch -V",
    "popd"
) -join " && "

cmd.exe /d /s /c $cmdPackage
if ($LASTEXITCODE -ne 0) {
    throw "Packaging failed. Ensure WiX Toolset 3 is installed and WIX env var is available."
}

Write-Host "[3/5] Validating packaged runtime dependencies..."
$cpackRoot = Join-Path $repoRoot "$BuildDir\_CPack_Packages\win64\WIX"
$filesWxs = Join-Path $cpackRoot "files.wxs"
if (-not (Test-Path $filesWxs)) {
    throw "Packaging output missing '$filesWxs'."
}

$requiredRuntimeDlls = @(
    "Qt6Core.dll",
    "Qt6Gui.dll",
    "Qt6Widgets.dll",
    "Qt6Qml.dll",
    "Qt6Quick.dll",
    "Qt6Multimedia.dll",
    "sqlite3.dll",
    "libcrypto-3-x64.dll",
    "libssl-3-x64.dll"
)

$filesWxsContent = Get-Content -Path $filesWxs -Raw
$missingRuntimeDlls = @()
foreach ($dll in $requiredRuntimeDlls) {
    if ($filesWxsContent -notmatch [regex]::Escape($dll)) {
        $missingRuntimeDlls += $dll
    }
}
if ($missingRuntimeDlls.Count -gt 0) {
    $missingList = $missingRuntimeDlls -join ", "
    throw "MSI payload sanity check failed. Missing runtime DLLs in files.wxs: $missingList"
}

Write-Host "[4/5] Searching for generated installer..."
$msi = Get-ChildItem -Path (Join-Path $repoRoot $BuildDir) -Filter *.msi -File | Sort-Object LastWriteTime -Descending | Select-Object -First 1
if (-not $msi) {
    throw "No .msi produced in '$BuildDir'."
}

Write-Host "[5/5] Installer ready: $($msi.FullName)"
Write-Host "Run it with: Start-Process -FilePath '$($msi.FullName)'"
