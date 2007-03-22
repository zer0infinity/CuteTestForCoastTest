#!/bin/ksh

mypath=`pwd`
NETSITE_ROOT=/db/wdtest; export NETSITE_ROOT
dbreldir=slapd-DailyBuildTests

stopretcode=44;

# stop server first in case it is still running
$NETSITE_ROOT/$dbreldir/stop-slapd
if [ $? -eq 0 ]; then
	stopretcode=0;
else
	stopretcode=$?;
	echo 'ERROR: stopping of ['$dbreldir'] failed!';
fi

exit $stopretcode;
