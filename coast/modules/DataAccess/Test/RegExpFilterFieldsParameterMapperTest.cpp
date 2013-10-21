/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "RegExpFilterFieldsParameterMapperTest.h"
#include "RegExpFilterFieldsParameterMapper.h"
#include "TestSuite.h"
#include "HierarchyInstallerWithConfig.h"
#include "Context.h"
#include "AnyUtils.h"

namespace {
    const char *_Value = "Value";
    const char *_Stream = "Stream";
	bool setupMappers(ROAnything roaMapperConfigs) {
		StartTrace(RegExpFilterFieldsParameterMapperTest.setupMappers);
		Anything mappersToInitialize;
		ROAnything mapperConfig;
		AnyExtensions::Iterator<ROAnything> aMapperConfigIterator(roaMapperConfigs);
		String mapperName;
		while (aMapperConfigIterator.Next(mapperConfig)) {
			if (aMapperConfigIterator.SlotName(mapperName)) {
				mappersToInitialize[mapperName] = Anything();
			}
		}
		HierarchyInstallerWithConfig ip(ParameterMapper::gpcCategory, roaMapperConfigs);
		return RegisterableObject::Install(mappersToInitialize, ParameterMapper::gpcCategory, &ip);
	}
	void unregisterMappers() {
		StartTrace(RegExpFilterFieldsParameterMapperTest.unregisterMappers);
		AliasTerminator at(ParameterMapper::gpcCategory);
		RegisterableObject::Terminate(ParameterMapper::gpcCategory, &at);
	}
	void setupContext(ROAnything caseConfig, Context &ctx) {
		coast::testframework::PutInStore(caseConfig["SessionStore"], ctx.GetSessionStore());
		coast::testframework::PutInStore(caseConfig["RoleStore"], ctx.GetRoleStoreGlobal());
		coast::testframework::PutInStore(caseConfig["TmpStore"], ctx.GetTmpStore());
		coast::testframework::PutInStore(caseConfig["Query"], ctx.GetQuery());
		coast::testframework::PutInStore(caseConfig["Env"], ctx.GetEnvStore());
	}
}

void RegExpFilterFieldsParameterMapperTest::ConfiguredTests() {
	StartTrace(RegExpFilterFieldsParameterMapperTest.ConfiguredTests);
	ROAnything caseConfig;
	AnyExtensions::Iterator<ROAnything, ROAnything, TString> aEntryIterator(GetTestCaseConfig());
	while (aEntryIterator.Next(caseConfig)) {
		TString caseName;
		if (!aEntryIterator.SlotName(caseName)) {
			t_assertm(false, "can not execute with unnamed case name, only named anything slots allowed");
			continue;
		}
		String getKeyName = caseConfig["Getkey"].AsString("HTTPHeader");
		if (t_assertm(setupMappers(caseConfig["MapperConfig"]), "ParameterMapper setup must succeed to execute tests")) {
			String mapperName = caseConfig["MapperConfig"].SlotName(0L);
			ParameterMapper *rm = ParameterMapper::FindParameterMapper(mapperName);
			if (t_assertm(rm != 0, TString("could not find mapper [") << mapperName.cstr() << "]")) {
				Context ctx;
				setupContext(caseConfig, ctx);
				ROAnything roaValue;
				if (caseConfig.LookupPath(roaValue, _Value)) {
					Anything value;
					t_assertm(rm->Get(getKeyName, value, ctx), caseName);
					assertAnyCompareEqual(roaValue, value, caseName, '.', ':');
				} else if (caseConfig.LookupPath(roaValue, _Stream)) {
					String strValue;
					OStringStream stream(strValue);
					t_assertm(rm->Get(getKeyName, stream, ctx), caseName);
					assertEquals(roaValue.AsString().cstr(), strValue.cstr());
				} else {
					t_assertm(false, TString("neither ").Append(_Value).Append(" nor ").Append(_Stream).Append(" is defined in configuration for ").Append(caseName));
					continue;
				}
			}
			unregisterMappers();
		}
	}
}

// builds up a suite of tests, add a line for each testmethod
Test *RegExpFilterFieldsParameterMapperTest::suite() {
	StartTrace(RegExpFilterFieldsParameterMapperTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, RegExpFilterFieldsParameterMapperTest, ConfiguredTests);
	return testSuite;
}
