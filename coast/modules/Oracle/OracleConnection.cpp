/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "OracleConnection.h"

#include "SysLog.h"
#include "Dbg.h"

#include <string.h>		// for strlen

using namespace oracle::occi;

//----- OracleConnection -----------------------------------------------------------------
OracleConnection::OracleConnection() :
	fpEnvironment( 0 ), fConnection()
{
	StartTrace(OracleConnection.OracleConnection);
}

OracleConnection::~OracleConnection()
{
	StartTrace(OracleConnection.~OracleConnection);
	// disconnect if OracleConnection exists
	Close();
}

bool OracleConnection::Open( String const &strServer, String const &strUsername, String const &strPassword )
{
	StartTrace(OracleConnection.Open);

	if ( fConnection ) {
		SYSERROR("tried to open already opened connection to server [" << strServer << "] and user [" << strUsername << "]!");
		return false;
	}

	// caution: if memory handles are supplied, they must allocate on Storage::Global()
	//  because memory gets allocated through them in Open and freed in Close. Throughout the
	//  lifetime of the connection, multiple threads could share the same connection and so we
	//  must take care not to allocate on the first Thread opening the connection
	if ( !fpEnvironment ) {
		fpEnvironment = Environment::createEnvironment( Environment::THREADED_UNMUTEXED );
		//			ctxp, // context pointer passed into user specific mem functions
		//			malloc_func, // malloc function to allocate handles and env specific memory
		//			realloc_func, // realloc function to allocate handles and env specific memory
		//			free_func, // free function to allocate handles and env specific memory
	}

	if ( !fConnection ) {
		std::string ssUsr( (const char *)strUsername ), ssPwd( (const char *)strPassword ), ssSrv( (const char *) strServer );
		ConnectionPtrType conn(fpEnvironment->createConnection( ssUsr, ssPwd, ssSrv ), fpEnvironment, &Environment::terminateConnection);
		fConnection = conn;
	}
	return ( fpEnvironment && fConnection );
}

bool OracleConnection::Close( bool bForce )
{
	StartTrace1(OracleConnection.Close, (bForce ? "" : "not ") << "forcing connection closing, connection is " << (fConnection ? "good" : "bad"));
	if ( fConnection ) {
		Trace("terminating connection");
		fConnection.reset();
		if ( fpEnvironment ) {
			Trace("terminating environment");
			Environment::terminateEnvironment( fpEnvironment );
		}
	}
	return true;
}
