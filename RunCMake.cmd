@echo off
setlocal

set basedir=%~dp0
cmake -H"%basedir%" -B"%basedir%buildWin32"


endlocal
