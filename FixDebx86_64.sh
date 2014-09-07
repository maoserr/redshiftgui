#!/bin/sh

mkdir ./buildLinuxx86_64/fixdeb
dpkg-deb -x ./buildLinuxx86_64/RedshiftGUI-0.2.4-Linux-x86_64.deb ./buildLinuxx86_64/fixdeb
dpkg-deb --control ./buildLinuxx86_64/RedshiftGUI-0.2.4-Linux-x86_64.deb ./buildLinuxx86_64/fixdeb/DEBIAN
chmod 0644 ./buildLinuxx86_64/fixdeb/DEBIAN/md5sums 
find ./buildLinuxx86_64/fixdeb -type d -print0 | xargs -0 chmod 755
fakeroot dpkg -b ./buildLinuxx86_64/fixdeb RedshiftGUI-0.2.4-Linux-x86_64-Fixed.deb

