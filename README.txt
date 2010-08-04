RedshiftGUI - Monitor color temperature adjustment GUI
	http://www.mao-yu.com/projects/redshiftgui/

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
1) Set sun elevation for transition period
2) Parameters for non-linear transition
3) (Hard) Video captured image as reference point?
	-Linux - v4l2 - http://v4l2spec.bytesex.org/spec/capture-example.html
	-Win - http://msdn.microsoft.com/en-us/library/dd757692(v=VS.85).aspx
4) Adjust gamma values
5) Better google maps parsing


III. Changelog
==============

(Version 0.1.3)
---------------
 * Added auto method in options
 * Added temperature map for elevation->temperature adjustment
 * Made brightness control work
 * Refactored much of the gamma code

Tuesday, July 20, 2010 - (Version 0.1.2)
----------------------------------------
 * Added brightness control
 * Ability to start minimized or disabled

