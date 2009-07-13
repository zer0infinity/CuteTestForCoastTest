/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "OraclePooledConnection.h"

#include "SysLog.h"
#include "Dbg.h"

#include <string.h>		// for strlen
static struct OraTerminator {
	~OraTerminator() {
		OCITerminate( OCI_DEFAULT );
	}
} fgOraTerminator;

//----- OraclePooledConnection -----------------------------------------------------------------
OraclePooledConnection::OraclePooledConnection()
{
	StartTrace(OraclePooledConnection.OraclePooledConnection);
}

OraclePooledConnection::~OraclePooledConnection()
{
	StartTrace(OraclePooledConnection.~OraclePooledConnection);
	// disconnect if OraclePooledConnection exists
	Close();
}

bool OraclePooledConnection::Open( String const &strServer, String const &strUsername, String const &strPassword )
{
	StartTrace(OraclePooledConnection.Open);

	if ( !fEnvironment ) {
		fEnvironment = OracleEnvironmentPtr(new OracleEnvironment());
		if ( fEnvironment->valid() ) {
			fConnection = OracleConnectionPtr( fEnvironment->createConnection(strServer, strUsername, strPassword) );
		}
	}
}

bool OraclePooledConnection::Close( bool bForce )
{
	StartTrace1(OraclePooledConnection.Close, (bForce ? "" : "not ") << "forcing connection closing");
	return true;
}

bool OraclePooledConnection::checkError( sword status )
{
	StartTrace1(OraclePooledConnection.checkError, "status: " << (long) status);
	if ( status == OCI_SUCCESS || status == OCI_NO_DATA ) {
		return false;
	}
	return true;
}

bool OraclePooledConnection::checkError( sword status, String &message )
{
	message = errorMessage( status );
	return checkError( status );
}

String OraclePooledConnection::errorMessage( sword status )
{
	// error handling: checks 'status' for errors
	// in case of an error an error message is generated

	text errbuf[512];
	sb4 errcode;
	String error;

	switch ( status ) {
		case OCI_NO_DATA:
		case OCI_SUCCESS:
			// no error
			return error;
			break;
		case OCI_SUCCESS_WITH_INFO:
			error << "Error - OCI_SUCCESS_WITH_INFO";
			break;
		case OCI_NEED_DATA:
			error << "Error - OCI_NEED_DATA";
			break;
		case OCI_ERROR:
			if ( fErrhp.getHandle() )
				OCIErrorGet( (dvoid *) fErrhp.getHandle(), (ub4) 1, (text *) NULL, &errcode, errbuf,
							 (ub4) sizeof ( errbuf ), (ub4) OCI_HTYPE_ERROR );
			error << "Error - " << (char *) errbuf;
			break;
		case OCI_INVALID_HANDLE:
			error << "Error - OCI_INVALID_HANDLE";
			break;
		case OCI_STILL_EXECUTING:
			error << "Error - OCI_STILL_EXECUTE";
			break;
		case OCI_CONTINUE:
			error << "Error - OCI_CONTINUE";
			break;
		default:
			break;
	}

	return error;
}
