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
#include "ITOString.h"
#include "IFAObject.h"

#include "OciAutoHandle.h"

class OracleEnvironment;

//---- OraclePooledConnection -----------------------------------------------------------
// connection to oracle db ... not thread safe... may not be used concurrently

class EXPORTDECL_COASTORACLE OracleConnection
{
	bool fConnected;
	OracleEnvironment &fOracleEnv;
	ErrHandleType fErrhp; // OCI error handle
	SrvHandleType fSrvhp; // OCI server connection handle (at most one outstanding call at a time!)
	SvcHandleType fSvchp; // OCI service context handle
	UsrHandleType fUsrhp; // OCI user session handle
	DscHandleType fDschp;
public:
	OracleConnection(OracleEnvironment &rEnv);
	~OracleConnection();

	bool Open(String const &strServer, String const &strUsername, String const &strPassword);
	void Close();

private:
	OCIError *ErrorHandle() {
		return fErrhp.getHandle();
	}

	OCISvcCtx *SvcHandle() {
		return fSvchp.getHandle();
	}

	OCIDescribe *DscHandle() {
		return fDschp.getHandle();
	}
	String errorMessage(sword status);
	bool checkError(sword status, String &message);
	bool checkError(sword status);
};

#endif /* ORACLECONNECTION_H_ */
