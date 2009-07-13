/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef ORACLEENVIRONMENT_H_
#define ORACLEENVIRONMENT_H_

class OracleEnvironment
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

	OracleConnection createConnection( String const &strUsr, String const &strPwd, String const &strSrv );

};

#endif /* ORACLEENVIRONMENT_H_ */
