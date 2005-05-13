/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "DataAccessImpl.h"

//--- interface include --------------------------------------------------------
#include "LoopBackDAImplTest.h"

//--- standard modules used ----------------------------------------------------
#include "System.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//---- LoopBackDAImplTest ----------------------------------------------------------------
Test *LoopBackDAImplTest::suite ()
// collect all test cases for the RegistryStream
{
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(LoopBackDAImplTest, ExecTest));

	return testSuite;

} // suite

LoopBackDAImplTest::LoopBackDAImplTest(TString tname) : TestCase(tname)
{

}

LoopBackDAImplTest::~LoopBackDAImplTest()
{

}

void LoopBackDAImplTest::setUp ()
// setup connector for this TestCase
{
	iostream *Ios = System::OpenStream("LoopBackDAImplTest", "any");
	if ( Ios ) {
		fStdContextAny.Import((*Ios));
		delete Ios;
	}

} // setUp

void LoopBackDAImplTest::ExecTest()
{
	StartTrace(LoopBackDAImplTest.ExecTest);

	Anything dummy;
	Context ctx(fStdContextAny, dummy, 0, 0, 0, 0);

	TraceAny(fStdContextAny, "Context Input: ");

	Anything tmpStore(ctx.GetTmpStore());

	ParameterMapper imapper("stdmapper");

	ResultMapper omapper("stdmapper");
	LoopBackDAImpl lbdai("LoopBackDATest");
	lbdai.CheckConfig("DataAccessImpl");
	t_assert(lbdai.Exec(ctx, &imapper, &omapper));

	TraceAny(tmpStore, "TempStore:");
	assertEqual("Hello", tmpStore["Mapper"]["mReply"].AsCharPtr());
	assertEqual("Loopback try", tmpStore["Mapper"]["hReply"].AsCharPtr());
	assertEqual("coast bsshht", tmpStore["Mapper"]["fReply"].AsCharPtr());
	assertEqual("XXX", tmpStore["Mapper"]["cReply"].AsCharPtr());
}

