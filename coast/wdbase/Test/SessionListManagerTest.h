/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SessionListManagerTest_H
#define _SessionListManagerTest_H

//---- baseclass include -------------------------------------------------
#include "WDBaseTestPolicies.h"
#include "Session.h"

//---- SessionListManagerTest ----------------------------------------------------------
//!TestCases description
class SessionListManagerTest : public TestFramework::TestCaseWithGlobalConfigDllAndModuleLoading
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	SessionListManagerTest(TString tstrName);

	//!destroys the test case
	~SessionListManagerTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp();

	//!deletes the environment for this test
	void tearDown ();

	//!test initialization and termination
	void InitFinisTest();
	//!test reinitialization
	void ReInitTest();
	//!test creation of sessions
	void CreateSessionTest();
	//!test lookup of sessions
	void LookupSessionTest();
	//!test disabling of sessions
	void DisableSessionTest();
	//!test deletion of sessions
	void CleanupSessionTest();
	//!test Filtering of sessions
	void StdFilterQueryTest();
	//!test information access about sessions
	void SessionListInfoTest();
	//!test information access about one session
	void GetASessionsInfoTest();
	//!test creation of sessions
	void SessionFactoryTest();
	//!test accounting of sessions
	void SessionAccountingTest();
	//!test generation of unique id
	void UniqueInstanceIdTest();

	//!test accounting of sessions, add them twice
	void AddSameSessionNTimesTest();

	//!callbacks from TestObjects
	void NotifyCalled(Session::ESessionEvt evt, Context &ctx);

	//!callbacks from TestObjects
	void DoPrepareSessionCalled(Context &ctx, Session *session);
};

#endif
