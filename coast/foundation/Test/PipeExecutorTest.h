/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _PipeExecutorTest_H
#define _PipeExecutorTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"
#include "Anything.h"

//---- forward declaration -----------------------------------------------

//---- PipeExecutorTest ----------------------------------------------------------
//!
class PipeExecutorTest : public TestCase
{
	Anything fConfig;
	ROAnything fTestCaseConfig;
public:
	//--- constructors
	PipeExecutorTest(TString tstrName);
	virtual ~PipeExecutorTest();
	void EchoCatTest(); // call cat for echoing stdin stdout
	void CatWorkingDirTest(); // call cat for reading a file in separate dir
	void EchoCatErrTest(); // call cat for echoing stdin stdout
	void EchoEnvTest();
	void FailExecTest();
	void KillTest();
	void CatGugusErrTest();

	void ParseParamTest(); // check for mechanism working
	void PrepareEnvTest(); // check for mechanism working
	void PrepareParamTest(); // check for mechanism working
	void DummyKillTest(); // check for mechanism working
	void ShellInvocationTest();

	void setUp();
	void tearDown();

	static Test *suite ();
};

#endif
