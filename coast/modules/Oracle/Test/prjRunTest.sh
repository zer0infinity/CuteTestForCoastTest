#!/bin/ksh
#-----------------------------------------------------------------------------------------------------
# Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
# All rights reserved.
#
# This library/application is free software; you can redistribute and/or modify it under the terms of
# the license that is included with this library/application in the file license.txt.
#-----------------------------------------------------------------------------------------------------

DB_SHELL=$( cd $DEV_HOME/3rdparty/installed/oracle/bin && cd $( ${SNIFF_DIR}/make_support/osreldir.sh ${MAKE_64BIT} ) && pwd -P )/sqlplus
#DB_SHELL=sqlplus
ConnectStringTestWrite=hikudevtestwrite/dhiku80_HIKUdevtestWRITE@(description=(address_list=(address=(protocol=TCP)(host=sczdhiku02)(port=1521)))(connect_data=(service_name=dhiku80)))
ConnectString=hikudevtest/Welc0me@(description=(address_list=(address=(protocol=TCP)(host=sczdhiku02)(port=1521)))(connect_data=(service_name=dhiku80)))

DB_IMPORTFILE='config/oratest_schema.sql'
DB_DROPFILE='config/oratest_dropschema.sql'
DB_CREATEUSERFILE='config/oratest_createuser.sql'

DB_GRANTFILE='config/oratest_grant.sql'
DB_CREATESYNONYMFILE='config/oratest_synonym.sql'

export NLS_LANG=".WE8ISO8859P1"

## add test specific things before the call to callTest
function prepareTest
{
	# won't work, must be done under SYSDBA rights:
	# $DB_SHELL ${ConnectString} < ${DB_CREATEUSERFILE}

	# Drop an reload the schema, logged in as hikudevtest
	$DB_SHELL ${ConnectString} < ${DB_DROPFILE}
	$DB_SHELL ${ConnectString} < ${DB_IMPORTFILE}

	# grants for hikudevtestwrite, logged in as hikudevtest
	$DB_SHELL ${ConnectString} < ${DB_GRANTFILE}

	# Logged in as hikudevtestwrite: must create synonym for user hikudevtestwrite (so that tests can run under hikudevtest user)
	$DB_SHELL ${ConnectStringTestWrite} < ${DB_CREATESYNONYMFILE}
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
