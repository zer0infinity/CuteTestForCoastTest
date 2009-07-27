/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef ORACLEPOOLEDCONNECTION_H_
#define ORACLEPOOLEDCONNECTION_H_

//--- modules used in the interface
#include "config_coastoracle.h"
#include "OracleEnvironment.h"
#include "OracleConnection.h"
#include "ITOString.h"
#include "IFAObject.h"
#include <memory>	// for auto_ptr

//---- OraclePooledConnection -----------------------------------------------------------
//! <b>Connection adapter to handle Oracle specific connections using Coast::Oracle::ConnectionPool</b>
/*!
 * This class deals with oracle connection specific things like OracleEnvironment and OracleConnection. When
 * such an object is first requested for OraclePooledConnection::Open, it will allocate the needed resources
 * like an environment and a connection which can then be used to process oracle database requests.
 * @par Configuration
 * --
 */
class EXPORTDECL_COASTORACLE OraclePooledConnection: public IFAObject
{
	typedef std::auto_ptr<OracleEnvironment> OracleEnvironmentPtr;
	typedef std::auto_ptr<OracleConnection> OracleConnectionPtr;

	OracleEnvironmentPtr fEnvironment;
	OracleConnectionPtr fConnection;
public:
	/*! Default ctor
	 */
	OraclePooledConnection();
	/*! Close connection and free allocated resources
	 */
	~OraclePooledConnection();

	/*! Create and initialize necessary objects for oracle database access
	 *
	 * @param strServer Server connection string to connect with
	 * @param strUsername Username to connect as
	 * @param strPassword Password for the given user
	 * @return true in case we could create necessary objects
	 */
	bool Open( String const &strServer, String const &strUsername, String const &strPassword );
	/*! Close the connection to the backend if it was opened before
	 * @param bForce when set to true, close the current connection to the back end regardless if a statement is still
	 * executing.
	 * @return true in case it was successful
	 */
	bool Close( bool bForce = false );

	/*! Check whether the underlying connection is open
	 * @return true if we are connected to the back end */
	bool isOpen() const {
		return ( fConnection.get() && fConnection->isOpen() );
	}

	/*! Access the associated environment object
	 * @return  pointer to OracleEnvironment
	 */
	OracleEnvironment *getEnvironment() const {
		return fEnvironment.get();
	}
	/*! Access the associated connection object
	 * @return  pointer to OracleConnection
	 */
	OracleConnection *getConnection() const {
		return fConnection.get();
	}

protected:
	//! returns nothing, object not to clone
	IFAObject *Clone() const {
		return NULL;
	}
};

#endif /* ORACLEPOOLEDCONNECTION_H_ */
