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
#include "AllocatorNewDelete.h"
#include "OracleEnvironment.h"
#include "OracleConnection.h"
#include "ITOString.h"
#include "IFAObject.h"

//---- OraclePooledConnection -----------------------------------------------------------
//! Connection adapter to handle Oracle specific connections using Coast::Oracle::ConnectionPool
/*!
 * This class deals with oracle connection specific things like OracleEnvironment and OracleConnection. When
 * such an object is first requested for OraclePooledConnection::Open, it will allocate the needed resources
 * like an environment and a connection which can then be used to process oracle database requests.
 */
class OraclePooledConnection: public IFAObject
{
	OracleEnvironmentPtr fEnvironment;
	OracleConnectionPtr fConnection;
	unsigned long fId, fPoolSize, fPoolBuckets;
	String fServer, fUser;
	OraclePooledConnection(const OraclePooledConnection &);
	OraclePooledConnection &operator=(const OraclePooledConnection &);
public:
	/*! Default ctor
	 */
	OraclePooledConnection(u_long lId, u_long lPoolSize, u_long lPoolBuckets);
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

	/*! Check whether the underlying connection is open
	 *
	 * @param strServer Server connection string to verify
	 * @param strUsername Username to verify
	 * @return true if we are connected to the back end matching server and user too */
	bool isOpenFor(String const &strServer, String const &strUsername) {
		return fServer.IsEqual( strServer ) && fUser.IsEqual( strUsername ) && isOpen();
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

	const String &getServer() const {
		return fServer;
	}
	const String &getUser() const {
		return fUser;
	}

protected:
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return NULL;
	}
};

#endif /* ORACLEPOOLEDCONNECTION_H_ */
