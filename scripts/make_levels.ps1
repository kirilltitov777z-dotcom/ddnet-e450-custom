# Собирает список всех официальных серверов DDNet со страницы статуса.
$ErrorActionPreference = 'Stop'
$Page = (Invoke-WebRequest -Uri 'https://ddnet.org/status/?lang=en' -UseBasicParsing -UserAgent 'Mozilla/5.0').Content
$Addrs = [regex]::Matches($Page, '\b(?:\d{1,3}\.){3}\d{1,3}:\d{4,5}\b') |
    ForEach-Object { $_.Value } | Sort-Object -Unique
if ($Addrs.Count -lt 20) { throw "Expected many servers, got $($Addrs.Count)" }
$Lines = @('# Auto-generated list of official DDNet servers = endless maps')
$Lines += $Addrs | ForEach-Object { "$_|DDNet" }
$Lines | Set-Content -Encoding UTF8 'infinite-levels.txt'
Write-Host "Wrote $($Addrs.Count) servers to infinite-levels.txt"
