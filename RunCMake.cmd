@echo off
setlocal

cmake -H"." -B"%~dp0buildWin32"

endlocal
