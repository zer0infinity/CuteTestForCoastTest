#!/bin/ksh

## add test specific things before the call to callTest
function prepareTest
{
	echo
}

## call to wdtest or whatever you want to call
function callTest
{
	# only run Test when not started within DailyBuild
	if [ -z "$DAILYBUILDRUNNING" ]; then
		${TEST_EXE} $cfg_testparams
		# remember return code of wdtest, signals number of failures
		return $?;
	else
		echo 'No Test within DailyBuild';
		return 0;
	fi
}

## add test specific things after the call to callTest
# remove generated files ...
function cleanupTest
{
	echo
}
