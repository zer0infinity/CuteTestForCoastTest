/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- interface include --------------------------------------------------------
#include "WriteFileDAImplTest.h"

//--- standard modules used ----------------------------------------------------
#include "StringStream.h"
#include "System.h"
#include "Context.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------
#if defined(WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif

//---- WriteFileDAImplTest ----------------------------------------------------------------
WriteFileDAImplTest::WriteFileDAImplTest(TString tname)
	: ConfiguredActionTest(tname, "WriteFileDAImplTestConfig")
{
	StartTrace(WriteFileDAImplTest.Ctor);
}

WriteFileDAImplTest::~WriteFileDAImplTest()
{
	StartTrace(WriteFileDAImplTest.Dtor);
}

bool WriteFileDAImplTest::CompareResult()
{
	StartTrace(WriteFileDAImplTest.CompareResult);
	bool bRet = false;
	iostream *fp = System::OpenStream("WriteTestFile", "txt");
	if (t_assert(fp != 0)) {
		OStringStream os;
		os << fp->rdbuf();
		bRet = assertEqual(fTestCaseConfig["Result"].AsString(), os.str());
	}
	return bRet;
}

void WriteFileDAImplTest::WriteFileTest()
{
	StartTrace(WriteFileDAImplTest.WriteFileTest);

	String testCaseName(name());
	System::IO::unlink("./WriteTestFile.txt");
	Context ctx;
	DoTestWithContext(fTestCaseConfig, testCaseName, ctx);
	t_assert(CompareResult());

	// Write again should fail because of default flag 'noreplace'
	fTestCaseConfig["ExpectedResult"] = false;
	String strContent = fTestCaseConfig["TmpStore"]["FileContent"].AsString();
	fTestCaseConfig["TmpStore"]["FileContent"] = "guguseli";
	DoTestWithContext(fTestCaseConfig, testCaseName, ctx);
	t_assert(CompareResult());

	// Write again should fail because of default flag 'noreplace'
	fTestCaseConfig["TmpStore"]["Mode"] = "noreplace";
	DoTestWithContext(fTestCaseConfig, testCaseName, ctx);
	t_assert(CompareResult());

	// try to append something to the file, should succeed again
	fTestCaseConfig["ExpectedResult"] = true;
	fTestCaseConfig["TmpStore"]["FileContent"] = strContent;
	Anything anyModes;
	anyModes.Append("append");
	fTestCaseConfig["TmpStore"]["Mode"] = anyModes;
	String strResult = fTestCaseConfig["Result"].AsString();
	strResult << strResult;
	fTestCaseConfig["Result"] = strResult;
	DoTestWithContext(fTestCaseConfig, testCaseName, ctx);
	t_assert(CompareResult());

	// truncate the file with some new content
	strResult = "truncated content\ntest this!";
	fTestCaseConfig["TmpStore"]["FileContent"] = strResult;
	fTestCaseConfig["Result"] = strResult;
	anyModes = "truncate";
	fTestCaseConfig["TmpStore"]["Mode"] = anyModes;
	DoTestWithContext(fTestCaseConfig, testCaseName, ctx);
	t_assert(CompareResult());
}

// builds up a suite of ConfiguredTestCases, add a line for each testmethod
Test *WriteFileDAImplTest::suite ()
{
	StartTrace(WriteFileDAImplTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, WriteFileDAImplTest, WriteFileTest);

	return testSuite;

} // suite
