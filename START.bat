@echo off
cd /d "%~dp0"
if not exist "%APPDATA%\DDNet" mkdir "%APPDATA%\DDNet"
copy /Y "autoexec_client.cfg" "%APPDATA%\DDNet\autoexec_client.cfg" >nul
if "%~1"=="" (
  start "" "DDNet-Custom-Launcher.exe"
) else (
  start "" "DDNet-Custom-Launcher.exe" %1
)
