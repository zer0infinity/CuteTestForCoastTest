/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "TemplateParserTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "TemplateParser.h"

//--- standard modules used ----------------------------------------------------
#include "Renderer.h"
#include "CacheHandler.h"
#include "AnyUtils.h"

//--- c-modules used -----------------------------------------------------------

//---- TemplateParserTest ----------------------------------------------------------------
TemplateParserTest::TemplateParserTest(TString tstrName)
	: TestCaseType(tstrName)
{
	StartTrace(TemplateParserTest.TemplateParserTest);
}

TemplateParserTest::~TemplateParserTest()
{
	StartTrace(TemplateParserTest.Dtor);
}

void TemplateParserTest::BuildEmptyCache()
{
	StartTrace(TemplateParserTest.BuildEmptyCache);
	TemplateParser p;
	IStringStream is("");
	Anything cache;
	cache = p.Parse(is);
	t_assert(cache.IsNull());
}

void TemplateParserTest::BuildLiteralCache()
{
	StartTrace(TemplateParserTest.BuildLiteralCache);
	TemplateParser p;

	IStringStream is("hello world");
	Anything cache;
	cache = p.Parse(is);
	t_assert(!cache.IsNull());
	assertEqual("hello world", cache[0L].AsCharPtr());
}

void TemplateParserTest::BuildSimpleFormCache()
{
	StartTrace(TemplateParserTest.BuildSimpleFormCache);
	TemplateParser p;

	// determine Coast actions, PreprocessAction is always defined...
	String form("<form method=GET action=PreprocessAction>hello world</form>");
	IStringStream is(form);
	Anything cache;
	cache = p.Parse(is);
	t_assert(!cache.IsNull());
	t_assert(cache[0L].IsDefined("FormRenderer"));
	t_assert(cache[0L]["FormRenderer"].IsDefined("Method"));
	assertEqual("GET", cache[0L]["FormRenderer"]["Method"].AsCharPtr());
	t_assert(cache[0L]["FormRenderer"].IsDefined("Action"));
	assertEqual("PreprocessAction", cache[0L]["FormRenderer"]["Action"].AsCharPtr());
	t_assert(cache[0L]["FormRenderer"].IsDefined("Layout"));
	assertEqual("hello world", cache[0L]["FormRenderer"]["Layout"][0L].AsCharPtr());
}

void TemplateParserTest::BuildSimpleFormNoCache()
{
	StartTrace(TemplateParserTest.BuildSimpleFormCache);
	TemplateParser p;

	String form(_QUOTE_(<form METHOD=GET action="http:../cgi-bin/doit.cgi">hello world</form>));
	IStringStream is(form);
	Anything cache;
	cache = p.Parse(is);
	t_assert(!cache.IsNull());
	t_assert(!cache[0L].IsDefined("FormRenderer"));
	assertEqual(_QUOTE_(<form method="GET" action="http:../cgi-bin/doit.cgi">hello world</form>), cache[0L].AsCharPtr());
}

void TemplateParserTest::BuildSimpleRendererSpec()
{
	StartTrace(TemplateParserTest.BuildSimpleRendererSpec);
	TemplateParser p;

	String templ(_QUOTE_(start <? /ContextLookupRenderer { "name" } ?> end));
	IStringStream is(templ);
	Anything cache;
	cache = p.Parse(is);
	t_assert(!cache.IsNull());
	String result;
	Context ctx;
	ctx.GetTmpStore()["name"] = "Peter";
	Renderer::RenderOnString(result, ctx, cache);
	assertEqual("start Peter end", result);
}
void TemplateParserTest::BuildNestedHTMLSpec()
{
	StartTrace(TemplateParserTest.BuildNestedHTMLSpec);
	TemplateParser p;

	String templ(_QUOTE_(start <? /ConditionalRenderer { /ContextCondition "name" /Defined ?>Ist "Definiert"<? /Undefined ?>nicht def#<% } %> end));
	IStringStream is(templ);
	Anything cache;
	cache = p.Parse(is);
	t_assert(!cache.IsNull());
	String result;
	Context ctx;
	Renderer::RenderOnString(result, ctx, cache);
	assertEqual("start nicht def# end", result);
	result = "";
	ctx.GetTmpStore()["name"] = "Peter";
	Renderer::RenderOnString(result, ctx, cache);
	assertEqual(_QUOTE_(start Ist "Definiert" end), result);
}

void TemplateParserTest::BuildWithEmptyXHTMLTag() {
	StartTrace(TemplateParserTest.BuildWithEmptyXHTMLTag);
	TemplateParser p;

	String form(_QUOTE_(some<br />test<img SRC="foo.jpg" />with <p>));
	IStringStream is(form);
	Anything cache;
	cache = p.Parse(is);
	t_assert(!cache.IsNull());
	assertEqual(_QUOTE_(some<br />test<img src="foo.jpg" />with <p>), cache[0L].AsCharPtr());
}
void TemplateParserTest::BuildFormWithInputFieldCache() {
	StartTrace(TemplateParserTest.BuildFormWithInputFieldCache);
	TemplateParser p;

	// determine Coast actions, PreprocessAction is always defined...
	String form("<form method=GET action=PreprocessAction><textarea name=area></textarea><input name=\"foo\" /><input type=submit name=abutton><input type=image name=animage></form>");
	IStringStream is(form);
	Anything cache;
	cache = p.Parse(is);
	t_assert(!cache.IsNull());
	t_assert(cache[0L].IsDefined("FormRenderer"));
	t_assert(cache[0L]["FormRenderer"].IsDefined("Method"));
	assertEqual("GET", cache[0L]["FormRenderer"]["Method"].AsCharPtr());
	t_assert(cache[0L]["FormRenderer"].IsDefined("Action"));
	assertEqual("PreprocessAction", cache[0L]["FormRenderer"]["Action"].AsCharPtr());
	t_assert(cache[0L]["FormRenderer"].IsDefined("Layout"));
	// test, when everything is ok
	String layout;
	Context ctx;
	Renderer::RenderOnString(layout, ctx, cache);
	t_assert(layout.Contains(_QUOTE_(name="fld_area")) >= 0);
	t_assert(layout.Contains(_QUOTE_(name="fld_foo")) >= 0);
	t_assert(layout.Contains(_QUOTE_(name="b_abutton")) >= 0);
	t_assert(layout.Contains(_QUOTE_(name="i_animage")) >= 0);
	layout = cache[0L]["FormRenderer"]["Layout"][0L].AsCharPtr();
	t_assert(layout.Contains(_QUOTE_(name="fld_area")) >= 0);
	t_assert(layout.Contains(_QUOTE_(name="fld_foo")) >= 0);
	t_assert(layout.Contains(_QUOTE_(name="b_abutton")) >= 0);
	t_assert(layout.Contains(_QUOTE_(name="i_animage")) >= 0);

}
void TemplateParserTest::BuildCacheWithMacro() {
	StartTrace(TemplateParserTest.BuildCacheWithMacro);
	TemplateParser p;

	String templ("start <? /ContextLookupRenderer { \"name\" } ?> ");
	templ.Append("is [[#wd ContextLookupRenderer  name]] ");
	templ.Append("and <!-- #wd ContextLookupRenderer name --> end");
	IStringStream is(templ);
	Anything cache;
	cache = p.Parse(is);
	t_assert(!cache.IsNull());
	String result;
	Context ctx;
	ctx.GetTmpStore()["name"] = "Peter";
	Renderer::RenderOnString(result, ctx, cache);
	assertEqual("start Peter is Peter and Peter end", result);
}
void TemplateParserTest::MacroWithinTag() {
	StartTrace(TemplateParserTest.MacroWithinTag);
	TemplateParser p;

	String templ("start <img src=[[#wd ContextLookupRenderer myimage]]> end");
	IStringStream is(templ);
	Anything cache;
	cache = p.Parse(is);
	t_assert(!cache.IsNull());
	String result;
	Context ctx;
	ctx.GetTmpStore()["myimage"] = "../images/foo.jpg";
	Renderer::RenderOnString(result, ctx, cache);
	assertEqual(_QUOTE_(start <img src="../images/foo.jpg"> end), result);
}
void TemplateParserTest::MacroWithinTagWithinQuote() {
	StartTrace(TemplateParserTest.MacroWithinTagWithinQuote);
	TemplateParser p;

	String templ("start <img src=\"../images/[[#wd ContextLookupRenderer myimage]].jpg\"> end");
	IStringStream is(templ);
	Anything cache;
	cache = p.Parse(is);
	t_assert(!cache.IsNull());
	String result;
	Context ctx;
	ctx.GetTmpStore()["myimage"] = "foo";
	Renderer::RenderOnString(result, ctx, cache);
	assertEqual(_QUOTE_(start <img src="../images/foo.jpg"> end), result);
}

void TemplateParserTest::InvalidMacroTests() {
	StartTrace(TemplateParserTest.InvalidMacroTest);
	InvalidMacroTest("start [[#invalid ]] end");
	InvalidMacroTest("start [[#wdinvalid ]] end");
	InvalidMacroTest("start [ end");
	InvalidMacroTest("start [[ end");
	InvalidMacroTest("start [[# end");
	InvalidMacroTest("start [[#w end");
}
void TemplateParserTest::InvalidMacroTest(const char * invalidmacro) {
	StartTrace(TemplateParserTest.InvalidMacroTest);
	TemplateParser p;

	String templ(invalidmacro);
	IStringStream is(templ);
	Anything cache;
	cache = p.Parse(is);
	t_assert(!cache.IsNull());
	assertEqual(templ, cache[0L].AsCharPtr(""));
}
void TemplateParserTest::QuotelessAttributes() {
	StartTrace(TemplateParserTest.QuotelessAttributes);
	TemplateParser p;

	String templ("<table border=3 width=100% bgcolor=#6699cc>");
	String correctlyquoted("<table border=\"3\" width=\"100%\" bgcolor=\"#6699cc\">");
	IStringStream is(templ);
	Anything cache;
	cache = p.Parse(is);
	t_assert(!cache.IsNull());
	assertEqual(correctlyquoted, cache[0L].AsCharPtr(""));

}
void TemplateParserTest::AnythingBracesWrong() {
	StartTrace(TemplateParserTest.AnythingBracesWrong);
	TemplateParser p;
	{
		String templ("<% { } } %>");
		IStringStream is(templ);
		Anything cache;
		cache = p.Parse(is);
		t_assert(!cache.IsNull());
		// just trigger the error message...
		String result;
		Context ctx;
		Renderer::RenderOnString(result, ctx, cache);
		assertEqual("", result);
	}
	{
		String templ("<% { %>foo");
		IStringStream is(templ);
		Anything cache;
		cache = p.Parse(is);
		t_assert(!cache.IsNull());
		// just trigger the error message...unexpected eof
		String result;
		Context ctx;
		Renderer::RenderOnString(result, ctx, cache);
		assertEqual("foo", result);
	}
}

void TemplateParserTest::NoTagWithinJavascript() {
	StartTrace(TemplateParserTest.NoTagWithinJavascript);
	TemplateParser p;

	String templ("<script language=\"javascript\"> if ( 3 < 5) true; else false; </script>");
	IStringStream is(templ);
	Anything cache;
	cache = p.Parse(is);
	t_assert(!cache.IsNull());
	String result;
	Context ctx;
	Renderer::RenderOnString(result, ctx, cache);
	assertEqual(templ, result);
}

void TemplateParserTest::BuildFormWithConfiguredTransitionTokens() {
	StartTrace(TemplateParserTest.BuildFormWithConfiguredTransitionTokens);
	CacheHandler *pCache = CacheHandler::Get();
	if ( t_assert(pCache != NULL) ) {
		ROAnything roaHTMLCache, roaCacheNameMap;
		roaHTMLCache = pCache->GetGroup("HTML");
		roaCacheNameMap = pCache->Get("HTMLMappings", "HTMLTemplNameMap");
		TraceAny(roaHTMLCache, "HTML");
		TraceAny(roaCacheNameMap, "NameMap");
		ROAnything roaTestPage = roaHTMLCache[roaCacheNameMap["TemplateParserExtendedTest"][0L].AsCharPtr()];
		TraceAny(roaTestPage, "content of page");
		if ( t_assertm(!roaTestPage.IsNull(), "expected to get a valid reference to the page") ) {
			String configFilename("TemplateParserExtendedTestMaster");
			istream *ifp = System::OpenStream(configFilename, "any");
			if ( t_assert(ifp != NULL) ) {
				Anything anyMaster;
				anyMaster.Import(*ifp, configFilename);
				delete ifp;
				assertAnyCompareEqual(anyMaster, roaTestPage, configFilename << ".any", '.', ':');
			}
		}
	}
}

// builds up a suite of testcases, add a line for each testmethod
Test * TemplateParserTest::suite () {
	StartTrace(TemplateParserTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, TemplateParserTest, BuildEmptyCache);
	ADD_CASE(testSuite, TemplateParserTest, BuildLiteralCache);
	ADD_CASE(testSuite, TemplateParserTest, BuildSimpleFormCache);
	ADD_CASE(testSuite, TemplateParserTest, BuildSimpleFormNoCache);
	ADD_CASE(testSuite, TemplateParserTest, BuildSimpleRendererSpec);
	ADD_CASE(testSuite, TemplateParserTest, BuildNestedHTMLSpec);
	ADD_CASE(testSuite, TemplateParserTest, BuildWithEmptyXHTMLTag);
	ADD_CASE(testSuite, TemplateParserTest, BuildFormWithInputFieldCache);
	ADD_CASE(testSuite, TemplateParserTest, BuildCacheWithMacro);
	ADD_CASE(testSuite, TemplateParserTest, MacroWithinTag);
	ADD_CASE(testSuite, TemplateParserTest, MacroWithinTagWithinQuote);
	ADD_CASE(testSuite, TemplateParserTest, InvalidMacroTests);
	ADD_CASE(testSuite, TemplateParserTest, QuotelessAttributes);
	ADD_CASE(testSuite, TemplateParserTest, AnythingBracesWrong);
	ADD_CASE(testSuite, TemplateParserTest, NoTagWithinJavascript);
	ADD_CASE(testSuite, TemplateParserTest, BuildFormWithConfiguredTransitionTokens);

	return testSuite;
}
