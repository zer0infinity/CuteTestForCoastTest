#!/bin/ksh

## add test specific things before the call to callTest
function prepareTest
{
	echo
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
