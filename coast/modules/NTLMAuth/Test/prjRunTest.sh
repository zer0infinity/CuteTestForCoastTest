#!/bin/ksh

## enable and write this function when you need to test for other files than the one
## defined in $TEST_EXE
## return 1 upon successful test and 0 otherwise
## this function gets called from within RunTests.sh when the option -c is given
#function checkTestExe
#{
#}

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
