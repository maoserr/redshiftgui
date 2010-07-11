I. Building
========

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

