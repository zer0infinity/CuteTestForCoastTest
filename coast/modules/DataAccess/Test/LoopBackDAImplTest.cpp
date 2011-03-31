/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "LoopBackDAImplTest.h"
#include "DataAccessImpl.h"
#include "TestSuite.h"
#include "SystemFile.h"
#include "Context.h"

//---- LoopBackDAImplTest ----------------------------------------------------------------
Test *LoopBackDAImplTest::suite ()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, LoopBackDAImplTest, ExecTest);
	return testSuite;
}

LoopBackDAImplTest::LoopBackDAImplTest(TString tname)
	: TestCaseType(tname)
{
}

LoopBackDAImplTest::~LoopBackDAImplTest()
{
}

void LoopBackDAImplTest::setUp ()
{
	std::iostream *Ios = Coast::System::OpenStream("LoopBackDAImplTest", "any");
	if ( Ios ) {
		fStdContextAny.Import((*Ios));
		delete Ios;
	}
}

void LoopBackDAImplTest::ExecTest()
{
	StartTrace(LoopBackDAImplTest.ExecTest);

	Anything dummy;
	Context ctx(fStdContextAny, dummy, 0, 0, 0, 0);

	TraceAny(fStdContextAny, "Context Input: ");

	Anything tmpStore(ctx.GetTmpStore());

	ParameterMapper imapper("stdmapper");
	imapper.Initialize("ParameterMapper");
	ResultMapper omapper("stdmapper");
	omapper.Initialize("ResultMapper");
	LoopBackDAImpl lbdai("LoopBackDATest");
	lbdai.Initialize("DataAccessImpl");
	t_assert(lbdai.Exec(ctx, &imapper, &omapper));

	TraceAny(tmpStore, "TempStore:");
	assertEqual("Hello", tmpStore["Mapper"]["mReply"].AsCharPtr());
	assertEqual("Loopback try", tmpStore["Mapper"]["hReply"].AsCharPtr());
	assertEqual("coast bsshht", tmpStore["Mapper"]["fReply"].AsCharPtr());
	assertEqual("XXX", tmpStore["Mapper"]["cReply"].AsCharPtr());
}

