/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _CopyActionsTest_h_
#define _CopyActionsTest_h_

#include "FoundationTestTypes.h"

class Context;
class String;

//---- CopyActionsTest -----------------------------------------------------------

class CopyActionsTest : public TestFramework::TestCaseWithConfig
{
public:
	CopyActionsTest(TString tstrName);
	virtual ~CopyActionsTest();

	static Test *suite();

	TString getConfigFileName();
	void CopyActionTest();

protected:
	String ExecAction(String token, Context &c, bool expectedResult);
};

#endif
