#!/bin/ksh

## add test specific things before the call to callTest
function prepareTest
{
	echo
	chmod u+w ${CONFIGDIRABS}/AnythingPerfTest.stat.any 2>/dev/null
	chmod u+w ${CONFIGDIRABS}/StringPerfTest.stat.any 2>/dev/null
}

## call to wdtest or whatever you want to call
function callTest
{
	${TEST_EXE} $cfg_testparams
	# remember return code of wdtest, signals number of failures
	return $?;
}

## add test specific things after the call to callTest
# remove generated files ...
function cleanupTest
{
	echo
}
