/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _FormRendererTest_h_
#define _FormRendererTest_h_

#include "TestCase.h"
#include "Role.h"
#include "FormRenderer.h"
#include "StringStream.h"

class TestRole : public Role
{
public:
	TestRole(const char *name) : Role(name) {}
};

//---- FormRendererTest -----------------------------------------------------------
class FormRendererTest : public testframework::TestCase
{
protected:

public:
	FormRendererTest (TString tstrName);
	virtual ~FormRendererTest();
	virtual void	setUp ();
	static Test		*suite ();

protected:
	Anything		fConfig;
	Anything		fEnvironment;
	TestRole		fRole;
	Context			fContext;
	OStringStream	fReply;
	FormRenderer	fFormRenderer;
	String			fCurrentTestMethod;

	void PrintResult();

	void ConfigureField6();
	void TestCase0();
};

#endif
