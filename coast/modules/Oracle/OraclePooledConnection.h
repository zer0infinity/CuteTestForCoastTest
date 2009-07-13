/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef O8CONNECTION_H_
#define O8CONNECTION_H_

//--- modules used in the interface
#include "config_coastoracle.h"
#include "ITOString.h"
#include "IFAObject.h"

#include "OciAutoHandle.h"

//---- OraclePooledConnection -----------------------------------------------------------
// connection to oracle db ... not thread safe... may not be used concurrently

class EXPORTDECL_COASTORACLE OraclePooledConnection: public IFAObject
{
	typedef std::auto_ptr<OracleEnvironment> OracleEnvironmentPtr;
	typedef std::auto_ptr<OracleConnection> OracleConnectionPtr;

	OracleEnvironmentPtr fEnvironment;
	OracleConnectionPtr fConnection;
public:
	OraclePooledConnection();
	~OraclePooledConnection();

	bool Open(String const &strServer, String const &strUsername, String const &strPassword);
	bool Close(bool bForce = false);

	OracleEnvironment *getEnvironment() const {
		return fEnvironment.get();
	}
	OracleConnection *getConnection() const {
		return fConnection.get();
	}

	String errorMessage(sword status);
	bool checkError(sword status, String &message);
	bool checkError(sword status);

protected:
	// returns nothing, object not cloneable
	IFAObject *Clone() const {
		return NULL;
	};
};

#endif /* O8CONNECTION_H_ */
