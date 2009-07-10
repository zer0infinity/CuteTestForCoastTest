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
#include "IFAObject.h"
#include "OciAutoHandle.h"

class String;

//---- OracleConnection -----------------------------------------------------------
// connection to oracle db

class EXPORTDECL_COASTORACLE OracleConnection: public IFAObject
{
public:
	OracleConnection();
	~OracleConnection();

	bool Open(String const &strServer, String const &strUsername, String const &strPassword);
	bool Close(bool bForce = false);

	oracle::occi::Connection *getConnection() const {
		return fConnection.get();
	}

protected:
	// returns nothing, object not cloneable
	IFAObject *Clone() const {
		return NULL;
	};

	// --- oracle API
	oracle::occi::Environment *fpEnvironment;
	ConnectionPtrType fConnection;
};

#endif /* O8CONNECTION_H_ */
