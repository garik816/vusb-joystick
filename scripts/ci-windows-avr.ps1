#requires -Version 7
<#
  Reuse installed toolchains on a Windows 11 self-hosted runner.
  Builds both the actual KiCad ATmega8 and the ATmega328P alternative.
  Does not access COM ports, flash MCUs or change fuse bits.
#>
[CmdletBinding()]
param([Parameter(Mandatory)][ValidateSet('test','setup','build')][string]$Phase)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$Repo = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$MiniIndex = 'https://mcudude.github.io/MiniCore/package_MCUdude_MiniCore_index.json'
$env:ARDUINO_UPDATER_ENABLE_NOTIFICATION = 'false'

function Check-Exit([string]$Name) {
    if ($LASTEXITCODE -ne 0) { throw "$Name failed with exit code $LASTEXITCODE" }
}
function Find-Python {
    foreach($name in @('python3','python')) {
        $c = Get-Command $name -ErrorAction SilentlyContinue
        if ($c -and $c.Source -and $c.Source -notmatch 'WindowsApps') { return $c.Source }
    }
    if (Test-Path 'C:\Program Files\Python311\python.exe') {
        return 'C:\Program Files\Python311\python.exe'
    }
    throw 'Python 3 missing on self-hosted runner'
}
function Find-Gpp {
    $path = 'C:\msys64\ucrt64\bin\g++.exe'
    if (Test-Path $path) {
        $env:PATH = "C:\msys64\ucrt64\bin;$env:PATH"
        return $path
    }
    $c = Get-Command 'g++' -ErrorAction SilentlyContinue
    if ($c) { return $c.Source }
    throw 'Install MSYS2 UCRT64 GCC/G++ or add it to PATH'
}
function Cache-Root {
    if ($env:RUNNER_TOOL_CACHE) { return (Join-Path $env:RUNNER_TOOL_CACHE 'vusb-avr') }
    return (Join-Path $env:LOCALAPPDATA 'vusb-avr')
}
function Find-Arduino([bool]$Install) {
    if ($env:ARDUINO_CLI -and (Test-Path $env:ARDUINO_CLI)) { return $env:ARDUINO_CLI }
    $c = Get-Command 'arduino-cli' -ErrorAction SilentlyContinue
    if ($c) { return $c.Source }
    $root = Cache-Root
    $exe = Join-Path $root 'arduino-cli.exe'
    if (Test-Path $exe) { return $exe }
    if (-not $Install) { throw 'Arduino CLI missing. Run setup phase first.' }

    # Pinned official Arduino CLI 1.5.1, downloaded only once.
    $url = 'https://github.com/arduino/arduino-cli/releases/download/v1.5.1/arduino-cli_1.5.1_Windows_64bit.zip'
    $hash = 'fabe42e0eb04d00e776a66178299ff95a46c623dbc260f997e58fd514853dd40'
    New-Item -ItemType Directory -Force -Path $root | Out-Null
    $tmp = Join-Path $env:RUNNER_TEMP 'vusb-cli-install'
    if (Test-Path $tmp) { Remove-Item $tmp -Force -Recurse }
    New-Item -ItemType Directory -Force -Path $tmp | Out-Null
    try {
        $zip = Join-Path $tmp 'cli.zip'
        Write-Host 'Downloading missing Arduino CLI 1.5.1 (one-time installation)'
        Invoke-WebRequest -Uri $url -OutFile $zip
        $actual = (Get-FileHash -Algorithm SHA256 -LiteralPath $zip).Hash.ToLowerInvariant()
        if ($actual -ne $hash) { throw "Arduino CLI archive SHA256 mismatch: $actual" }
        Expand-Archive -Path $zip -DestinationPath $tmp -Force
        Copy-Item (Join-Path $tmp 'arduino-cli.exe') $exe -Force
    } finally {
        Remove-Item -Path $tmp -Recurse -Force -ErrorAction SilentlyContinue
    }
    return $exe
}
function Host-Tests {
    Set-Location $Repo
    $py = Find-Python
    $gpp = Find-Gpp
    & $py 'scripts/check_hid_descriptor.py'
    Check-Exit 'HID report descriptor test'
    $out = Join-Path $env:RUNNER_TEMP 'vusb-host-tests.exe'
    if (Test-Path $out) { Remove-Item $out -Force }
    & $gpp -std=c++11 -Wall -Wextra -Werror 'tests/test_logic.cpp' -o $out
    Check-Exit 'Host tests compile'
    & $out
    Check-Exit 'Host tests execution'
    Write-Host 'PASS: HID descriptor + host-side axis/button/hat tests'
}
function Setup-Tools {
    Set-Location $Repo
    $cli = Find-Arduino $true
    & $cli version
    Check-Exit 'Arduino CLI version'
    if ($env:GITHUB_ENV) {
        "ARDUINO_CLI=$cli" | Out-File -FilePath $env:GITHUB_ENV -Encoding utf8 -Append
    }
    $cores = ((& $cli core list) -join [Environment]::NewLine)
    Check-Exit 'Arduino CLI core list'
    $wantMini = $cores -notmatch 'MiniCore:avr'
    $wantAvr = $cores -notmatch 'arduino:avr'
    if ($wantMini -or $wantAvr) {
        & $cli core update-index --additional-urls $MiniIndex
        Check-Exit 'Board index update'
        if ($wantMini) {
            & $cli core install MiniCore:avr --additional-urls $MiniIndex
            Check-Exit 'MiniCore install'
        } else { Write-Host 'Using installed MiniCore:avr' }
        if ($wantAvr) {
            & $cli core install arduino:avr
            Check-Exit 'Arduino AVR install'
        } else { Write-Host 'Using installed arduino:avr' }
    } else { Write-Host 'Reusing both AVR cores, no download or reinstall.' }
    & $cli core list
    Check-Exit 'Installed cores verification'
}
function Build-Firmware {
    Set-Location $Repo
    $cli = Find-Arduino $false
    $dist = Join-Path $Repo 'dist'
    if (Test-Path $dist) { Remove-Item $dist -Force -Recurse }
    New-Item -ItemType Directory -Force -Path $dist | Out-Null
    $boards = @(
        @{ Name='ATmega8-16MHz'; Fqbn='MiniCore:avr:8:clock=16MHz_external,bootloader=no_bootloader,BOD=2v7,LTO=Os_flto' },
        @{ Name='ATmega328P-16MHz'; Fqbn='arduino:avr:uno' }
    )
    foreach($board in $boards) {
        Write-Host "::group::Compile $($board.Name)"
        $folder = Join-Path $dist $board.Name
        New-Item -ItemType Directory -Force -Path $folder | Out-Null
        & $cli compile --warnings all --fqbn $board.Fqbn --libraries (Join-Path $Repo 'libraries') --output-dir $folder (Join-Path $Repo 'joystik')
        Check-Exit "Arduino compile $($board.Name)"
        $hex = @(Get-ChildItem -LiteralPath $folder -Filter '*.hex' -File)
        if ($hex.Count -eq 0) { throw "No HEX generated for $($board.Name)" }
        $lines = @(
            'firmware=v2.0.0',
            "board=$($board.Name)",
            "fqbn=$($board.Fqbn)",
            "source_commit=$env:GITHUB_SHA",
            'programming=ISP (CI does not flash hardware)'
        )
        foreach($file in $hex) {
            $sha256 = (Get-FileHash -LiteralPath $file.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
            $lines += "$($file.Name) sha256=$sha256"
            Write-Host "Produced $($file.FullName) SHA256=$sha256"
        }
        $lines | Set-Content -LiteralPath (Join-Path $folder 'BUILD_INFO.txt') -Encoding utf8
        Write-Host '::endgroup::'
    }
}
switch ($Phase) {
    test { Host-Tests }
    setup { Setup-Tools }
    build { Build-Firmware }
}
