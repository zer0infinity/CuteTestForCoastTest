/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "OraclePooledConnection.h"
#include "SystemLog.h"
#include "Dbg.h"
#include <string.h>		// for strlen
//----- OraclePooledConnection -----------------------------------------------------------------
OraclePooledConnection::OraclePooledConnection( u_long lId, u_long lPoolSize, u_long lPoolBuckets ) :
	fId( lId ), fPoolSize(lPoolSize), fPoolBuckets(lPoolBuckets)
{
	StatTrace(OraclePooledConnection.OraclePooledConnection, "empty", Coast::Storage::Current());
}

OraclePooledConnection::~OraclePooledConnection()
{
	StatTrace(OraclePooledConnection.~OraclePooledConnection, "closing connection", Coast::Storage::Current());
	// disconnect if OracleConnection exists
	Close( true );
}

bool OraclePooledConnection::Open( String const &strServer, String const &strUsername, String const &strPassword )
{
	StartTrace1(OraclePooledConnection.Open, "server [" << strServer << "] user [" << strUsername << "]");
	if ( !fEnvironment.get() ) {
		fEnvironment = OracleEnvironmentPtr( new (Coast::Storage::Global()) OracleEnvironment(
				OracleEnvironment::THREADED_UNMUTEXED, fId, fPoolSize, fPoolBuckets ) );
	}
	if ( fEnvironment.get() && fEnvironment->valid() ) {
		if ( !fConnection.get() )
			fConnection = OracleConnectionPtr( fEnvironment->createConnection( strServer,
											   strUsername, strPassword ) );
		else {
			fConnection->Open( strServer, strUsername, strPassword );
		}
		fServer = strServer;
		fUser = strUsername;
	}
	return fConnection.get();
}

bool OraclePooledConnection::Close( bool bForce )
{
	StatTrace(OraclePooledConnection.Close, (bForce ? "" : "not ") << "forcing connection closing", Coast::Storage::Current());
	if ( fConnection.get() ) {
		fConnection->Close();
	}
	if ( bForce ) {
		fConnection.reset();
		fEnvironment.reset();
	}
	return true;
}
