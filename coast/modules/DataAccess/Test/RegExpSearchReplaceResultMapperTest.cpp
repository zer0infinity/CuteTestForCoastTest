/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "RegExpSearchReplaceResultMapperTest.h"
#include "RegExpSearchReplaceResultMapper.h"
#include "TestSuite.h"
#include "AnyUtils.h"
#include "CheckStores.h"

void RegExpSearchReplaceResultMapperTest::ConfiguredTests()
{
	StartTrace(RegExpSearchReplaceResultMapperTest.ConfiguredTests);
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
		RegExpSearchReplaceResultMapper m(mapperName);
		m.SetConfig(ResultMapper::gpcCategory, mapperName, caseConfig["MapperConfig"]);
		m.Initialize(ResultMapper::gpcCategory);
		Context ctx;
		t_assertm(m.Put(putKeyName, value, ctx), caseName);
		String outputLocation = m.GetDestinationSlot(ctx);
		outputLocation.Append(m.getDelim()).Append(putKeyName);
		assertAnyCompareEqual(caseConfig["Expected"], ctx.GetTmpStore(), caseName, m.getDelim(), m.getIndexDelim());

		Anything anyFailureStrings;
		Coast::TestFramework::CheckStores(anyFailureStrings, caseConfig["Result"], ctx, caseName, Coast::TestFramework::exists);
		// non-existence tests
		Coast::TestFramework::CheckStores(anyFailureStrings, caseConfig["NotResult"], ctx, caseName, Coast::TestFramework::notExists);
		for ( long sz=anyFailureStrings.GetSize(),i=0; i<sz;++i ) {
			t_assertm(false, anyFailureStrings[i].AsString().cstr());
		}

		m.Finalize();
	}
}

// builds up a suite of tests, add a line for each testmethod
Test *RegExpSearchReplaceResultMapperTest::suite ()
{
	StartTrace(RegExpSearchReplaceResultMapperTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, RegExpSearchReplaceResultMapperTest, ConfiguredTests);
	return testSuite;
}
