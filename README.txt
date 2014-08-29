RedshiftGUI - Monitor color temperature adjustment GUI
	http://maoserr.github.io/projects/redshiftgui/

I. Building
===========

1. Prerequisites for building
-----------------------------
You'll need the following libraries to build this program:
	- IUP graphical toolkit
		- It is best to use the prebuilt binaries on windows
		- On Linux it is necessary to build your own to get tray icons
	- libcurl
		- It is best build this statically on windows
		- If you're using MSVC, make sure to use the /MT switch to build
	- GTK2 on Linux (if you're building IUP statically)

2. Building
-----------
Run the Bash script on linux or the Batchfile on windows to generate the make files or projects.  Then you should be able to compile everything.
	- This is the RunCMake.sh/.cmd file


II. Todo list
=============
1) (Hard) Video captured image as reference point?
	-Linux - v4l2 - http://v4l2spec.bytesex.org/spec/capture-example.html
	-Win - http://msdn.microsoft.com/en-us/library/dd757692(v=VS.85).aspx
2) Adjust gamma values
4) Ability to adjust based on weather
5) Custom event scripts
6) Image analysis
7) Check for updates
8) Add log file option
9) (Windows only) Allocate console if logging.


III. Changelog
==============

Upcoming (Version 0.3.0)
------------------------
 * Better google maps parsing (Address lookups)
 * Added another geocode IP lookup service (Geobytes)
 * Change download provider to sourceforge (Github is too awkward)

Friday, August 29, 2014 (Version 0.2.4)
-----------------------------------------
 * Locale bug fix release

Thursday, August 05, 2010 (Version 0.2.1)
-----------------------------------------
 * Added auto method in options
 * Added temperature map for elevation->temperature adjustment
 * Made brightness control work
 * Refactored much of the gamma code
 * Add preview mode

Tuesday, July 20, 2010 - (Version 0.1.2)
----------------------------------------
 * Added brightness control
 * Ability to start minimized or disabled

IV. Limitations
===============
1. WinGDI has a lower limit on gamma ramps, so brightness cannot go below a
   certain limit.
	- This can be fixed by the following registry key:
	[HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\ICM]
	"GdiIcmGammaRange"=dword:00000100

