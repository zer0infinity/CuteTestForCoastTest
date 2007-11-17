#!/bin/sh

export WD_PATH=config:.

if [ "$1" != "" ] ; then
	src/wdtest $1
else
	for file in `ls src/*Test.cpp | awk -F'.cpp' '{ print $1}' | awk -F'src/' '{print $2}'` ; do
		if [ "$file" != "ConnectorTest" ] ; then
			src/wdtest $file
		fi
	done
fi

