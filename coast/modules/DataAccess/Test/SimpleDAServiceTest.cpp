/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "SimpleDAServiceTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "SimpleDataAccessService.h"

//--- standard modules used ----------------------------------------------------
#include "StringStream.h"
#include "ServiceDispatcher.h"
#include "RequestProcessor.h"
#include "Dbg.h"
#include "StringStreamSocket.h"
#include "AnyIterators.h"

//---- SimpleDAServiceTest ----------------------------------------------------------------
SimpleDAServiceTest::SimpleDAServiceTest(TString tname)
	: TestCaseType(tname)
{
	StartTrace(SimpleDAServiceTest.Ctor);
}

SimpleDAServiceTest::~SimpleDAServiceTest()
{
	StartTrace(SimpleDAServiceTest.Dtor);
}

//:use the ServiceDispatcher to obtain a simple service
void SimpleDAServiceTest::SimpleDispatch()
{
	StartTrace(SimpleDAServiceTest.SimpleDispatch);
	{
		// test with objects configuration
		Context ctx;
		ServiceDispatcher sd("TestSimpleDAService");
		sd.Initialize("ServiceDispatcher");
		ctx.Push("TestSimpleDAService", &sd);

		ServiceHandler *sh = sd.FindServiceHandler(ctx);
		t_assertm(sh != 0, "expected to find SimpleDataAccessService handler");
		assertEqualm("SimpleDataAccessService", sd.FindServiceName(ctx), "expected to find test TestSimpleDAService name");
		if (sh) {
			String servicename;
			sh->GetName(servicename);
			assertEqualm("SimpleDataAccessService", servicename, "expected to find test TestSimpleDAService name");
		}
		String strKey;
		ctx.Pop(strKey);
	}
}
//: try to use it with a very simple DataAccess backend
void SimpleDAServiceTest::SimpleServiceCall()
{
	StartTrace(SimpleDAServiceTest.SimpleServiceCall);
}
//: try to use it with a missing DataAccess backend
void SimpleDAServiceTest::FailedServiceCall()
{
	StartTrace(SimpleDAServiceTest.FailedServiceCall);
	RequestProcessor *httpProcessor = RequestProcessor::FindRequestProcessor("TestHTTPProcessor");
	Anything anyTmpStore, anyRequest, env;
	ROAnything caseConfig;
	AnyExtensions::Iterator<ROAnything, ROAnything, TString> aEntryIterator(GetTestCaseConfig());
	while (aEntryIterator.Next(caseConfig)) {
		TString caseName;
		if ( !aEntryIterator.SlotName(caseName) ) {
			caseName << "At index: " << aEntryIterator.Index();
		}
		AnyLookupInterfaceAdapter<ROAnything> lia(caseConfig);
		String strInOut = caseConfig["RequestLine"].AsString();
		StringStreamSocket ss(strInOut);
		Context ctx(&ss);
		ctx.Push("tempargs", &lia);
		Anything anyValue = "TestHTTPProcessor";
		Context::PushPopEntry<Anything> aRPEntry(ctx, "RPName", anyValue, "RequestProcessor");
		httpProcessor->ProcessRequest(ctx);
		assertCharPtrEqualm(caseConfig["Expected"].AsString(), strInOut, caseName);
	}
}

// builds up a suite of testcases, add a line for each testmethod
Test *SimpleDAServiceTest::suite ()
{
	StartTrace(SimpleDAServiceTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, SimpleDAServiceTest, SimpleDispatch);
	ADD_CASE(testSuite, SimpleDAServiceTest, SimpleServiceCall);
	ADD_CASE(testSuite, SimpleDAServiceTest, FailedServiceCall);
	return testSuite;
}

class TestHTTPProcessor: public RequestProcessor {
public:
	//!named object shared by all requests
	TestHTTPProcessor(const char *processorName) :
		RequestProcessor(processorName) {
		StartTrace(TestHTTPProcessor.ctor);
	}

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		StartTrace(TestHTTPProcessor.Clone);
		return new (a) TestHTTPProcessor(fName);
	}

protected:
	bool DoVerifyRequest(Context &ctx) {
		StartTrace(TestHTTPProcessor.DoVerifyRequest);
		return !ctx.GetRequest().AsString().IsEqual("VerifyFail");
	}
	bool DoReadInput(std::iostream &Ios, Context &ctx) {
		Anything args;
		bool status = args.Import(Ios);
		StatTraceAny(TestHTTPProcessor.DoReadInput, args, "request arguments " << (status ? "success" : "failure"), Storage::Current());
		ctx.PushRequest(args);
		return status;
	}
	bool DoProcessRequest(std::ostream &reply, Context &ctx) {
		StartTrace(TestHTTPProcessor.DoProcessRequest);
		ServiceDispatcher sd("TestSimpleDAService");
		ctx.Push("TestSimpleDAService", &sd);
		return sd.Dispatch2Service(reply, ctx);
	}

	//! render the protocol specific error msg
	void DoHandleVerifyError(std::ostream &reply, Context &ctx) {
		StartTrace(TestHTTPProcessor.DoHandleVerifyError);
		reply << "TestHTTPProcessor::DoHandleVerifyError" << std::flush;
	}
	void DoHandleReadInputError(std::ostream &reply, Context &ctx) {
		StartTrace(TestHTTPProcessor.DoHandleReadInputError);
		reply << "TestHTTPProcessor::DoHandleReadInputError" << std::flush;
	}

	void DoHandleProcessRequestError(std::ostream &reply, Context &ctx) {
		StartTrace(TestHTTPProcessor.DoHandleProcessRequestError);
		ctx.Lookup(ctx.Lookup("RequestProcessorErrorSlot", "SlotNotFound!"))[0L].PrintOn(reply, false);
		OStringStream ostr;
		ctx.DebugStores("blabla", ostr, true);
		Trace(ostr.str());
	}
};

//--- TestHTTPProcessor ----------------------------------------------------------
RegisterRequestProcessor(TestHTTPProcessor);
