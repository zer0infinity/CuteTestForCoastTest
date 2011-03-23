/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "RegExpFilterFieldsResultMapperTest.h"
#include "RegExpFilterFieldsResultMapper.h"
#include "TestSuite.h"

void RegExpFilterFieldsResultMapperTest::ConfiguredTests()
{
	StartTrace(RegExpFilterFieldsResultMapperTest.ConfiguredTests);
	ROAnything caseConfig;
	AnyExtensions::Iterator<ROAnything> aEntryIterator(GetTestCaseConfig());
	while (aEntryIterator.Next(caseConfig)) {
		String mapperName;
		if ( !aEntryIterator.SlotName(mapperName) ) {
			t_assertm(false, "can not execute with unnamed case name, only named anything slots allowed");
			continue;
		}
		TString caseName(mapperName.cstr());
		Anything value = caseConfig["Value"].DeepClone();
		String putKeyName = caseConfig["Putkey"].AsString("HTTPHeader");
		RegExpFilterFieldsResultMapper m(mapperName);
		m.SetConfig(ResultMapper::gpcCategory, mapperName, caseConfig["MapperConfig"]);
		m.Initialize(ResultMapper::gpcCategory);
		Context ctx;
		t_assertm(m.Put(putKeyName, value, ctx), caseName);
		String outputLocation = m.GetDestinationSlot(ctx);
		outputLocation.Append(m.getDelim()).Append(putKeyName);
		assertAnyEqualm(caseConfig["Expected"], ctx.Lookup(outputLocation), caseName);
		m.Finalize();
	}
}

// builds up a suite of tests, add a line for each testmethod
Test *RegExpFilterFieldsResultMapperTest::suite ()
{
	StartTrace(RegExpFilterFieldsResultMapperTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, RegExpFilterFieldsResultMapperTest, ConfiguredTests);
	return testSuite;
}
