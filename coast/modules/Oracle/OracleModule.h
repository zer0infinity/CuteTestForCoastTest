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
#include "WDModule.h"
#include "ConnectionPool.h"
#include <memory>

//---- OracleModule ----------------------------------------------------------
//! Module to initialize Oracle Data Access
/*!
 * OracleModule is the module needed to initialize when wanting to access Oracle database server back ends. The
 * configuration needed is the configuration for Coast::Oracle::ConnectionPool which is best looked up there. Main
 * purpose of this module is to provide proper WDModule::Init and WDModule::Finis code to setup and cleanup all
 * the needed parts. In this case, initialization is delegated to Coast::Oracle::ConnectionPool which sets up its
 * connection pool to fulfill OracleDAImpl requests.
 * @section oms1 OracleModule configuration
 @code
 /OracleModule {
	 /ConnectionPool {...}
 }
 @endcode
 * @par \c ConnectionPool
 * Optional\n
 * @see @ref cps1
 */
class OracleModule: public WDModule
{
	typedef std::auto_ptr<Coast::Oracle::ConnectionPool> ConnectionPoolPtr;
	ConnectionPoolPtr fpConnectionPool;
public:
	/*! Name using ctor to register OracleModule in the Registry using the correct name
	 * @param name Used to register in Registry
	 */
	OracleModule( const char *name );
	/*! terminates oracle services */
	~OracleModule();

	/*! Currently the only way to gain access to the associated Coast::Oracle::ConnectionPool instance
	 * @return pointer to the Coast::Oracle::ConnectionPool
	 */
	Coast::Oracle::ConnectionPool *GetConnectionPool();
protected:
	/*! Initialization method called when servers boots up
	 * @param config Global server configuration to be used for initialization
	 * @return true in case we could successfully initialize what we wanted
	 */
	virtual bool Init( const ROAnything config );
	/*! Finalizing method to properly terminate oracle related services when server wants to shut down
	 * @return true in case termination was successful
	 */
	virtual bool Finis();
};

#endif
