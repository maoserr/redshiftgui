#!/bin/sh

fullname=`readlink -f $0`
basedir=`dirname "$fullname"`
cmake-gui -H"$basedir" -B"$basedir/buildLinux64"

