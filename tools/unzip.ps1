function unzipTo {
    param (
        [string]$archiveFilePath,
        [string]$destinationPath
    )

    # Ensure absolute paths. Convert from relative if needed
    if ($archiveFilePath -notlike '?:\*') {
        $archiveFilePath = [System.IO.Path]::Combine($PWD, $archiveFilePath)
    }

    if ($destinationPath -notlike '?:\*') {
        $destinationPath = [System.IO.Path]::Combine($PWD, $destinationPath)
    }

    # Test supported features.
    if ($PSVersionTable.PSVersion.Major -ge 3) {
        # Powershell 3 (Windows 8). This needs .NET framework 4.5, available in Windows 8


        Add-Type -AssemblyName System.IO.Compression
        Add-Type -AssemblyName System.IO.Compression.FileSystem

        $archiveFile = [System.IO.File]::Open($archiveFilePath, [System.IO.FileMode]::Open)
        $archive = [System.IO.Compression.ZipArchive]::new($archiveFile)

        # If the directory exists, unzip item by itep to force the extraction. Else it would fail if file already exists.
        if (Test-Path $destinationPath) {
            foreach ($item in $archive.Entries) {
                $destinationItemPath = [System.IO.Path]::Combine($destinationPath, $item.FullName)

                if ($destinationItemPath -like '*/') {
                    New-Item $destinationItemPath -Force -ItemType Directory > $null
                } else {
                    New-Item $destinationItemPath -Force -ItemType File > $null

                    [System.IO.Compression.ZipFileExtensions]::ExtractToFile($item, $destinationItemPath, $true)
                }
            }
        } else {
            # On, no folder present, use the simpler method.
            [System.IO.Compression.ZipFileExtensions]::ExtractToDirectory($archive, $destinationPath)
        }
    }
    else {
        # We are on powerwhell 2. We try using the system. Note that this requires Windows 7 since it uses the Explorer zip capability.
        # Note also that this is slow for big files.
        New-Item -ItemType directory -Path $destinationPath -Force

        $shell = New-Object -ComObject Shell.Application
        $zip = $shell.Namespace($archiveFilePath)
        $shell.Namespace($destinationPath).CopyHere($zip.items())
    }
    # This is as slow as the Shell.Application.CopyHere, so the System.IO.ExtractToDirectory is favoured
    #if ($PSVersionTable.PSVersion.Major -ge 5) {
    #    # Powershell 5 (Windows 10). Expand-Archive supported
    #    Expand-Archive -Path $archiveFilePath -DestinationPath $destinationPath
    #}
}

$arg1, $arg2 = $args

unzipTo $arg1 $arg2
