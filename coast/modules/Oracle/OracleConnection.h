/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef ORACLECONNECTION_H_
#define ORACLECONNECTION_H_

//--- modules used in the interface
#include "config_coastoracle.h"
#include "Anything.h"
#include "OciAutoHandle.h"

class OracleEnvironment;
class OracleStatement;

//---- OracleConnection -----------------------------------------------------------
//! <b>Abstraction for an Oracle connection</b>
/*!
 * @par Configuration
 * --
 * @par Documentation
 * This class serves as abstraction for all the OCI calls which need to be done to establish a connection to an oracle back end.
 * The OracleConnection itself will be created when calling OracleEnvironment::createConnection. All this is done from within
 * OraclePooledConnection which will be tracked by Coast::Oracle::ConnectionPool.
 * The main functions this class serves for is to OracleConnection::Open and OracleConnection::Close the connection
 * to the back end and to let OracleConnection::createStatement give us an OracleStatement. The statement is the holder for
 * any of simple statements up to complex StoredProcedure calls.
 */
class EXPORTDECL_COASTORACLE OracleConnection
{
	//! track the connection status
	bool fConnected;
	//! hold a reference to the surrounding environment
	OracleEnvironment &fOracleEnv;
	//! OCI error handle
	ErrHandleType fErrhp;
	//! OCI server connection handle
	SrvHandleType fSrvhp;
	//! OCI service context handle
	SvcHandleType fSvchp;
	//! OCI user session handle
	UsrHandleType fUsrhp;
	//! OCI description handle
	DscHandleType fDschp;
public:
	/*! Main construction entry point
	 * @param rEnv the surrounding OracleEnvironment which was used to create us
	 */
	OracleConnection( OracleEnvironment &rEnv );
	/*! Destruction of the connection, close if needed */
	~OracleConnection();

	/*! Open a connection to the back end
	 *
	 * @param strServer Server connection string to connect with
	 * @param strUsername Username to connect as
	 * @param strPassword Password for the given user
	 * @return true in case we could establish a connection to the back end
	 */
	bool Open( String const &strServer, String const &strUsername, String const &strPassword );
	/*! Close the connection to the backend if it was opened before
	 */
	void Close();

	/*! Gain access to the surrounding OracleEnvironment class
	 *
	 * @return Reference to the OracleEnvironment which was used to create this OracleConnection
	 */
	OracleEnvironment &getEnvironment() const {
		return fOracleEnv;
	}

	/*! Create a new OracleStatement object based on this connection
	 *
	 * @param strStatement Statement string to use for the newly created OracleStatement object
	 * @param lPrefetchRows How many rows to fetch in a server round trip, can be used to optimize performance on a per statement basis
	 * @param roaSPDescription Needs to be supplied if the statement is a stored procedure or function. This data is needed to supply the correct parameter names and types.
	 * @return newly created OracelStatment in case of success
	 */
	OracleStatement *createStatement( String const &strStatement, long lPrefetchRows, ROAnything roaSPDescription =
										  ROAnything() );

	/*! access OCIError handle
	 *
	 * @return OCIError handle
	 */
	OCIError *ErrorHandle() {
		return fErrhp.getHandle();
	}

	/*! access OCISvcCtx handle
	 *
	 * @return OCISvcCtx handle
	 */
	OCISvcCtx *SvcHandle() {
		return fSvchp.getHandle();
	}

	/*! access OCIDescribe handle
	 *
	 * @return OCIDescribe handle
	 */
	OCIDescribe *DscHandle() {
		return fDschp.getHandle();
	}

	/*! check the given status value against an error condition and report it as message
	 *
	 * @param status OCI status value to check
	 * @param message String with appropriate error message filled in
	 * @return true in case there was an error
	 */
	bool checkError( sword status, String &message );

	/*! Return an error message string for the given status value
	 *
	 * @param status OCI status value to use
	 * @return Appropriate error message
	 */
	String errorMessage( sword status );

	/*! check the given status value against an error condition
	 *
	 * @param status OCI status value to check
	 * @return true in case there was an error
	 */
	bool checkError( sword status );
};

#endif /* ORACLECONNECTION_H_ */
