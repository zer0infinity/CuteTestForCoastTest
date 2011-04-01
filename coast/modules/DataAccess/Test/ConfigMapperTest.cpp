/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "ConfigMapperTest.h"
#include "Mapper.h"
#include "TestSuite.h"
#include "AnyIterators.h"
#include "Context.h"

void ConfigMapperTest::ConfigTest() {
	StartTrace(ConfigMapperTest.ConfigTest);
	String leafName("Test");
	ROAnything caseConfig;
	AnyExtensions::Iterator<ROAnything, ROAnything, TString> aEntryIterator(GetTestCaseConfig());
	while (aEntryIterator.Next(caseConfig)) {
		TString caseName;
		aEntryIterator.SlotName(caseName);
		Trace("Running " << caseName << " Test");

		// prepare tmp store
		Context ctx;
		ctx.GetTmpStore() = caseConfig["TmpStore"].DeepClone();

		// call
		Anything result;
		ConfigMapper cm(caseName);
		cm.Initialize(ParameterMapper::gpcCategory);
		cm.SetConfig(cm.getInstalledCategory(), caseName, caseConfig["MapperConfig"]);
		cm.Get("", result, ctx);
		TraceAny(result, "resulting config: ");

		// compare result and expected
		TraceAny(ctx.GetTmpStore(), "tmp store after:");
		assertAnyEqual(caseConfig["Expected"], result);
	}
}

// builds up a suite of tests, add a line for each testmethod
Test *ConfigMapperTest::suite() {
	StartTrace(ConfigMapperTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, ConfigMapperTest, ConfigTest);
	return testSuite;
}
