#!/bin/ksh

## enable and write this function when you need to test for other files than the one
## defined in $TEST_EXE
## return 1 upon successful test and 0 otherwise
## this function gets called from within RunTests.sh when the option -c is given
function checkTestExe
{
	# dummy return until this gets re-enabled
	return 1;
}

function removeNewProTest
{
	cat <<EOF  | /usr/local/mysql/bin/mysql -u root -p"RotmoaD?"
DROP DATABASE IF EXISTS NewProTest;
DROP DATABASE IF EXISTS NewProTestTestDB;
DROP DATABASE IF EXISTS NewProTestAttDB;
EOF
	cd $DEV_HOME/WWW
	rm -rf NewProTest;
}

## add test specific things before the call to callTest
function prepareTest
{
	echo
#	cd $DEV_HOME/WWW
#	removeNewProTest
#	MakeNewProject.sh NewProTest MySQLBasedStandard
}

## call to wdtest or whatever you want to call
function callTest
{
#	cd NewProTest
#	./src/Test/RunTests.sh
#	let result1=$?
#	./FunkTest/RunFunkTest.sh
#	let result2=$?
#	return `expr $result1 + $result2`;
	return 0;
}

## add test specific things after the call to callTest
# remove generated files ...
function cleanupTest
{
	echo
#	# give server some time to terminate and release all files
#	sleep 2
#	removeNewProTest;
}
