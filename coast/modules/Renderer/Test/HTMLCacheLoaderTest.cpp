/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "HTMLCacheLoaderTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "HTMLTemplateCacheLoader.h"

//--- standard modules used ----------------------------------------------------
#include "TemplateParser.h"
#include "HTMLTemplateRenderer.h"

//---- HTMLCacheLoaderTest ----------------------------------------------------------------
HTMLCacheLoaderTest::HTMLCacheLoaderTest(TString tstrName)
	: TestCaseType(tstrName)
{
	StartTrace(HTMLCacheLoaderTest.Ctor);
}

HTMLCacheLoaderTest::~HTMLCacheLoaderTest()
{
	StartTrace(HTMLCacheLoaderTest.Dtor);
}

void HTMLCacheLoaderTest::LoadEmptyCacheTest()
{
	StartTrace(HTMLCacheLoaderTest.LoadEmptyCacheTest);
	TemplateParser tp;
	HTMLTemplateCacheLoader htcl(&tp);
	Anything result = htcl.Load("nonexistentfile");
	t_assert(result.IsNull());
	t_assert(Storage::Global() == result.GetAllocator());
	//-- now the same with an existing but empty file
	const char *const TESTFILE = "HTMLCacheLoaderTestEmpty";
	std::ostream *os = Coast::System::OpenOStream(TESTFILE, "html");
	if (t_assertm((os != NULL), "assumed to be able to write file")) {
		delete os;
		result = htcl.Load(TESTFILE);
		t_assert(result.IsNull());
		t_assert(Storage::Global() == result.GetAllocator());
	}
	String filename(TESTFILE);
	filename.Append(".html");
	t_assert(0 == Coast::System::IO::unlink(filename));
}

void HTMLCacheLoaderTest::SimpleBuildCacheTest()
{
	StartTrace(HTMLCacheLoaderTest.SimpleBuildCacheTest);
	const String TEMPLATE("<b>a simple template</b>");
	IStringStream is(TEMPLATE);
	Anything cache;
	TemplateParser tp;
	HTMLTemplateCacheLoader htcl(&tp);
	htcl.BuildCache(cache, is);
	t_assert(!cache.IsNull());
	assertEqual(1, cache.GetSize());
//modern cache format:
	assertEqual(TEMPLATE, cache[0L].AsString());
// classic template cache format:
//	assertEqual(0,cache[0L][0L].AsLong(-1));
//	assertEqual(TEMPLATE,cache[0L][1L].AsString());
//	assertEqual(2,cache[0L].GetSize());
}
void HTMLCacheLoaderTest::SimpleMacroBuildCacheTest()
{
	StartTrace(HTMLCacheLoaderTest.SimpleMacroBuildCacheTest);
	const String TEMPLATE("a macro [[#wd HTMLTemplateRenderer { /Template bar }]] with text");
	IStringStream is(TEMPLATE);
	Anything cache;
	TemplateParser tp;
	HTMLTemplateCacheLoader htcl(&tp);
	htcl.BuildCache(cache, is);
	t_assert(!cache.IsNull());
	assertEqual(3, cache.GetSize());
#if 1 // new cache format
	t_assert(cache[0L].GetType() == AnyCharPtrType);
	assertCharPtrEqual("a macro ", cache[0L].AsString());
	t_assert(cache[2L].GetType() == AnyCharPtrType);
	assertCharPtrEqual(" with text", cache[2L].AsString());
	t_assert(cache[1L].GetType() == AnyArrayType);
	String rname = cache[1L].SlotName(0L);
	assertCharPtrEqual("HTMLTemplateRenderer", cache[1L].SlotName(0L));
	t_assert(cache[1L]["HTMLTemplateRenderer"].IsDefined("Template"));
	assertCharPtrEqual("bar", cache[1L]["HTMLTemplateRenderer"]["Template"].AsString());

#else // classic cache format
	assertEqual(2, cache[0L].GetSize());
	assertEqual(3, cache[1L].GetSize());
	assertEqual(2, cache[2L].GetSize());
	assertEqual(0, cache[0L][0L].AsLong(-1));
	assertEqual(2, cache[1L][0L].AsLong(-1));
	assertEqual(0, cache[2L][0L].AsLong(-1));

	assertCharPtrEqual("a macro ", cache[0L][1L].AsString());
	assertCharPtrEqual(" with text", cache[2L][1L].AsString());
	Renderer *r = Renderer::FindRenderer("HTMLTemplateRenderer");
	t_assertm(r, "excpected HTMLTemplateRenderer to be there");
	t_assert(r == cache[1L][1L].AsIFAObject());

	t_assert(cache[1L][2L].IsDefined("Template"));
	assertCharPtrEqual("bar", cache[1L][2L]["Template"].AsString());
#endif
}
void HTMLCacheLoaderTest::SimpleCommentBuildCacheTest()
{
	StartTrace(HTMLCacheLoaderTest.SimpleCommentBuildCacheTest);
	const String TEMPLATE("a macro <!--#wd HTMLTemplateRenderer { /Template bar } --> with text");
	IStringStream is(TEMPLATE);
	Anything cache;
	TemplateParser tp;
	HTMLTemplateCacheLoader htcl(&tp);
	htcl.BuildCache(cache, is);
	t_assert(!cache.IsNull());
	assertEqual(3, cache.GetSize());
#if 1
	t_assert(cache[0L].GetType() == AnyCharPtrType);
	assertCharPtrEqual("a macro ", cache[0L].AsString());
	t_assert(cache[2L].GetType() == AnyCharPtrType);
	assertCharPtrEqual(" with text", cache[2L].AsString());
	t_assert(cache[1L].GetType() == AnyArrayType);
	String rname = cache[1L].SlotName(0L);
	assertCharPtrEqual("HTMLTemplateRenderer", cache[1L].SlotName(0L));
	t_assert(cache[1L]["HTMLTemplateRenderer"].IsDefined("Template"));
	assertCharPtrEqual("bar", cache[1L]["HTMLTemplateRenderer"]["Template"].AsString());
#else // classic cache format
	assertEqual(2, cache[0L].GetSize());
	assertEqual(3, cache[1L].GetSize());
	assertEqual(2, cache[2L].GetSize());
	assertEqual(0, cache[0L][0L].AsLong(-1));
	assertEqual(2, cache[1L][0L].AsLong(-1));
	assertEqual(0, cache[2L][0L].AsLong(-1));

	assertCharPtrEqual("a macro <!--#wd HTMLTemplateRenderer { /Template bar } -->", cache[0L][1L].AsString());
	assertCharPtrEqual(" with text", cache[2L][1L].AsString());
	Renderer *r = Renderer::FindRenderer("HTMLTemplateRenderer");
	t_assertm(r, "excpected HTMLTemplateRenderer to be there");
	t_assert(r == cache[1L][1L].AsIFAObject());

	t_assert(cache[1L][2L].IsDefined("Template"));
	assertCharPtrEqual("bar", cache[1L][2L]["Template"].AsString());
#endif
}
void HTMLCacheLoaderTest::ConsecutiveCommentBuildCacheTest()
{
	StartTrace(HTMLCacheLoaderTest.ConsecutiveCommentBuildCacheTest);
	const String TEMPLATE("test <!--#wd Lookup foo ---><!-- hui --><!--#wd ContextLookupRenderer bar -->");
	IStringStream is(TEMPLATE);
	Anything cache;
	TemplateParser tp;
	HTMLTemplateCacheLoader htcl(&tp);
	htcl.BuildCache(cache, is);
	t_assert(!cache.IsNull());
	assertEqual(4, cache.GetSize());

#if 1
	assertCharPtrEqual("test ", cache[0L].AsString());
	assertCharPtrEqual("<!-- hui -->", cache[2L].AsString());
	assertCharPtrEqual("Lookup", cache[1L].SlotName(0L));
	assertCharPtrEqual("ContextLookupRenderer", cache[3L].SlotName(0L));
	assertCharPtrEqual("foo", cache[1L]["Lookup"][0L].AsString());
	assertCharPtrEqual("bar", cache[3L]["ContextLookupRenderer"][0L].AsString());
#else // classic cache format

	assertEqual(2, cache[0L].GetSize());
	assertEqual(3, cache[1L].GetSize());
	assertEqual(2, cache[2L].GetSize());
	assertEqual(3, cache[3L].GetSize());
	assertEqual(0, cache[0L][0L].AsLong(-1));
	assertEqual(2, cache[1L][0L].AsLong(-1));
	assertEqual(0, cache[2L][0L].AsLong(-1));
	assertEqual(2, cache[3L][0L].AsLong(-1));

	assertCharPtrEqual("test <!--#wd Lookup foo --->", cache[0L][1L].AsString());
	assertCharPtrEqual("<!-- hui --><!--#wd ContextLookupRenderer bar -->", cache[2L][1L].AsString());
	Renderer *r = Renderer::FindRenderer("ContextLookupRenderer");
	t_assertm(r, "excpected ContextLookupRenderer to be there");
	t_assert(r == cache[1L][1L].AsIFAObject());
	t_assert(r == cache[3L][1L].AsIFAObject());
#endif
}

void HTMLCacheLoaderTest::CheckCacheIsLoaded()
{
	StartTrace(HTMLCacheLoaderTest.CheckCacheIsLoaded);
	Anything config;
	config["TemplatesDir"] = "config/HTMLTemplates";
	HTMLTemplateRenderer::BuildCache(config);
	t_assert(HTMLTemplateRenderer::fgTemplates.GetSize() > 0);
	t_assert(HTMLTemplateRenderer::fgNameMap.GetSize() > 0);
}

// builds up a suite of testcases, add a line for each testmethod
Test *HTMLCacheLoaderTest::suite ()
{
	StartTrace(HTMLCacheLoaderTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, HTMLCacheLoaderTest, LoadEmptyCacheTest);
	ADD_CASE(testSuite, HTMLCacheLoaderTest, SimpleBuildCacheTest);
	ADD_CASE(testSuite, HTMLCacheLoaderTest, SimpleMacroBuildCacheTest);
	ADD_CASE(testSuite, HTMLCacheLoaderTest, SimpleCommentBuildCacheTest);
	ADD_CASE(testSuite, HTMLCacheLoaderTest, ConsecutiveCommentBuildCacheTest);
	ADD_CASE(testSuite, HTMLCacheLoaderTest, CheckCacheIsLoaded);
	return testSuite;
}
