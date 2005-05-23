/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ConfiguredActionTest_H
#define _ConfiguredActionTest_H

//---- baseclass include -------------------------------------------------
#include "ConfiguredTestCase.h"

class Context;
class Anything;

//---- ConfiguredActionTest ----------------------------------------------------------
//!<b>generic test case for configured action testing</b>
/*!
To use this testing method you have to do the following steps:
-# include "ConfiguredActionTest.h" in SetupRunner
-# add the Suite ConfiguredActionTest in setupRunner-method
-# add ConfiguredActionTestConfig.any in your tests config directory

The config's format :
<PRE>
/RunOnly {							optional, only run the specified named tests defined in RunTestCases. This always leads to one failure telling you that only selected tests were run!
	TestCaseNameToTestExclusively
	...
}
/RunTestCases {
	/FirstTestCaseName	{				the name is used by assert*m macros as testCaseName
		/TheAction {					mandatory, this script will be executed as action-script
			ActionScript to execute
		}
		/SessionStore { /Slot A ...... }		optional, all slots get copied in the Context's SessionStore and RoleStore
		/TmpStore { /Slot A ...... }			optional, all slots get copied in the Context's TmpStore
		/Query { /Slot A ...... }			optional, all slots get copied in the Context's Query
		/ExpectedResult		Boolean			optional, defaults to true, expected ret value of Action::ExecAction call
		/ExpectedToken		String			optional, defaults to "TheAction", expected value of transitionToken after Action::ExecAction call
		/Result {					optional, expected state of context stores after Action::ExecAction call
			/Delim   		String		optional, default ".", first char is taken as delimiter for named slots
			/IndexDelim		String		optional, default ":", first char is taken as delimiter for indexed slots
			/SessionStore	{ ... }			optional, SessionStore <i>lazy</i> match
			/RoleStore 		{ ... }		optional, RoleStore <i>lazy</i> match
			/Query 			{ ... } 	optional, Query <i>lazy</i> match
			/TmpStore	{ /SlotA .. /SlotB .. } optional, TmpStore <i>lazy</i> match only for defined slots - other slots in the Contexts TmpStore are ignored
		}
		/NotResult {					optional, list of anythings/paths that are not expected (i.e. must not be present)
			/Delim   		String		optional, default ".", first char is taken as delimiter for named slots
			/IndexDelim		String		optional, default ":", first char is taken as delimiter for indexed slots
			/SessionStore	{ ... }			optional, not expected entries for session store
			/RoleStore 		{ ... }		optional, not expected entries for role store
			/Query 			{ ... } 	optional, not expected entries for query store
			/TmpStore 		{ ... }		optional, not expected entries for tmp store --> see example below
		}
	}
	/NextTestCaseName { ......}
}
</PRE>
Example for /Result case in config. <i>lazy</i> means that only the slots listed recursively below /Result will get tested in the given store.
Therefore the slot /NotResult was also invented to allow explicit testing of non-existent slots.
The special name <b>ignore</b> can be used to check for slot existence but not its content. This is especially useful when results can vary.
<pre>
/Result {
	/SessionStore {
		/SlotX {
			/SlotY 345
			/SlotZ <b>ignore</b>	<b>expect slot to exist but ignore everything below</b>
		}
	}
	/TmpStore {
		/SlotA {
			abc
		}
		/SlotE <b>*</b>		<b>expect Null-Anything</b>
	}
}
</pre>
Example for /NotResult case in config
<pre>
/NotResult {
	/SessionStore {
		/SlotX { /SlotY { /SlotZ * } }
	}
	/TmpStore {
		/SlotA {
			/SlotB *
			/SlotC {
				/SlotD *
			}
		}
		/SlotE *
	}
}
</pre>
This means that the following paths must not exist in the result context:
<pre>
	* SessionStore.SlotX.SlotY.SlotZ
	* TmpStore.SlotA.SlotB
	* TmpStore.SlotA.SlotC.SlotD
	* TmpStore.SlotE
</pre>
\note You can only check for absence of named slots (so far). Values are not checked, thus * must be provided as leaf (or any other dummy) for correct syntax.
 */
class ConfiguredActionTest : public ConfiguredTestCase
{
public:
	/*! TestCase constructor
		\param name name of the test */
	ConfiguredActionTest(TString tstrName);
	/*! TestCase constructor
		\param name name of the test
		\param configFileName name of the tests config file */
	ConfiguredActionTest(TString tstrName, TString configFileName);

	//! TestCase destructor
	~ConfiguredActionTest();

	//! sets the environment for this test
	void setUp ();

	//! deletes the environment for this test
	void tearDown ();

	//! loop over the slots in ConfiguredActionTestConfig.any
	void RunTestCases();

	//--- public api
	//! builds up a suite of testcases for this test
	static Test *suite ();

protected:
	/*!	Executes the testcase
		\param testCase the test case's config
		\param testCaseName String that is printed with failure messages */
	virtual void DoTest(Anything testCase, const char *testCaseName);

	/*!	Executes the testcase with a given context
		\param testCase the test case's config
		\param testCaseName String that is printed with failure messages
		\param ctx the context to perform the test with */
	virtual void DoTest(Anything testCase, const char *testCaseName, Context &ctx);

	/*!	Executes the configured TheAction
		\param ctx client supplied context to retrieve Action results
		\param testCase the test case's config
		\param testCaseName String that is printed with failure messages */
	virtual void DoTestWithContext(Anything testCase, const String &testCaseName, Context &ctx);

	/*!	Prepares the config of the testcase
		implements config sharing, configured with /UseConfig slot
		\param originalConfig the configuration as it comes from the configfile
		\return the configuration for the testcase */
	virtual Anything PrepareConfig(Anything originalConfig);

	//! type switch für store checks
	enum eResultCheckType { exists, notExists };

	/*!	utility method to perform Checks in ctx stores
		expected has the format
		<PRE>
		{
			/SessionStore	{ .. }		# The whole content is compared to the ctxToCheck's SessionStore
			/RoleStore	{ .. }			# The whole content is compared to the ctxToCheck's RoleStore
			/TmpStore	{				# Each slot is compared to the slot with the same name
				/Slot1	{ .. }			# in ctxToCheck's TmpStore
				/Slot2	*
			}
		}</PRE>
		\param expected Anything containing expected results
		\param ctxToCheck contexts that supplies the SessionStore and TmpStore to be checked
		\param testCaseName String that is printed with failure messages
		\param rct result check type, either eResultCheckType::exists or eResultCheckType::notExists */
	virtual void DoCheckStores(ROAnything expected, Context &ctxToCheck, const char *testCaseName, eResultCheckType rct = exists);

	//! Really compare the store using AnyUtils::AnyCompareEqual
	void DoCheckStore(ROAnything anyInput, ROAnything anyMaster, const char *storeName, const char *testCaseName, char delimSlot = '.', char delimIdx = ':', eResultCheckType rct = exists);

	//! helper method to generate a list of paths out of an anything
	virtual void GeneratePathList(Anything &pathList, ROAnything &input, String pathSoFar, char delimSlot);

	//Server *fServer;
	Anything fGlobalConfig;
};

#endif
