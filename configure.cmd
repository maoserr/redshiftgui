@echo off
setlocal
:: This only works with NT and above

:: Options
set GENERATOR="Visual Studio 10"
GOTO :paramstart

:: Help screen
:help
	echo Supported Generators: ^(use -G ^<generator^>^)
	echo    Visual Studio 9 2008
	echo    NMake Makefiles
	echo    MinGW Makefiles
	echo Options: ^(Use -D[option]=^<value^>^)
	echo    ENABLE_WINGDI=[yes]^|no
	echo    ENABLE_IUP=[yes]^|no
	echo    CMAKE_BUILD_TYPE=Debug^|[Release]
GOTO:EOF

:: Parameter parsing
:paramstart
	IF [%1]==[] (
		call :runcmake %*
		IF [%GENERATOR%]==["MinGW Makefiles"] call :genmake
		IF [%GENERATOR%]==["NMake Makefiles"] call :genmake
		GOTO:EOF
	)
	IF [%1]==[-G] (
		set GENERATOR=%2
		SHIFT
		GOTO paramend
	)
	IF [%1]==[-h] GOTO help
	IF [%1]==[--help] GOTO help
:paramend
SHIFT
GOTO paramstart

:runcmake
	set GENSTRIP=%GENERATOR:"=%
	set GENNOSPC=%GENSTRIP: =_%
	cmake -H"." -B"%~dp0build%GENNOSPC%" %*
GOTO:EOF

:genmake
	echo #This is automatically generated for %GENERATOR%.>Makefile
	for %%A IN (all docs package) DO (
		echo %%A:>>Makefile
		IF [%GENERATOR%]==["MinGW Makefiles"] call :rulemwmake %%A
		IF [%GENERATOR%]==["NMake Makefiles"] call :rulenmake %%A
	)
GOTO:EOF

:rulenmake
	echo		cd "build%GENNOSPC%">>Makefile
	echo		$^(MAKE^)>>Makefile %%A
	echo		cd ..>>Makefile
GOTO:EOF

:rulemwmake
	echo		$^(MAKE^) -C "build%GENNOSPC%">>Makefile %1
GOTO:EOF

endlocal

