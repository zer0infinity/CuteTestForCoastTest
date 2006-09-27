/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ReadFileDAImplTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "ReadFileDAImpl.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

//---- ReadFileDAImplTest ----------------------------------------------------------------
ReadFileDAImplTest::ReadFileDAImplTest(TString tstrName) : TestCaseType(tstrName)
{
	StartTrace(ReadFileDAImplTest.Ctor);
}

ReadFileDAImplTest::~ReadFileDAImplTest()
{
	StartTrace(ReadFileDAImplTest.Dtor);
}

void ReadFileDAImplTest::GetFileNameTest()
{
	StartTrace(ReadFileDAImplTest.GetFileNameTest);

	ReadFileDAImpl hfl("test");
	ParameterMapper mp("test");
	mp.Initialize("ParameterMapper");

	Context ctx;
	String filename("defaultFileName");
	String ext("defaultExt");

	t_assertm(!hfl.GetFileName(filename, ext, ctx, &mp), "expected failure of filename generation");
	assertEqual("defaultFileName", filename);
	assertEqual("defaultExt", ext);

	Anything tmpStore(ctx.GetTmpStore());
	tmpStore["DocumentRoot"] = ".";
	t_assertm(!hfl.GetFileName(filename, ext, ctx, &mp), "expected failure of filename generation");
	assertEqual(".", filename);
	assertEqual("defaultExt", ext);

	tmpStore["Filename"] = "/config/TestData";
	t_assertm(hfl.GetFileName(filename, ext, ctx, &mp), "expected success of filename generation");
	assertEqual("./config/TestData", filename);
	assertEqual("defaultExt", ext);

	tmpStore["DocumentRoot"] = ".";
	tmpStore["Filename"] = "/config/TestData";
	tmpStore["Extension"] = "any";

	t_assertm(hfl.GetFileName(filename, ext, ctx, &mp), "expected success of filename generation");
	assertEqual("./config/TestData", filename);
	assertEqual("any", ext);
}

void ReadFileDAImplTest::GetFileStreamTest()
{
	StartTrace(ReadFileDAImplTest.GetFileStreamTest);
	ReadFileDAImpl hfl("test");
	ParameterMapper mp("test");
	mp.Initialize("ParameterMapper");
	Context ctx;
	Anything tmpStore(ctx.GetTmpStore());
	tmpStore["DocumentRoot"] = ".";
	tmpStore["Filename"] = "/config/TestData";
	tmpStore["Extension"] = "any";
	istream *pStream = hfl.GetFileStream(ctx, &mp);
	t_assertm(pStream != NULL, "expected valid file stream");
	delete pStream;

	tmpStore["Mode"] = "text";
	pStream = hfl.GetFileStream(ctx, &mp);
	t_assertm(pStream != NULL, "expected valid file stream");
	delete pStream;

	tmpStore.Remove("DocumentRoot");
	tmpStore["Filename"] = "TestData";
	pStream = hfl.GetFileStream(ctx, &mp);
	t_assertm(pStream != NULL, "expected valid file stream for relative filename test");
	delete pStream;
}

// builds up a suite of testcases, add a line for each testmethod
Test *ReadFileDAImplTest::suite ()
{
	StartTrace(ReadFileDAImplTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, ReadFileDAImplTest, GetFileNameTest);
	ADD_CASE(testSuite, ReadFileDAImplTest, GetFileStreamTest);

	return testSuite;
}
