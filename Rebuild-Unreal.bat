@echo off
set SCRIPT=%~dp0Rebuild-Unreal.ps1
powershell -ExecutionPolicy Bypass -NoProfile -File "%SCRIPT%" %*
pause
