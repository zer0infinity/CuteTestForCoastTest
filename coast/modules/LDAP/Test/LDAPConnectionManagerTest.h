/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _LDAPConnectionManagerTest_H
#define _LDAPConnectionManagerTest_H

//---- baseclass include -------------------------------------------------
#include "ConfiguredActionTest.h"
#include "LDAPConnection.h"

//---- LDAPConnectionManagerTest ----------------------------------------------------------
//! <B>Tests LDAPConnecton and LDAPErrorHandler</B>
class LDAPConnectionManagerTest : public ConfiguredActionTest
{
public:
	//--- constructors

	/*! \param name name of the test and its configuration file */
	LDAPConnectionManagerTest(TString tstrName);

	//! destroys the test case
	~LDAPConnectionManagerTest();

	//--- public api

	//! builds up a suite of tests
	static Test *suite ();

	TString getConfigFileName();

	//! describe this Test
	void ConnectionManagerTest();
	void AutoRebindTest();
	void NoAutoRebindTest();

private:
	LDAP *CreateBadConnectionHandle(const String &name, String &badConnectionPoolId, long &maxBadConnections);
	String fetchPoolId(ParameterMapper *pGetter);
};

#endif
