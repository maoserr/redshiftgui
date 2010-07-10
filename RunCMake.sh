#!/bin/sh

fullname=`readlink -f $0`
basedir=`dirname "$fullname"`
cmake -H"$basedir" -B"$basedir/build`uname``uname -m`"

