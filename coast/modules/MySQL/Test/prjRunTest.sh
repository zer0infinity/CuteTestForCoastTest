#!/bin/ksh

RSH_CMD=ssh
REMOTE_HOST=zaphod
REMOTE_TMPDIR=/tmp/DailyMYSQLTests
LOGIN_USER=wdtester
IMPORT_CMD='cat ./config/TestDBDefinition.sql | /usr/bin/mysql -uroot -p"RotmoaD?"'
DROP_CMD='cat ./config/dropTestDefinitions.sql | /usr/bin/mysql -uroot -p"RotmoaD?"'

importcode=44;

function doTestLocal
{
	echo '   importing mysql-data';
	eval $IMPORT_CMD
	importcode=$?;
	echo '   import returned:'$importcode
}

function doTestRemote
{
	# use rexec to set up mysql
	# first 'copy' the needed files to /tmp/DailyMYSQLTests on remote machine
	echo '   importing mysql-data';
	LD_LIBRARY_PATH_SAVED=$LD_LIBRARY_PATH
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH_NATIVE
	( $RSH_CMD -n -l $LOGIN_USER $REMOTE_HOST "mkdir -p $REMOTE_TMPDIR/config;"; )
	for myfile in config/TestDBDefinition.sql config/dropTestDefinitions.sql; do
		( $RSH_CMD -l $LOGIN_USER $REMOTE_HOST "cat >$REMOTE_TMPDIR/$myfile; chmod +x $REMOTE_TMPDIR/$myfile;" <./$myfile; )
		echo '   remote copying ['$myfile']';
	done
	echo '   remote executing import';
	$RSH_CMD -n -l $LOGIN_USER $REMOTE_HOST "cd $REMOTE_TMPDIR; $IMPORT_CMD; echo \$?" >rexec.out 2>rexec.err
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
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH_SAVED
}

function doCleanupLocal
{
	eval $DROP_CMD
	if [ $? -ne 0 ]; then
		echo 'ERROR: dropping of test-db failed!';
		return $?;
	fi
	echo '   mysql-db dropped';
}

function doCleanupRemote
{
	# use rexec to set stop
	echo '   remote dropping db';
	LD_LIBRARY_PATH_SAVED=$LD_LIBRARY_PATH
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH_NATIVE
	( $RSH_CMD -n -l $LOGIN_USER $REMOTE_HOST "cd $REMOTE_TMPDIR; $DROP_CMD; echo \$?" >rexec.out 2>rexec.err )
	local mycode=`tail -1 rexec.out`;
	if [ $mycode -ne 0 ]; then
		echo '    result was '$mycode;
		echo '=== stdout ===';
		cat rexec.out
		echo '=== stderr ===';
		cat rexec.err
		break;
	fi
	( $RSH_CMD -n -l $LOGIN_USER $REMOTE_HOST 'rm -rf $REMOTE_TMPDIR;'; )
	rm -f rexec.out rexec.err
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH_SAVED
}

## add test specific things before the call to callTest
function prepareTest
{
	# insert testdata here
	if [ "${DOMAIN}" = "hsr.ch" ]; then
		doTestRemote
	elif [ "${CURSYSTEM}" = "SunOS" ]; then
		doTestRemote
	elif [ "${CURSYSTEM}" = "Linux" ]; then
		doTestLocal
	fi
	echo
}

## call to wdtest or whatever you want to call
function callTest
{
	if [ $importcode -eq 0 ]; then
		${TEST_EXE} $cfg_testparams
		# remember return code of wdtest, signals number of failures
		return $?;
	else
		echo 'ERROR: setting up of mysql-db failed, aborting!';
		return $importcode;
	fi
}

## add test specific things after the call to callTest
# remove generated files ...
function cleanupTest
{
	echo '   dropping mysql testdata';
	if [ "${DOMAIN}" = "hsr.ch" ]; then
		doCleanupRemote
	elif [ "${CURSYSTEM}" = "SunOS" -a "$HOSTNAME" = "marvin" ]; then
		doCleanupRemote
	elif [ "${CURSYSTEM}" = "SunOS" -a "$HOSTNAME" = "atlantis" ]; then
		doCleanupRemote
	elif [ "${CURSYSTEM}" = "Linux" ]; then
		doCleanupLocal
	fi
	echo
}
