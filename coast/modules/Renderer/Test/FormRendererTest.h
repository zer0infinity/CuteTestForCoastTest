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

class Anything;
class MetaThing;
class Context;
class OStringStream;
class String;

class TestRole : public Role
{
public:
	TestRole(const char *name) : Role(name) {}
};

//---- FormRendererTest -----------------------------------------------------------
class FormRendererTest : public TestCase
{
protected:

public:
	FormRendererTest (TString name); // : TestCase (name) {}
	virtual ~FormRendererTest();
	virtual void	setUp ();
	static Test		*suite ();

protected:
	Anything		fConfig;
	MetaThing		fEnvironment;
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
