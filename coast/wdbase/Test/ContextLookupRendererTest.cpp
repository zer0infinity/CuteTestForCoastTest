/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "ContextLookupRendererTest.h"
#include "TestSuite.h"
#include "LookupRenderers.h"

ContextLookupRendererTest::ContextLookupRendererTest (TString tname)
	: TestCaseType(tname)
{
}

void ContextLookupRendererTest::ContextCharPtr()
// test the date renderer with simple formatting strings
{
	ContextLookupRenderer contextLookupRenderer("");
	Context fContext;
	Anything tmpStore(fContext.GetTmpStore());
	String testString = "EinString";
	String testKey = "EinSchl�ssel";

	tmpStore[testKey] = testString;

	fConfig["ContextLookupName"] = testKey;
	ROAnything roConfig(fConfig);

	contextLookupRenderer.RenderAll( fReply, fContext, roConfig);

	assertEqual(testString, fReply.str());
}

void ContextLookupRendererTest::ContextLong()
// test the date renderer with simple formatting strings
{
	ContextLookupRenderer contextLookupRenderer("");
	Context fContext;
	Anything tmpStore(fContext.GetTmpStore());
	long 	testLong = 123456;
	String 	testKey = "EinSchl�ssel";

	tmpStore[testKey] = testLong;

	fConfig["ContextLookupName"] = testKey;
	ROAnything roConfig(fConfig);

	contextLookupRenderer.RenderAll( fReply, fContext, roConfig);

	assertEqual("123456", fReply.str());
}

void ContextLookupRendererTest::ContextDouble()
// test the date renderer with simple formatting strings
{
	ContextLookupRenderer contextLookupRenderer("");
	Context fContext;
	Anything tmpStore(fContext.GetTmpStore());
	double 	testDouble 	= 123.456;
	String 	testKey 	= "EinSchl�ssel";

	tmpStore[testKey] = testDouble;

	fConfig["ContextLookupName"] = testKey;
	ROAnything roConfig(fConfig);

	contextLookupRenderer.RenderAll( fReply, fContext, roConfig);

	assertEqual("123.456", fReply.str().SubString(0, 7));
}

void ContextLookupRendererTest::ContextNull()
// test the date renderer with simple formatting strings
{
	ContextLookupRenderer contextLookupRenderer("");
	Context fContext;
	Anything tmpStore(fContext.GetTmpStore());
	Anything NullAny;
	String 	testKey 	= "EinSchl�ssel";

	tmpStore[testKey] = NullAny;

	fConfig["ContextLookupName"] = testKey;
	ROAnything roConfig(fConfig);

	contextLookupRenderer.RenderAll( fReply, fContext, roConfig);

	assertEqual("", fReply.str());
}

void ContextLookupRendererTest::ContextCharPtrConfigArray()
// test the date renderer with simple formatting strings
{
	ContextLookupRenderer contextLookupRenderer("");
	Context fContext;
	Anything tmpStore(fContext.GetTmpStore());
	String testString = "EinString";
	String testKey = "EinSchl�ssel";

	tmpStore[testKey] = testString;

	fConfig.Append( testKey );
	ROAnything roConfig(fConfig);

	contextLookupRenderer.RenderAll( fReply, fContext, roConfig);

	assertEqual("EinString", fReply.str());
}

void ContextLookupRendererTest::ContextCharPtrDef()
// test the date renderer with simple formatting strings
{
	Context fContext;

	ContextLookupRenderer contextLookupRenderer("");
	String testString = "EinString";

	fConfig["ContextLookupName"] = "";
	fConfig["Default"] = testString;
	ROAnything roConfig(fConfig);

	contextLookupRenderer.RenderAll( fReply, fContext, roConfig);

	assertEqual("EinString", fReply.str());
}

void ContextLookupRendererTest::ContextLongDef()
// test the date renderer with simple formatting strings
{
	Context fContext;

	ContextLookupRenderer contextLookupRenderer("");
	long 	testLong = 123456;

	fConfig["ContextLookupName"] = "";
	fConfig["Default"] = testLong;
	ROAnything roConfig(fConfig);

	contextLookupRenderer.RenderAll( fReply, fContext, roConfig);

	assertEqual("123456", fReply.str());
}

void ContextLookupRendererTest::ContextDoubleDef()
// test the date renderer with simple formatting strings
{
	Context fContext;

	ContextLookupRenderer contextLookupRenderer("");
	double 	testDouble 	= 123.456;

	fConfig["ContextLookupName"] = "";
	fConfig["Default"] = testDouble;
	ROAnything roConfig(fConfig);

	contextLookupRenderer.RenderAll( fReply, fContext, roConfig);

	assertEqual("123.456", fReply.str().SubString(0, 7));
}

void ContextLookupRendererTest::ContextNullDef()
// test the date renderer with simple formatting strings
{
	Context fContext;

	ContextLookupRenderer contextLookupRenderer("");
	Anything NullAny;

	fConfig["ContextLookupName"] = "";
	fConfig["Default"] = NullAny;
	ROAnything roConfig(fConfig);

	contextLookupRenderer.RenderAll( fReply, fContext, roConfig);

	assertEqual("", fReply.str());
}

void ContextLookupRendererTest::NestedLookup()
// test the lookup in nested things
{
	ContextLookupRenderer contextLookupRenderer("");
	Context fContext;
	Anything tmpStore(fContext.GetTmpStore());
	String testString1 = "AString";
	String testString2 = "Two Strings";
	String testKey = "AKey";
	String arrayKey = "AnArray";
	String combinedKey(arrayKey);
	combinedKey << "." << testKey;
	String nullDelim( (void *)"\0", 1);
	const char *atDelim = "@";
	// it is a bit tricky to store 0 in an Anything programmatically
	String expectedResult(testString1);

	// setup a nested and a flat access
	tmpStore[arrayKey][testKey] = testString1;
	tmpStore[combinedKey] = testString2;

	// test access to nested testString1 using standard delimiter
	fConfig["ContextLookupName"] = combinedKey;
	ROAnything roConfig(fConfig);

	contextLookupRenderer.RenderAll( fReply, fContext, roConfig);

	assertEqual(expectedResult, fReply.str());
	// by default I use '.' as delimiter.

	// test access to nested testString1 using @ as delimiter
	String combinedKey2(arrayKey);
	combinedKey2 << atDelim << testKey;
	fConfig["ContextLookupName"] = combinedKey2;
	fConfig["Delim"] = atDelim;		// use a special delimiter
	roConfig = fConfig;

	contextLookupRenderer.RenderAll( fReply, fContext, roConfig);
	expectedResult << testString1;

	assertEqual(expectedResult, fReply.str());
	// by default I use '.' as delimiter.

	// test access to non nested testString2 without delimiter
	fConfig["ContextLookupName"] = combinedKey;
	fConfig["Delim"] = nullDelim;		// do not use delimiters

	contextLookupRenderer.RenderAll( fReply, fContext, roConfig);
	expectedResult << testString2;

	assertEqual(expectedResult, fReply.str());

	// test access to non nested testString2 with nonpresent delimiter
	fConfig["Delim"] = "/";		// this delimiter is not in combinedKey

	contextLookupRenderer.RenderAll( fReply, fContext, roConfig);
	expectedResult << testString2;

	assertEqual(expectedResult, fReply.str());
} // NestedLookup

void ContextLookupRendererTest::NestedLookupWithoutSlotnames()
// test the lookup in nested things without using slotnames (positional params)
{
	ContextLookupRenderer contextLookupRenderer("");
	Context fContext;
	Anything tmpStore(fContext.GetTmpStore());
	String testString1 = "AnotherString";
	String testString2 = "Two other Strings";
	String testKey = "AKey";
	String arrayKey = "AnArray";
	String combinedKey(arrayKey);
	combinedKey << "." << testKey;
	String expectedResult(testString1);

	// setup a nested and a flat access
	tmpStore[arrayKey][testKey] = testString1;
	tmpStore[combinedKey] = testString2;

	// test access to nested testString1 using standard delimiter
	fConfig[0L] = combinedKey;
	ROAnything roConfig(fConfig);

	contextLookupRenderer.RenderAll( fReply, fContext, roConfig);

	assertEqual(expectedResult, fReply.str());
	// by default I use '.' as delimiter.

	// test access to nested testString1 using @ as delimiter
	const char *atDelim = "@";
	String combinedKey2(arrayKey);
	combinedKey2 << atDelim << testKey;
	fConfig[0L] = combinedKey2;
	fConfig[1L] = "Default";
	fConfig[2L] = atDelim;		// use a special delimiter
	roConfig = fConfig;

	contextLookupRenderer.RenderAll( fReply, fContext, roConfig);
	expectedResult << testString1;

	assertEqual(expectedResult, fReply.str());
	// by default I use '.' as delimiter.

	// test access to non nested testString2 without delimiter
	String nullDelim( (void *)"\0", 1);
	// it is a bit tricky to store 0 in an Anything programmatically
	fConfig[0L] = combinedKey;
	fConfig[2L] = nullDelim;		// do not use delimiters

	contextLookupRenderer.RenderAll( fReply, fContext, roConfig);
	expectedResult << testString2;

	assertEqual(expectedResult, fReply.str());

	// test access to non nested testString2 with nonpresent delimiter
	fConfig[2L] = "/";		// this delimiter is not in combinedKey

	contextLookupRenderer.RenderAll( fReply, fContext, roConfig);
	expectedResult << testString2;

	assertEqual(expectedResult, fReply.str());
} // NestedLookupWithoutSlotnames

Test *ContextLookupRendererTest::suite ()
{
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, ContextLookupRendererTest, ContextCharPtr);
	ADD_CASE(testSuite, ContextLookupRendererTest, ContextLong);
	ADD_CASE(testSuite, ContextLookupRendererTest, ContextDouble);
	ADD_CASE(testSuite, ContextLookupRendererTest, ContextNull);
	ADD_CASE(testSuite, ContextLookupRendererTest, ContextCharPtrConfigArray);
	ADD_CASE(testSuite, ContextLookupRendererTest, ContextCharPtrDef);
	ADD_CASE(testSuite, ContextLookupRendererTest, ContextLongDef);
	ADD_CASE(testSuite, ContextLookupRendererTest, ContextDoubleDef);
	ADD_CASE(testSuite, ContextLookupRendererTest, ContextNullDef);
	ADD_CASE(testSuite, ContextLookupRendererTest, NestedLookup);
	ADD_CASE(testSuite, ContextLookupRendererTest, NestedLookupWithoutSlotnames);

	return testSuite;
}
