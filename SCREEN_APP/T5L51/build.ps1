[CmdletBinding()]
param(
    [string]$SdccBin,
    [switch]$UpdateDwinSet
)

$ErrorActionPreference = 'Stop'
$projectRoot = $PSScriptRoot
$buildDir = Join-Path $projectRoot 'build\sdcc'
$distDir = Join-Path $projectRoot 'dist'

if (-not $SdccBin) {
    $candidates = @('C:\msys64\ucrt64\bin')
    $sdccCommand = Get-Command sdcc.exe -ErrorAction SilentlyContinue
    if ($sdccCommand) {
        $candidates += Split-Path $sdccCommand.Source -Parent
    }
    $candidates = $candidates | Where-Object {
        $_ -and (Test-Path -LiteralPath (Join-Path $_ 'sdcc.exe'))
    }
    $SdccBin = $candidates | Select-Object -First 1
}

if (-not $SdccBin) {
    throw 'SDCC not found. Run setup_sdcc.ps1 first or pass -SdccBin <directory>.'
}

$sdcc = Join-Path $SdccBin 'sdcc.exe'
$assembler = Join-Path $SdccBin 'sdas8051.exe'
$makebin = Join-Path $SdccBin 'makebin.exe'
$sdccRoot = (Resolve-Path -LiteralPath (Join-Path $SdccBin '..\share\sdcc')).Path

foreach ($tool in @($sdcc, $assembler, $makebin)) {
    if (-not (Test-Path -LiteralPath $tool)) {
        throw "Required SDCC tool not found: $tool"
    }
}

# SDCC starts sdcpp/sdld as child processes, so its bin directory must be on PATH.
$env:PATH = "$SdccBin;$env:PATH"
New-Item -ItemType Directory -Force -Path $buildDir, $distDir | Out-Null

$includeArgs = @(
    '-IUSER',
    '-IHANDWARE\UART2',
    '-IHANDWARE\TASK',
    '-IHANDWARE\NOR_FLASH',
    "-I$(Join-Path $sdccRoot 'include\mcs51')",
    "-I$(Join-Path $sdccRoot 'include')"
)

$compileArgs = @(
    '-mmcs51',
    '--model-large',
    '--std-c11',
    '--opt-code-size',
    '--xram-loc', '0x8000',
    '--xram-size', '0x8000',
    '--iram-size', '0x100',
    '--code-size', '0x8000'
) + $includeArgs

$sources = @(
    'USER\main.c',
    'USER\sys.c',
    'HANDWARE\UART2\uart2.c',
    'HANDWARE\TASK\task.c',
    'HANDWARE\NOR_FLASH\nor_flash.c'
)

function Invoke-Checked {
    param(
        [Parameter(Mandatory)] [string]$FilePath,
        [Parameter(Mandatory)] [object[]]$Arguments
    )
    & $FilePath @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "$FilePath failed with exit code $LASTEXITCODE"
    }
}

Push-Location $projectRoot
try {
    foreach ($source in $sources) {
        $name = [IO.Path]::GetFileNameWithoutExtension($source)
        Write-Host "[CC] $source"
        Invoke-Checked $sdcc ($compileArgs + @(
            '-c', $source,
            '-o', (Join-Path $buildDir "$name.rel")
        ))
    }

    Write-Host '[AS] USER\startup_sdcc.asm'
    Invoke-Checked $assembler @(
        '-plosgff',
        (Join-Path $buildDir 'startup_sdcc.rel'),
        'USER\startup_sdcc.asm'
    )
}
finally {
    Pop-Location
}

Push-Location $buildDir
try {
    Write-Host '[LD] T5L51.ihx'
    Invoke-Checked $sdcc @(
        '-mmcs51',
        '--model-large',
        '--xram-loc', '0x8000',
        '--xram-size', '0x8000',
        '--iram-size', '0x100',
        '--code-size', '0x8000',
        "-L$(Join-Path $sdccRoot 'lib\large')",
        '-o', 'T5L51.ihx',
        'startup_sdcc.rel',
        'main.rel',
        'sys.rel',
        'uart2.rel',
        'task.rel',
        'nor_flash.rel'
    )

    Write-Host '[BIN] T5L51.bin'
    Invoke-Checked $makebin @('-p', 'T5L51.ihx', 'T5L51.bin')
}
finally {
    Pop-Location
}

$binaryPath = Join-Path $buildDir 'T5L51.bin'
$bytes = [IO.File]::ReadAllBytes($binaryPath)
$expectedSignature = [byte[]](0xFF, 0xFF, 0x44, 0x57, 0x49, 0x4E, 0x54, 0x35)

if ($bytes.Length -gt 0x8000) {
    throw "T5L51.bin exceeds the 32 KiB application limit: $($bytes.Length) bytes"
}
if ($bytes.Length -le 0x102) {
    throw 'T5L51.bin is too small to contain the required T5L header.'
}
if ($bytes[0x0000] -ne 0x02 -or $bytes[0x0023] -ne 0x02 -or $bytes[0x002B] -ne 0x02) {
    throw 'Reset/UART2/Timer2 interrupt vectors are missing.'
}
for ($i = 0; $i -lt $expectedSignature.Length; $i++) {
    if ($bytes[0x00F8 + $i] -ne $expectedSignature[$i]) {
        throw 'The required FF FF DWINT5 signature is missing at 0x00F8.'
    }
}
if ($bytes[0x0100] -ne 0x02) {
    throw 'The T5L application entry jump is missing at 0x0100.'
}

$distBinary = Join-Path $distDir 'T5L51.bin'
Copy-Item -LiteralPath $binaryPath -Destination $distBinary -Force
Copy-Item -LiteralPath (Join-Path $buildDir 'T5L51.map') -Destination (Join-Path $distDir 'T5L51.map') -Force

if ($UpdateDwinSet) {
    $dwinSetBinary = Join-Path $projectRoot '..\DGUS\DWIN_SET\T5L51.bin'
    Copy-Item -LiteralPath $distBinary -Destination $dwinSetBinary -Force
    Write-Host "[COPY] $dwinSetBinary"
}

$hash = (Get-FileHash -Algorithm SHA256 -LiteralPath $distBinary).Hash
Write-Host ''
Write-Host 'T5L51 SDCC build succeeded.'
Write-Host "BIN    : $distBinary"
Write-Host "SIZE   : $($bytes.Length) bytes / 32768 bytes"
Write-Host "SHA256 : $hash"
