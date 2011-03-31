/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ContextTest_H
#define _ContextTest_H

#include "WDBaseTestPolicies.h"

class Registry;

//---- ContextTest ----------------------------------------------------------
//!TestCases for the Context class
class ContextTest : public TestFramework::TestCaseWithGlobalConfigDllAndModuleLoading
{
public:
	//--- constructors

	//!TestCases for the Context class
	ContextTest(TString tstrName);

	//!destroys the test case
	~ContextTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	//!test constructor with an anything containing request infos
	void RequestConstructorTest();

	//!test constructor with a socket
	void SocketCtorTests();

	//!test the empty constructor
	void EmptyConstructorTest();

	//!test the lookup functionality
	void LookupTests();
	void SimplePushNoPop();
	void SimpleNamedPushPop();
	void VerySimplePush();
	//!test the lookup functionality
	void RequestSettingTest();

	//!test the push/pop functionality of stores
	void StoreTests();
	void SimplePushPop();
	void SimplePushWithEmptyStore();
	void MoreThanOnePush();
	void MoreThanOnePushWithSameKey();
	void MoreThanOnePushWithSameKeyPrefix();

	//!test the replace of named lookupables functionality
	void FindReplace();

	//!test the remove of named lookupables
	void RemoveTest();

	//!test the session push/lookup functionality
	void SessionPushTest();

	//!test working of session store using more than one context
	void SessionStoreTest();

	//!test working of role store using more than one context
	void RoleStoreTest();

	//!test access to pushed named objects
	void ObjectAccessTests();

	//!Test the canonical Setters and Getters
	void SetNGetPage();
	void SetNGetRole();

	//!Test the Session refcounting maintained by context
	void RefCountTest();
	//!Test the Session unlocking mechanism within Page Rendering
	void SessionUnlockingTest();
};

#endif
