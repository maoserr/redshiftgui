@echo off
setlocal

set basedir=%~dp0
cmake-gui -H"%basedir%" -B"%basedir%buildWin32"


endlocal
