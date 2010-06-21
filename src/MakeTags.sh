#!/bin/sh

ctags -N --extra=+fq --fields=aim --language-force=c++ *.c *.h gui/*.c gui/*.h backends/*.c backends/*.h

