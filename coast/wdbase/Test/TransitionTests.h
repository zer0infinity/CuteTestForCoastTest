/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _TransitionTests_H
#define _TransitionTests_H

#include "WDBaseTestPolicies.h"

//---- forward declaration -----------------------------------------------

//---- TransitionTests ----------------------------------------------------------
//!TestCases for Coast state transitions
class TransitionTests : public TestFramework::TestCaseWithGlobalConfigDllAndModuleLoading
{
public:
	//--- constructors

	//!TestCases
	//! \param name name of the test
	TransitionTests(TString tstrName);

	//!destroys the test case
	~TransitionTests();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	TString getConfigFileName();

	//! main
	void RunRequestSequence();

	void PBOWLoginSequence1();
	void PBOWLoginSequence2();
	void PBOWFailedBookmarkSequence();
	void PBOWBookmarkSequence();

protected:
	Anything	fBookmarkedRequest;

	bool EvalRequest(ROAnything request, Anything &returned);
	Anything AddSessionInfo(ROAnything request, Anything context);
};

#endif
