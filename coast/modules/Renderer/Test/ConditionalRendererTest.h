/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ConditionalRendererTest_h_
#define _ConditionalRendererTest_h_

//-*-Mode: C++;-*-

#include "RendererTest.h"

//---- ConditionalRendererTest -----------------------------------------------------------

class ConditionalRendererTest : public RendererTest
{
public:
	ConditionalRendererTest(TString name);
	virtual ~ConditionalRendererTest();

	static Test *suite ();
	void setUp ();
protected:
	void ConfigureTrue();
	void ConfigureFalse();
	void ConfigureDefined();
	void ConfigureUndefined();
	void ConfigureError();
	void CondTrue();
	void CondFalse();
	void CondDefined();
	void CondUndefined();
	void CondError();

	void ConfigureOnlyTrue();
	void ConfigureOnlyFalse();
	void ConfigureOnlyDefined();
	void ConfigureOnlyUndefined();
	void ConfigureOnlyError();
	void CondOnlyTrue();
	void CondOnlyFalse();
	void CondOnlyDefined();
	void CondOnlyUndefined();
	void CondOnlyError();

	void ConfigureMissingTrue();
	void ConfigureMissingFalse();
	void ConfigureMissingDefined();
	void ConfigureMissingUndefined();
	void ConfigureMissingError();
	void CondMissingTrue();
	void CondMissingFalse();
	void CondMissingDefined();
	void CondMissingUndefined();
	void CondMissingError();

	void ConfigureTrueWithoutTrueDefined();
	void ConfigureFalseWithoutFalseDefined();
	void CondTrueWithoutTrueDefined();
	void CondFalseWithoutFalseDefined();

	void ConfigureTrueConfUndefined();
	void ConfigureFalseConfUndefined();
	void ConfigureDefinedConfUndefined();
	void ConfigureUndefinedConfUndefined();
	void ConfigureErrorConfUndefined();
	void CondTrueConfUndefined();
	void CondFalseConfUndefined();
	void CondDefinedConfUndefined();
	void CondUndefinedConfUndefined();
	void CondErrorConfUndefined();

	void SetTrueFalseCase(long condition);
	void TestTrueFalseCase();
};

#endif
