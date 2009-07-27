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
	// disconnect if OracleConnection exists
	Close( true );
}

bool OraclePooledConnection::Open( String const &strServer, String const &strUsername, String const &strPassword )
{
	StartTrace(OraclePooledConnection.Open);

	if ( !fEnvironment.get() ) {
		fEnvironment = OracleEnvironmentPtr(new OracleEnvironment(OracleEnvironment::THREADED_UNMUTEXED));
	}
	if ( fEnvironment.get() && fEnvironment->valid() ) {
		if ( !fConnection.get() ) {
			fConnection = OracleConnectionPtr( fEnvironment->createConnection(strServer, strUsername, strPassword) );
		} else {
			fConnection->Open(strServer, strUsername, strPassword);
		}
	}
	return fConnection.get();
}

bool OraclePooledConnection::Close( bool bForce )
{
	StartTrace1(OraclePooledConnection.Close, (bForce ? "" : "not ") << "forcing connection closing");
	if ( bForce ) {
		fConnection.reset();
		fEnvironment.reset();
	} else if ( fConnection.get() ) {
		fConnection->Close();
	}
	return true;
}
