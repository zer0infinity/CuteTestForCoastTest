#!/bin/ksh
#-----------------------------------------------------------------------------------------------------
# Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
# All rights reserved.
#
# This library/application is free software; you can redistribute and/or modify it under the terms of
# the license that is included with this library/application in the file license.txt.
#-----------------------------------------------------------------------------------------------------

#DB_SHELL=$DEV_HOME/3rdparty/installed/oracle/bin/${OSREL}/sqlplus
DB_SHELL=sqlplus
ConnectString=oratest/oratest@sifs-coast1.hsr.ch:1521/orcl
DB_IMPORTFILE='config/oratest_schema.sql'
DB_DROPFILE='config/drop_oratest_schema.sql'

## add test specific things before the call to callTest
function prepareTest
{
	$DB_SHELL ${ConnectString} < ${DB_DROPFILE}
	$DB_SHELL ${ConnectString} < ${DB_IMPORTFILE}
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
	$DB_SHELL ${ConnectString} < ${DB_DROPFILE}
	echo
}
