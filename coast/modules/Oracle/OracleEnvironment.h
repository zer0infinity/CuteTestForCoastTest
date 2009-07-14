/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef ORACLEENVIRONMENT_H_
#define ORACLEENVIRONMENT_H_

//--- modules used in the interface
#include "config_coastoracle.h"

#include "OciAutoHandle.h"

class OracleConnection;
class String;

class EXPORTDECL_COASTORACLE OracleEnvironment
{
	EnvHandleType fEnvhp; // OCI environment handle
public:
	OracleEnvironment();
	virtual ~OracleEnvironment();

	OCIEnv *EnvHandle() {
		return fEnvhp.getHandle();
	}

	bool valid() const {
		return fEnvhp;
	}

	OracleConnection *createConnection( String const &strSrv, String const &strUsr, String const &strPwd );
};

#endif /* ORACLEENVIRONMENT_H_ */
