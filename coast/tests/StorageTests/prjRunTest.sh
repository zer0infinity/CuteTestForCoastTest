#-----------------------------------------------------------------------------------------------------
# Copyright (c) 2007, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
# All rights reserved.
#
# This library/application is free software; you can redistribute and/or modify it under the terms of
# the license that is included with this library/application in the file license.txt.
#-----------------------------------------------------------------------------------------------------

## enable and write this function when you need to test for other files than the one
## defined in $TEST_EXE
## return 1 upon successful test and 0 otherwise
## this function gets called from within RunTests.sh when the option -c is given
#function checkTestExe
#{
#}

## add test specific things before the call to callTest
prepareTest()
{
	chmod u+w $CONFIGDIR/BoostPoolTest.stat.any
}

## call to wdtest or whatever you want to call
callTest()
{
	${TEST_EXE} $cfg_testparams
	# remember return code of wdtest, signals number of failures
	return $?;
}

## add test specific things after the call to callTest
# remove generated files ...
cleanupTest()
{
	echo
}
