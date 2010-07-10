#!/bin/bash

for i in 16 22 24 32 36 48 64 72 96 128 192 256
do
	mkdir -p ./icons/hicolor/${i}x${i}/apps
	rsvg -w $i -h $i ./icons/hicolor/scalable/apps/redshiftgui.svg \
		./icons/hicolor/${i}x${i}/apps/redshiftgui.png
done

