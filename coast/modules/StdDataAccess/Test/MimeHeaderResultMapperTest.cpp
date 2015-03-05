/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "MimeHeaderResultMapperTest.h"
#include "MimeHeaderResultMapper.h"
#include "TestSuite.h"
#include "Context.h"

void MimeHeaderResultMapperTest::ConfiguredTests()
{
	StartTrace(MimeHeaderResultMapperTest.ConfiguredTests);
	ROAnything caseConfig;
	AnyExtensions::Iterator<ROAnything> aEntryIterator(GetTestCaseConfig());
	while (aEntryIterator.Next(caseConfig)) {
		String mapperName;
		if ( !aEntryIterator.SlotName(mapperName) ) {
			t_assertm(false, "can not execute with unnamed case name, only named anything slots allowed");
			continue;
		}
		TString caseName(mapperName.cstr());
		String strIn = caseConfig["Input"].AsString();
		String putKeyName = caseConfig["Putkey"].AsString("HTTPHeader");
		IStringStream is(strIn);
		MimeHeaderResultMapper m(mapperName);
		m.SetConfig(ResultMapper::gpcCategory, mapperName, caseConfig["MapperConfig"]);
		m.Initialize(ResultMapper::gpcCategory);
		Context ctx;
		t_assertm(m.Put(putKeyName, is, ctx), caseName);
		String outputLocation = m.GetDestinationSlot(ctx);
		outputLocation.Append(m.getDelim()).Append(putKeyName);
		assertAnyEqualm(caseConfig["Expected"], ctx.Lookup(outputLocation), caseName);
		m.Finalize();
	}
}

// builds up a suite of tests, add a line for each testmethod
Test *MimeHeaderResultMapperTest::suite ()
{
	StartTrace(MimeHeaderResultMapperTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, MimeHeaderResultMapperTest, ConfiguredTests);
	return testSuite;
}
