#!/bin/ksh
REMOTE_HOST=marvin
LOGIN_USER=wdtester
VERSION=4
importcode=44;
ldif2import=$(pwd)/config/testdbdata.ldif
ldapinstance=slapd-DailyBuildTests
loadscript="$(dirname $0)/ldapLoadTestData.sh"
if [ ! -x $loadscript ]; then
	loadscript="ldapLoadTestData.sh"
fi

function doTestLocal
{
	echo '   importing ldap-data';
	${loadscript} -i $ldapinstance -f $ldif2import -l $LOGIN_USER -h $REMOTE_HOST -v $VERSION load
	importcode=$?;
	echo '   import returned:'$importcode
}

function doTestRemote
{
	echo '   importing ldap-data';
	LD_LIBRARY_PATH_SAVED=$LD_LIBRARY_PATH
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH_NATIVE
	${loadscript} -r -f $ldif2import -i $ldapinstance -l $LOGIN_USER -h $REMOTE_HOST -v $VERSION load
	importcode=$?;
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH_SAVED
}

function doCleanupLocal
{
	# stop ldap server again here
	${loadscript} -i $ldapinstance stop
    if [ $rc == 0 ] -o  "$version" != "4" -a $rc == "2" ]
    then
		:
	else
		echo 'ERROR: stopping of ['$dbreldir'] failed!';
		return $rc;
	fi
	echo '   ldap-server stopped';
}

function doCleanupRemote
{
	LD_LIBRARY_PATH_SAVED=$LD_LIBRARY_PATH
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH_NATIVE
	${loadscript} -r -i $ldapinstance -l $LOGIN_USER -h $REMOTE_HOST -v $VERSION stop
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH_SAVED
}

## add test specific things before the call to callTest
function prepareTest
{
	# insert testdata in ldap here
	if [ "${CURSYSTEM}" = "SunOS" -a "$HOSTNAME" = "marvin" ]; then
		doTestLocal
	elif [ "${CURSYSTEM}" = "SunOS" -o "${CURSYSTEM}" = "Linux" ] ; then
		doTestRemote
	fi
	return 0;
}

## call to wdtest or whatever you want to call
function callTest
{
	if [ $importcode -eq 0 ]; then
		${TEST_EXE} $cfg_testparams
		# remember return code of wdtest, signals number of failures
		return $?;
	else
		echo 'ERROR: setting up of ldap server failed, aborting!';
		return $importcode;
	fi
}

## add test specific things after the call to callTest
# remove generated files ...
function cleanupTest
{
	echo '   stopping ldap-server';
	if [ "${CURSYSTEM}" = "SunOS" -a "$HOSTNAME" = "marvin" ]; then
		doCleanupLocal
	elif [ "${CURSYSTEM}" = "SunOS" -o "${CURSYSTEM}" = "Linux" ] ; then
		doCleanupRemote
	fi
	rm -f rexec.err rexec.out
	return 0;
}
