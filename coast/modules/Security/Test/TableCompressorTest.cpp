/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include <stdlib.h>
#include "Anything.h"
#include "Threads.h"
#include "TestSuite.h"
#include "TableCompressor.h"
#include "TableCompressorTest.h"

//---- TableCompressorTest ----------------------------------------------------------------
Test *TableCompressorTest::suite ()
{
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, TableCompressorTest, CalcKeyTest);
	ADD_CASE(testSuite, TableCompressorTest, CalcAtTest);
	ADD_CASE(testSuite, TableCompressorTest, MakeTableTest);
	ADD_CASE(testSuite, TableCompressorTest, MakeRevTableTest);
	ADD_CASE(testSuite, TableCompressorTest, InitTest);
	ADD_CASE(testSuite, TableCompressorTest, InitWithExpansionTest);
	ADD_CASE(testSuite, TableCompressorTest, CompressExpandTest);
	ADD_CASE(testSuite, TableCompressorTest, CompressExpandWithEmptyConfigTest);
	ADD_CASE(testSuite, TableCompressorTest, InstallConfigTest);
	ADD_CASE(testSuite, TableCompressorTest, ExpandConfigTest);

	return testSuite;
}

TableCompressorTest::TableCompressorTest(TString tname) : TestCaseType(tname)
{
}

TableCompressorTest::~TableCompressorTest()
{
}

void TableCompressorTest::CalcAtTest()
{
	TableCompressor tc("test");
	t_assertm('a' == tc.KeyAt(-1), "expected 'a'");
	t_assertm('a' == tc.KeyAt(0), "expected 'a'");
	t_assertm('z' == tc.KeyAt(25), "expected 'z'");
	t_assertm('A' == tc.KeyAt(26), "expected 'A'");
	t_assertm('Z' == tc.KeyAt(51), "expected 'Z'");
	t_assertm('Z' == tc.KeyAt(52), "expected 'Z'");
	t_assertm('o' == tc.KeyAt(14), "expected 'o'");
	t_assertm('E' == tc.KeyAt(30), "expected 'G'");
}

void TableCompressorTest::CalcKeyTest()
{
	TableCompressor tc("test");
	String result;

	result = tc.CalcKey(-1);
	assertEqual("a", result);

	result = tc.CalcKey(0);
	assertEqual("a", result);

	result = tc.CalcKey(25);
	assertEqual("z", result);

	result = tc.CalcKey(26);
	assertEqual("A", result);

	result = tc.CalcKey(51);
	assertEqual("Z", result);

	result = tc.CalcKey(52);
	assertEqual("aa", result);

	result = tc.CalcKey(103);
	assertEqual("aZ", result);

	result = tc.CalcKey(104);
	assertEqual("ba", result);

	result = tc.CalcKey(123);
	assertEqual("bt", result);
}

void TableCompressorTest::MakeTestTable(TableCompressor *tc)
{
	Anything state;
	state.Append("foo");
	state.Append("slosh");
	state.Append("bah");

	tc->MakeTable(state, "test", Anything());
}

void TableCompressorTest::MakeTableTest()
{
	Anything result;
	result["test"]["foo"] = "a";
	result["test"]["slosh"] = "b";
	result["test"]["bah"] = "c";

	TableCompressor tc("test");
	MakeTestTable(&tc);

	assertAnyEqual(result, tc.fMap);

}

void TableCompressorTest::MakeRevTableTest()
{
	Anything result;
	result["test"]["foo"] = "a";
	result["test"]["slosh"] = "b";
	result["test"]["bah"] = "c";
	result["revtest"]["a"] = "foo";
	result["revtest"]["b"] = "slosh";
	result["revtest"]["c"] = "bah";

	TableCompressor tc("test");
	MakeTestTable(&tc);
	tc.MakeReverseTable(tc.fMap["test"], "test", "revtest");

	assertAnyEqual(result, tc.fMap);
}

void TableCompressorTest::InitTest()
{
	Anything key2UriMap;
	Anything val2UriMap;
	Anything valMapTags;

	key2UriMap.Append("Page");
	key2UriMap.Append("Action");
	key2UriMap.Append("SessionId");

	val2UriMap.Append("SomeVeryLongPage1");
	val2UriMap.Append("SomeUltraLongAction");
	val2UriMap.Append("AVerySophisticatedRole");

	valMapTags.Append("Page");
	valMapTags.Append("Role");
	valMapTags.Append("Action");

	Anything config;

	config["SecurityModule"]["TableCompressor"]["Key2UriMap"] = key2UriMap;
	config["SecurityModule"]["TableCompressor"]["Val2UriMap"] = val2UriMap;
	config["SecurityModule"]["TableCompressor"]["ValMapTags"] = valMapTags;

	Anything result;
	result["Key2UriMap"]["Page"] = "a";
	result["Key2UriMap"]["Action"] = "b";
	result["Key2UriMap"]["SessionId"] = "c";
	result["Uri2KeyMap"]["a"] = "Page";
	result["Uri2KeyMap"]["b"] = "Action";
	result["Uri2KeyMap"]["c"] = "SessionId";

	result["Val2UriMap"]["SomeVeryLongPage1"] = "a";
	result["Val2UriMap"]["SomeUltraLongAction"] = "b";
	result["Val2UriMap"]["AVerySophisticatedRole"] = "c";
	result["Uri2ValMap"]["a"] = "SomeVeryLongPage1";
	result["Uri2ValMap"]["b"] = "SomeUltraLongAction";
	result["Uri2ValMap"]["c"] = "AVerySophisticatedRole";

	result["ValMapTags"] = valMapTags;

	TableCompressor tc("test");
	tc.Init(config);

	assertAnyEqual(result, tc.fMap);
}

void TableCompressorTest::CompressExpandTest()
{
	Anything key2UriMap;
	Anything val2UriMap;
	Anything valMapTags;

	key2UriMap.Append("Page");
	key2UriMap.Append("Role");
	key2UriMap.Append("Action");
	key2UriMap.Append("SessionId");

	val2UriMap.Append("SomeVeryLongPage1");
	val2UriMap.Append("SomeUltraLongAction");
	val2UriMap.Append("AVerySophisticatedRole");

	valMapTags.Append("Page");
	valMapTags.Append("Role");
	valMapTags.Append("Action");

	Anything config;

	config["SecurityModule"]["TableCompressor"]["Key2UriMap"] = key2UriMap;
	config["SecurityModule"]["TableCompressor"]["Val2UriMap"] = val2UriMap;
	config["SecurityModule"]["TableCompressor"]["ValMapTags"] = valMapTags;

	TableCompressor tc("test");
	tc.Init(config);

	Anything state;
	state["Role"] = "AVerySophisticatedRole";
	state["Page"] = "SomeVeryLongPage1";
	state["Action"] = "SomeUltraLongAction";
	state["OtherState"] = "AValue";

	String result;
	result = "{/b \"c\"/a \"a\"/c \"b\"/OtherState \"AValue\"}";

	String compressed;
	Anything expanded;
	tc.DoCompress(compressed, state);
	assertEqual(result, compressed);
	tc.DoExpand(expanded, result);
	assertAnyEqual(state, expanded);
}

void TableCompressorTest::CompressExpandWithEmptyConfigTest()
{
	Anything config;
	TableCompressor tc("test");
	tc.Init(config);

	Anything state;
	state["Role"] = "AVerySophisticatedRole";
	state["Page"] = "SomeVeryLongPage1";
	state["Action"] = "SomeUltraLongAction";
	state["OtherState"] = "AValue";

	String result;
	result = "{/Role \"AVerySophisticatedRole\"/Page \"SomeVeryLongPage1\"/Action \"SomeUltraLongAction\"/OtherState \"AValue\"}";

	String compressed;
	Anything expanded;
	tc.DoCompress(compressed, state);
	assertEqual(result, compressed);
	tc.DoExpand(expanded, result);
	assertAnyEqual(state, expanded);
}

void TableCompressorTest::InstallConfigTest()
{
	Anything pages;
	pages["Page"].Append("SomeVeryLongPage1");
	pages["Page"]["ASubPage"].Append("SomeVeryLongPage2");

	Anything state;
	Anything result;
	result.Append("Page");
	result.Append("SomeVeryLongPage1");
	result.Append("ASubPage");
	result.Append("SomeVeryLongPage2");

	TableCompressor tc("test");
	tc.InstallConfig(0L, state, pages);

	assertAnyEqual(result, state);
}

void TableCompressorTest::ExpandConfigTest()
{
	Anything pages;
	pages["Page"].Append("SomeVeryLongPage1");
	pages["Page"]["ASubPage"].Append("SomeVeryLongPage2");

	Anything config;
	config["Pages"] = pages;

	Anything state;
	state[0L]["Expand"] = "Pages";
	Anything result;
	result.Append("Page");
	result.Append("SomeVeryLongPage1");
	result.Append("ASubPage");
	result.Append("SomeVeryLongPage2");

	TableCompressor tc("test");
	tc.ExpandConfig(0L, state, config);

	assertAnyEqual(result, state);
}

void TableCompressorTest::InitWithExpansionTest()
{
	Anything key2UriMap;
	Anything val2UriMap;
	Anything valMapTags;
	Anything expander;

	key2UriMap.Append("Page");
	key2UriMap.Append("Action");
	key2UriMap.Append("SessionId");

	expander["Expand"] = "Pages";
	val2UriMap.Append(expander.DeepClone());

	expander["Expand"] = "Roles";
	val2UriMap.Append(expander.DeepClone());

	expander["Expand"] = "Actions";
	val2UriMap.Append(expander.DeepClone());

	valMapTags.Append("Page");
	valMapTags.Append("Role");
	valMapTags.Append("Action");

	Anything pages;
	pages["Page"].Append("SomeVeryLongPage1");
	pages["Page"]["ASubPage"].Append("SomeVeryLongPage2");

	Anything roles;
	roles["Role"].Append("AVerySophisticatedRole");
	roles["Role"]["ASubRole"].Append("SomeVeryLongRole2");

	Anything actions;
	actions["Action"].Append("SomeUltraLongAction");
	actions["Action"]["ASubAction"].Append("SomeUltraLongAction2");

	Anything config;
	config["SecurityModule"]["TableCompressor"]["Key2UriMap"] = key2UriMap;
	config["SecurityModule"]["TableCompressor"]["Val2UriMap"] = val2UriMap;
	config["SecurityModule"]["TableCompressor"]["ValMapTags"] = valMapTags;
	config["Pages"] = pages;
	config["Roles"] = roles;
	config["Actions"] = actions;

	Anything result;
	result["Key2UriMap"]["Page"] = "a";
	result["Key2UriMap"]["Action"] = "b";
	result["Key2UriMap"]["SessionId"] = "c";
	result["Uri2KeyMap"]["a"] = "Page";
	result["Uri2KeyMap"]["b"] = "Action";
	result["Uri2KeyMap"]["c"] = "SessionId";

	result["Val2UriMap"]["Page"] = "a";
	result["Val2UriMap"]["SomeVeryLongPage1"] = "b";
	result["Val2UriMap"]["ASubPage"] = "c";
	result["Uri2ValMap"]["a"] = "Page";
	result["Uri2ValMap"]["b"] = "SomeVeryLongPage1";
	result["Uri2ValMap"]["c"] = "ASubPage";

	result["ValMapTags"] = valMapTags;

	TableCompressor tc("test");
	tc.Init(config);

	assertAnyEqual(result, tc.fMap);
}

