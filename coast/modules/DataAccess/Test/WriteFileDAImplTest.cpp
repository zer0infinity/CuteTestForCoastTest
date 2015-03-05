/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "WriteFileDAImplTest.h"
#include "TestSuite.h"
#include "Context.h"
#include "AnyIterators.h"
#include "StringStream.h"
#if defined(WIN32)
#include <io.h>
#endif

//---- WriteFileDAImplTest ----------------------------------------------------------------
WriteFileDAImplTest::WriteFileDAImplTest(TString tname)
	: ConfiguredActionTest(tname)
{
	StartTrace(WriteFileDAImplTest.WriteFileDAImplTest);
}

TString WriteFileDAImplTest::getConfigFileName()
{
	return "WriteFileDAImplTestConfig";
}

WriteFileDAImplTest::~WriteFileDAImplTest()
{
	StartTrace(WriteFileDAImplTest.Dtor);
}

bool WriteFileDAImplTest::CompareResult(TString strResult)
{
	StartTrace(WriteFileDAImplTest.CompareResult);
	bool bRet = false;
	std::iostream *fp = coast::system::OpenStream("WriteTestFile", "txt");
	if (t_assert(fp != 0)) {
		OStringStream os;
		os << fp->rdbuf() << std::flush;
		bRet = assertEqual(strResult, os.str());
	}
	return bRet;
}

void WriteFileDAImplTest::WriteFileTest()
{
	StartTrace(WriteFileDAImplTest.WriteFileTest);

	String testCaseName(name());
	coast::system::io::unlink("./WriteTestFile.txt");
	Context ctx;
	ROAnything roaConfig;
	AnyExtensions::Iterator<ROAnything> aEntryIterator(GetTestCaseConfig());
	while ( aEntryIterator.Next(roaConfig) ) {
		DoTestWithContext(roaConfig, testCaseName, ctx);
		t_assert(CompareResult(roaConfig["Result"].AsCharPtr()));
	}
}

// builds up a suite of tests, add a line for each testmethod
Test *WriteFileDAImplTest::suite ()
{
	StartTrace(WriteFileDAImplTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, WriteFileDAImplTest, WriteFileTest);
	return testSuite;
}
