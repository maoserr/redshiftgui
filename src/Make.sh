#!/bin/sh

if [ `uname -m` = i686 ]; then
	cd ../buildLinux32
	make
else
	cd ../buildLinux64
	make
fi

