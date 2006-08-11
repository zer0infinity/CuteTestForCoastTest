#-----------------------------------------------------------------------------------------------------
# Copyright (c) 2006, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
# All rights reserved.
#
# This library/application is free software; you can redistribute and/or modify it under the terms of
# the license that is included with this library/application in the file license.txt.
#-----------------------------------------------------------------------------------------------------

_SYBASE_DIR=/sybasehome/sybase12
DB_NAME=pub2
DB_USER=-Usa
DB_PASSWD=-P"sagamma01h"
DB_SHELL=isql
DB_IMPORTFILE='config/createPub2.sql'
DB_DROPFILE='config/dropPub2.sql'
DB_INTERFACEFILE=${_SYBASE_DIR}/interfaces
SHELL_EXPORTS="${_SYBASE_DIR}/SYBASE.sh"
REMOTE_TMPDIR=/tmp/DailySybaseTests
IMPORT_CMD="cat ${DB_IMPORTFILE} | ${DB_SHELL} ${DB_USER} ${DB_PASSWD} -S ${DB_SERVER} -I ${DB_INTERFACEFILE}"
DROP_CMD="cat ${DB_DROPFILE} | ${DB_SHELL} ${DB_USER} ${DB_PASSWD} -S ${DB_SERVER} -I ${DB_INTERFACEFILE}"
importcode=44;

function doTestLocal
{
	if [ "${DOMAIN}" = "hsr.ch" ]; then
		echo '   importing '${DB_NAME}'-data';

		( . ${SHELL_EXPORTS}; eval $IMPORT_CMD )
		importcode=$?;
		echo '   import returned:'$importcode
	else
		echo
	fi
}

function doTestRemote
{
	if [ "${DOMAIN}" = "hsr.ch" ]; then
		# use rexec to set up ${DB_NAME}
		# first 'copy' the needed files to /tmp/DailySybaseTests on remote machine
		echo '   importing '${DB_NAME}'-data';
		( $RSH_CMD -n -l ${RSH_USER} $REMOTE_HOST "mkdir -p $REMOTE_TMPDIR/config;"; )
		for myfile in ${DB_IMPORTFILE} ${DB_DROPFILE}; do
			( $RSH_CMD -l ${RSH_USER} $REMOTE_HOST "cat >$REMOTE_TMPDIR/$myfile; chmod +x $REMOTE_TMPDIR/$myfile;" <./$myfile; )
			echo '   remote copying ['$myfile']';
		done
		echo '   remote executing import';
		$RSH_CMD -n -l ${RSH_USER} $REMOTE_HOST "cd $REMOTE_TMPDIR; . ${SHELL_EXPORTS}; $IMPORT_CMD; echo \$?" >rexec.out 2>rexec.err
		local mycode=`tail -1 rexec.out`;
		if [ $mycode -ne 0 ]; then
			echo '    result was '$mycode;
			echo '=== stdout ===';
			cat rexec.out
			echo '=== stderr ===';
			cat rexec.err
			importcode=$mycode;
			break;
		fi
		if [ $importcode -eq 44 ]; then
			# if code is still 44 then everything was good
			importcode=0;
		fi
	else
		echo
	fi
}

function doCleanupLocal
{
	if [ "${DOMAIN}" = "hsr.ch" ]; then
		( . ${SHELL_EXPORTS}; eval $DROP_CMD )
		if [ $? -ne 0 ]; then
			echo 'ERROR: dropping of '${DB_NAME}' failed!';
			return $?;
		fi
		echo '   '${DB_NAME}'-db dropped';
	else
		echo
	fi
}

function doCleanupRemote
{
	if [ "${DOMAIN}" = "hsr.ch" ]; then
		# use rexec to set stop
		echo '   remote dropping '${DB_NAME};
		( $RSH_CMD -n -l ${RSH_USER} $REMOTE_HOST "cd $REMOTE_TMPDIR; . ${SHELL_EXPORTS}; $DROP_CMD; echo \$?" >rexec.out 2>rexec.err )
		local mycode=`tail -1 rexec.out`;
		if [ $mycode -ne 0 ]; then
			echo '    result was '$mycode;
			echo '=== stdout ===';
			cat rexec.out
			echo '=== stderr ===';
			cat rexec.err
			break;
		fi
		( $RSH_CMD -n -l ${RSH_USER} $REMOTE_HOST 'rm -rf $REMOTE_TMPDIR;'; )
		rm -f rexec.out rexec.err
	else
		echo
	fi
}
## enable and write this function when you need to test for other files than the one
## defined in $TEST_EXE
## return 1 upon successful test and 0 otherwise
## this function gets called from within RunTests.sh when the option -c is given
function checkTestExe
{
	if [ -x "${TEST_EXE}" ]; then
		return 1;
	else
		return 0;
	fi;
}

## enable and write this function when you need to test for other files than the one
## defined in $TEST_EXE
## return 1 upon successful test and 0 otherwise
## this function gets called from within RunTests.sh when the option -c is given
function checkTestExe
{
	# dummy return until we have full Sybase support at ifs
	return 1;
}

## add test specific things before the call to callTest
function prepareTest
{
	# insert testdata here
	if [ "${DOMAIN}" = "hsr.ch" ]; then
		echo '   setting up '${DB_NAME}' testdata';
		if [ "$HOSTNAME" = "bondo" ]; then
			doTestLocal
		else
			doTestRemote
		fi
		return 0;
	else
		return 1;
	fi
}

## call to wdtest or whatever you want to call
function callTest
{
	if [ "${DOMAIN}" = "hsr.ch" ]; then
		if [ $importcode -eq 0 ]; then
			${TEST_EXE} $cfg_testparams
			# remember return code of wdtest, signals number of failures
			return $?;
		else
			echo 'ERROR: setting up of '${DB_NAME}'-db failed, aborting!';
			return $importcode;
		fi;
	else
		echo 'No Test on this platform and host';
		${TEST_EXE} $cfg_testparams
		return $?;
	fi
}

## add test specific things after the call to callTest
# remove generated files ...
function cleanupTest
{
	if [ "${DOMAIN}" = "hsr.ch" ]; then
		echo '   dropping '${DB_NAME}' testdata';
		if [ "$HOSTNAME" = "bondo" ]; then
			doCleanupLocal
		else
			doCleanupRemote
		fi
	else
		echo
	fi
}
