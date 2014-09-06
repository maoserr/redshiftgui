#!/bin/sh

mkdir ./buildLinuxi686/fixdeb
dpkg-deb -x ./buildLinuxi686/RedshiftGUI-0.2.4-Linux-i686.deb ./buildLinuxi686/fixdeb
dpkg-deb --control ./buildLinuxi686/RedshiftGUI-0.2.4-Linux-i686.deb ./buildLinuxi686/fixdeb/DEBIAN
chmod 0644 ./buildLinuxi686/fixdeb/DEBIAN/md5sums 
find ./buildLinuxi686/fixdeb -type d -print0 | xargs -0 chmod 755
fakeroot dpkg -b ./buildLinuxi686/fixdeb RedshiftGUI-0.2.4-Linux-i686-Fixed.deb

