[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$winget = Get-Command winget.exe -ErrorAction Stop

Write-Host 'Installing MSYS2...'
& $winget.Source install --id MSYS2.MSYS2 --exact `
    --accept-package-agreements --accept-source-agreements `
    --silent --disable-interactivity
if ($LASTEXITCODE -ne 0) {
    throw "winget failed with exit code $LASTEXITCODE"
}

$bash = 'C:\msys64\usr\bin\bash.exe'
if (-not (Test-Path -LiteralPath $bash)) {
    throw "MSYS2 bash not found: $bash"
}

Write-Host 'Installing SDCC 4.6 or newer from the MSYS2 UCRT64 repository...'
& $bash -lc 'pacman -Sy --noconfirm mingw-w64-ucrt-x86_64-sdcc'
if ($LASTEXITCODE -ne 0) {
    throw "pacman failed with exit code $LASTEXITCODE"
}

& 'C:\msys64\ucrt64\bin\sdcc.exe' --version
Write-Host 'SDCC setup completed.'
