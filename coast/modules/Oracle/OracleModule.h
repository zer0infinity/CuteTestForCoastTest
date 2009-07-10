/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _OracleModule_H
#define _OracleModule_H

//---- WDModule include -------------------------------------------------
#include "config_coastoracle.h"
#include "WDModule.h"
#include "ConnectionPool.h"
#include <memory>

//---- OracleModule ----------------------------------------------------------
//! <B>Module to initialize Oracle Data Access</B>
/*!
\par Configuration
\code
{
	/ConnectionPool {							optional, ConnectionPool configuration, if not specified, default values get used
		/ParallelQueries			long		defines number of parallel queries/sp-calls which can be issued, default 5
		/CloseConnectionTimeout		long		timeout after which to close open connections
	}
}
\endcode
*/
class EXPORTDECL_COASTORACLE OracleModule : public WDModule
{
public:
	//--- constructors
	OracleModule(const char *name);
	~OracleModule();

	//:implementers should initialize module using config
	virtual bool Init(const ROAnything config);
	//:implementers should terminate module expecting destruction
	virtual bool Finis();

	Coast::Oracle::ConnectionPool *GetConnectionPool();

private:
	typedef std::auto_ptr<Coast::Oracle::ConnectionPool> ConnectionPoolPtr;
	ConnectionPoolPtr fpConnectionPool;
};

#endif
