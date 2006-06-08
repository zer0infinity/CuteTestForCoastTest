/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _NewRendererTest_H
#define _NewRendererTest_H

//---- baseclass include -------------------------------------------------
#include "WDBaseTestPolicies.h"

//---- NewRendererTest ----------------------------------------------------------
//!TestClass for Renderers
//! Rendererspecifications are defined in NewRendererTestConfig.any
//! To add a new TestCase append a new Entry to the <B>/TestCases</B> slot.
//! Testcase entry specification :
//! <PRE>{
//!		/Renderer 	Rendererspecification	# ! Use full Renderer Classnames
//!		/Env		Anything that serves as Context for this TestCase
//!		/Expected	String
//! }</PRE>
//! There is also a Toplevel slot <B>/EnvForAllCases</B> specifing an Anything that serves
//! as Context for all TestCases
//class NewRendererTest : public TestFramework::TestCaseWithConfig
class NewRendererTest : public TestFramework::TestCaseWithGlobalConfigDllAndModuleLoading
{
public:
	//--- constructors
	NewRendererTest(TString tstrName);
	~NewRendererTest();

	//--- public api
	//!generate NewRendererTest test suite
	//! \return a new test is created by this method
	static Test *suite ();

	TString getConfigFileName() {
		return "NewRendererTestConfig";
	}

	//!Testcase runs all configured RendererSpecs
	void TestCases();
};

#endif
