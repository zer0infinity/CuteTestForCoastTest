#!/bin/ksh

mypath=`pwd`

retcode=44;

# stop server first in case it is still running
$mypath/stop-ldap.sh
if [ $? -eq 0 ]; then
	# now import the ldif file to overwrite the existing db
	# works only when db-server is down
	$mypath/do_import.sh
	if [ $? -eq 0 ]; then
		# start server again
		$mypath/start-ldap.sh
		if [ $? -eq 0 ]; then
			retcode=0;
		else
			retcode=$?;
			echo 'ERROR: starting of ['$dbreldir'] failed!';
		fi
	else
		retcode=$?;
		echo 'ERROR: importing of testdbdata failed!';
	fi
else
	retcode=$?;
	echo 'ERROR: stopping of ['$dbreldir'] failed!';
fi

exit $retcode
