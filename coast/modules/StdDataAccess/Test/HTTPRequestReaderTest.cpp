/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "HTTPRequestReaderTest.h"
#include "HTTPRequestReader.h"
#include "TestSuite.h"
#include "FoundationTestTypes.h"
#include "Context.h"
#include "MIMEHeader.h"
#include "StringStreamSocket.h"
#include "AnyIterators.h"
#include "AnyUtils.h"

void HTTPRequestReaderTest::ReadMinimalInputTest() {
	StartTrace(HTTPRequestReaderTest.ReadMinimalInputTest);

	ROAnything cConfig;
	AnyExtensions::Iterator<ROAnything, ROAnything, TString> aEntryIterator(GetTestCaseConfig());
	while (aEntryIterator.Next(cConfig)) {
		Anything anyParams = cConfig.DeepClone();
		AnyLookupInterfaceAdapter<Anything> lia(anyParams);
		TString caseName;
		aEntryIterator.SlotName(caseName);
		if (not caseName.Length())
			caseName.Append("failed at idx:").Append(aEntryIterator.Index());

		MIMEHeader header;
		HTTPRequestReader reader(header);

		String uri(cConfig["RequestLine"].AsString());
		Trace("Request to process [" << uri << "]");
		StringStreamSocket ss(uri);
		Context ctx(&ss);
		ctx.Push("tempargs", &lia);
		bool ret = reader.ReadRequest(ctx, *(ss.GetStream()));
		assertComparem(cConfig["Expected"]["Return"].AsBool(false), equal_to, ret, caseName);
		assertAnyEqualm(cConfig["Expected"]["Request"], reader.GetRequest(), caseName);
		OStringStream oss;
		ctx.DebugStores("bla", oss, true);
		Trace(oss.str());
		if (cConfig["Expected"].IsDefined("TmpStore")) {
			assertAnyCompareEqual(cConfig["Expected"]["TmpStore"], ctx.GetTmpStore(), caseName, '.', ':');
		}
	}
}

Test *HTTPRequestReaderTest::suite() {
	StartTrace(HTTPRequestReaderTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, HTTPRequestReaderTest, ReadMinimalInputTest);
	return testSuite;
}
