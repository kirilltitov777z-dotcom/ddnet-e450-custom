@echo off
cd /d "%~dp0"
type nul > stop.flag
echo STOP flag created. Close DDNet now - launcher will exit instead of loading next level.
timeout /t 3 >nul
