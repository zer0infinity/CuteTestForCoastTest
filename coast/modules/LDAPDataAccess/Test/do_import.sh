#!/bin/ksh

mypath=`pwd`
NETSITE_ROOT=/usr/local/netscape/server4; export NETSITE_ROOT
dbreldir=slapd-DailyBuildTests
importbinpath=bin/slapd/server
importbin=ns-slapd
file2import=$mypath/config/testdbdata.ldif

retcode=44;

# now import the ldif file to overwrite the existing db
# works only when db-server is down
cd $NETSITE_ROOT/$importbinpath; $importbin ldif2db -f $NETSITE_ROOT/$dbreldir/config/slapd.conf -C -i $file2import
if [ $? -eq 0 ]; then
	retcode=0;
else
	retcode=$?;
	echo 'ERROR: importing of testdbdata failed!';
fi

exit $retcode;
