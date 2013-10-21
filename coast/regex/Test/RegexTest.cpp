/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "TestSuite.h"
#include "RE.h"
#include "RegexTest.h"
#include "SystemFile.h"
#include "StringStream.h"
#include "RECompiler.h"
#include "PoolAllocator.h"
#include "TestTimer.h"
#include "Tracer.h"

using namespace coast;

//---- RegexTest ----------------------------------------------------------------
RegexTest::RegexTest(TString tstrName) : TestCaseType(tstrName)
{
	StartTrace(RegexTest.Ctor);
}

RegexTest::~RegexTest()
{
	StartTrace(RegexTest.Dtor);
}

void RegexTest::MatchLiteral()
{
	StartTrace(RegexTest.MatchLiteral);

	String suspect("hello world!\n");

	RE tomatch("hello");
	t_assert(tomatch.ContainedIn(suspect));
	t_assert(!tomatch.ContainedIn("foo"));
}
void RegexTest::MatchDotDotDot()
{
	StartTrace(RegexTest.MatchLiteral);

	String suspect("hello world!\n");

	RE tomatch("...");
	long s = 0, l = 0;
	t_assert(tomatch.ContainedIn(suspect));
	t_assert(tomatch.Match(suspect, s, l));
	assertEqual(0L, s);
	assertEqual(3L, l);
	s = suspect.Length() - 2;
	l = 0;
	t_assert(! tomatch.Match(suspect, s, l)); // shor match
}
void RegexTest::MatchDot()
{
	StartTrace(RegexTest.MatchLiteral);

	String suspect("hello world!\n");

	RE tomatch(".");
	long s = 0, l = 0;

	t_assert(tomatch.ContainedIn(suspect));
	t_assert(tomatch.Match(suspect, s, l));
	assertEqual(0L, s);
	assertEqual(1L, l);
	s = l = 0L;

	tomatch.setProgram(RECompiler().compile("h"));
	t_assert(tomatch.ContainedIn(suspect));
	t_assert(tomatch.Match(suspect, s, l));
	assertEqual(0L, s);
	assertEqual(1L, l);
	s = l = 0L;

	tomatch.setProgram(RECompiler().compile("he..o"));
	t_assert(tomatch.ContainedIn(suspect));
	t_assert(tomatch.Match(suspect, s, l));
	assertEqual(0L, s);
	assertEqual(5L, l);
	s = l = 0L;

	tomatch.setProgram(RECompiler().compile("w"));
	t_assert(tomatch.ContainedIn(suspect));
	t_assert(tomatch.Match(suspect, s, l));
	assertEqual(6L, s);
	assertEqual(1L, l);
	s = l = 0L;

	tomatch.setProgram(RECompiler().compile("wor..!"));
	t_assert(tomatch.ContainedIn(suspect));
	t_assert(tomatch.Match(suspect, s, l));
	assertEqual(6L, s);
	assertEqual(6L, l);
	s = l = 0L;

	tomatch.setProgram(RECompiler().compile("he...o"));
	t_assert(!tomatch.Match(suspect, s, l));
	assertEqual(0L, l);
	t_assert(!tomatch.ContainedIn(suspect));
	s = l = 0L;

	tomatch.setProgram(RECompiler().compile("wo.d"));
	t_assert(!tomatch.Match(suspect, s, l));
	assertEqual(0L, l);
	t_assert(!tomatch.ContainedIn(suspect));

}
void RegexTest::MatchDotStar()
{
	StartTrace(RegexTest.MatchLiteral);

	String suspect("hello world!\n");

	RE tomatch(".+");
	long s = 0, l = 0;

	t_assert(tomatch.ContainedIn(suspect));
	t_assert(tomatch.Match(suspect, s, l));
	assertEqual(0L, s);
	assertEqual(suspect.Length() - 1, l);
	Trace( "start:" << s << " len:" << l);
}
void RegexTest::MatchAStar()
{
	StartTrace(RegexTest.MatchLiteral);

	String suspect("hello world!\n");

	RE tomatch("l*l");
	long s = 0, l = 0;

	t_assert(tomatch.ContainedIn(suspect));
	t_assert(tomatch.Match(suspect, s, l));
	assertEqual(2L, s);
	assertEqual(2L, l);
	s = l = 0;
	t_assert(tomatch.Match("llll", s, l));
	assertEqual(0L, s);
	assertEqual(4L, l);
	t_assert(tomatch.Match("006llllllandsomestuff", s, l));
	assertEqual(3L, s);
	assertEqual(6L, l);
}

void RegexTest::LeftMostLongestTest()
{
	StartTrace(RegexTest.LeftMostLongestTest);
	RE tomatch("(abcd|abc)(d|)");
	// does fail for (abc|abcd)(d|) as RE, so
	// leftmost longest for lowest subexpr isn't implemented (yet)
	long s = 0, l = 0;
	t_assert(tomatch.Match("abcd", s, l));
	assertEqual(0, s);
	assertEqual(4, l);
	assertEqual(0, tomatch.GetStartRegister(1));
	assertEqual(4, tomatch.GetEndRegister(1));
	assertCharPtrEqual("abcd", tomatch.GetMatch(1));
	assertEqual(4, tomatch.GetStartRegister(2));
	assertEqual(4, tomatch.GetEndRegister(2));
	assertCharPtrEqual("", tomatch.GetMatch(2));
}

void RegexTest::GetMatchTest()
{
	StartTrace(RegexTest.GetMatchTest);
	{
		RE tomatch("\\btest\\b");
		t_assert(tomatch.ContainedIn("this is a test string"));
		assertCharPtrEqual("test", tomatch.GetMatch(0));
	}
	{
		RE tomatch("((\\bt\\w+)(.*\\2))");
		t_assert(tomatch.ContainedIn("now test this test string"));
		assertCharPtrEqual("test this test", tomatch.GetMatch(0));
		assertCharPtrEqual("test this test", tomatch.GetMatch(1));
		assertCharPtrEqual("test", tomatch.GetMatch(2));
		assertCharPtrEqual(" this test", tomatch.GetMatch(3));
	}
}


void RegexTest::BackRefTest()
{
	StartTrace(RegexTest.BackRefTest);

	RE emptybackref("(a)*-\\1b");
	t_assert(emptybackref.ContainedIn("a-ab"));
	t_assert(emptybackref.ContainedIn("aa-ab"));
	t_assert(!emptybackref.ContainedIn("aa-aab"));
	t_assert(!emptybackref.ContainedIn("-b"));
	t_assert(!emptybackref.ContainedIn("aaa-b"));

	RE complexbackref("((a|b{10,11})|(b))*-\\2");
	t_assert(complexbackref.ContainedIn("aaab-a"));
	t_assert(complexbackref.ContainedIn("bbbbbbbbbb-bbbbbbbbbb"));
	t_assert(!complexbackref.ContainedIn("bbbbbbbbb-bbbbbbbbbb"));
}

void RegexTest::MatchCharTest()
{
	StartTrace(RegexTest.MatchCharTest);
}

// make the following a real regexp by using . for the a, because otherwise
// we just time string compare, because of the prefix optimization
// with a single character prefix, it seems to be fair enough, otherwise
// comparison takes too long for useful tests (several seconds)
static const char *gLiteralExpr = "b.ck-tracking oriented stream-of-solution functions";
void RegexTest::LargeLiteralTest()
{
	StartTrace(RegexTest.LargeLiteralTest);
	RE literal(gLiteralExpr);
	long s = 0, l = 0;
	// should be timed:
	std::istream *pStream = system::OpenStream("LargeLiteralData", "h");
	if (t_assert(pStream != NULL)) {
		String largeData;
		while (pStream->good()) {
			largeData.Append(*pStream, 1000L);
		}
		Trace("Large data: " << largeData);
		t_assert(literal.Match(largeData, s, l));
		assertEqual(l, strlen(gLiteralExpr));
		Trace("large literal matched at " << s << " with len " << l);
	}
	delete pStream;
}

void RegexTest::LargeDotStarTest()
{
	StartTrace(RegexTest.LargeLiteralTest);
	const char *gDotStarExpr = "back-tracking .* functions";
	RE literal(gDotStarExpr);
	long s = 0, l = 0;
	// should be timed:
	std::istream *pStream = system::OpenStream("LargeLiteralData", "h");
	if (t_assert(pStream != NULL)) {
		String largeData;
		while (pStream->good()) {
			largeData.Append(*pStream, 1000L);
		}
		t_assert(literal.Match(largeData, s, l));
		assertEqual(l, strlen(gLiteralExpr));
		Trace("large dot star matched at " << s << " with len " << l);
	}
	delete pStream;
}

#include "ShortLiteralData.h"
void RegexTest::ShortLiteralTest()
{
	StartTrace(RegexTest.ShortLiteralTest);
	RE literal(gLiteralExpr);
	long s = 0, l = 0;
	// should be timed:
	for (unsigned line = 0; line < sizeof(gShortLiteralData) / sizeof(const char *); ++line) {
		if (literal.Match(gShortLiteralData[line], s, l)) {
			assertEqual(l, strlen(gLiteralExpr));
			assertEqual(4738, line);
			Trace("short literal matched at line " << (long)line << " with len " << l );
		}
	}
}

void RegexTest::MatchAnything(long id, Anything aCase)
{
	StartTrace(RegexTest.MatchAnything);
	TraceAny(aCase, "testcase");
	long lineId = id + 10; // adjust line count offset for easy navigation in RegexTest.any
	t_assertm(aCase.GetSize() >= 2, TString("short RE testcase spec at\nRegexTest.any:") << lineId);
	const char *re = aCase[1L].AsCharPtr();
	RE r(re);
	long expected = aCase[0L].AsLong(0);
	if (expected >= 2) {
		t_assertm(!r.IsValid(), TString("expected compile to fail at\nRegexTest.any:") << lineId << " for >" << re << "<");
		return;
	}
	t_assertm(r.IsValid(), TString("excpected successfull compile at\nRegexTest.any:") << lineId << " for >" << re << "<");
	if (!r.IsValid()) {
		return;
	}
	long s = 0, l = 0;
	const char *search = aCase[2L].AsCharPtr(0);
	t_assertm(search != NULL, TString("missing search at ") << lineId);
	if (!search) {
		return;
	}
	bool match = r.Match(search, s, l);
	t_assertm(match == (expected == 0), TString("expected match result ") << expected << " wrong at\nRegexTest.any:" << lineId << " for >" << re << "< with search >" << search << "<");
	assertEqualm(aCase[3L].AsLong(s), s, TString("expected match position wrong at\nRegexTest.any:") << lineId);
	assertEqualm(aCase[4L].AsLong(l), l, TString("expected match length wrong at\nRegexTest.any:") << lineId);
}
//:read config file and do all the tests
void RegexTest::MatchConfig()
{
	StartTrace(RegexTest.MatchConfig);

	std::istream *is = system::OpenStream("RegexTest", "any");
	t_assert(is && is->good());
	if (!is || !is->good()) {
		return;
	}
	Anything alltests;
	t_assert(alltests.Import(*is));
	t_assert(alltests.GetSize() > 0);
	for (long id = 0; id < alltests.GetSize(); ++id) {
		MatchAnything(id, alltests[id]);
	}
}

void RegexTest::MatchFlagsTest()
{
	StartTrace(RegexTest.MatchFlagsTest);

	RE r("^.b+$", RE::MATCH_ICASE);

	t_assert(r.ContainedIn("ABB"));
	t_assert(r.ContainedIn("aBB"));
	t_assert(r.ContainedIn("abb"));
	t_assert(!r.ContainedIn("aBBc\naBb"));

	r.SetMatchFlags(RE::ICASE_MULTILINE);
	t_assert(r.ContainedIn("ABB"));
	t_assert(r.ContainedIn("aBB"));
	t_assert(r.ContainedIn("abb"));
	t_assert(r.ContainedIn("aBBc\naBb"));

	r.SetMatchFlags(RE::DOT_MATCHES_NEWLINE);
	t_assert(r.ContainedIn("\nbbb"));
	t_assert(!r.ContainedIn("ABB"));
	t_assert(!r.ContainedIn("aBB"));
	t_assert(r.ContainedIn("abb"));
	t_assert(!r.ContainedIn("aBBc\naBb"));
}

void RegexTest::SplitTest()
{
	StartTrace(RegexTest.SplitTest);

	String tosplit("abbbaabbaaabaaaa");
	RE splitter("b+");
	Anything a = splitter.Split(tosplit);
	TraceAny(a, "splitted");
	assertEqual(4, a.GetSize());
	assertEqual("a", a[0L].AsCharPtr());
	assertEqual("aa", a[1L].AsCharPtr());
	assertEqual("aaa", a[2L].AsCharPtr());
	assertEqual("aaaa", a[3L].AsCharPtr());
}

void RegexTest::SubstTest()
{
	StartTrace(RegexTest.SubstTest);

	String tosubst("replace the\tblanks    or tabs\t    within");
	RE subster("\\s+");
	String res = subster.Subst(tosubst, "#", true);
	assertEqual("replace#the#blanks#or#tabs#within", res);
	res = subster.Subst(tosubst, "#", false);
	assertEqual("replace#the\tblanks    or tabs\t    within", res);
	RE subster2("[ \t]+");
	res = subster2.Subst(tosubst, "#", true);
	assertEqual("replace#the#blanks#or#tabs#within", res);
}

void RegexTest::GrepTest()
{
	StartTrace(RegexTest.GrepTest);

	std::istream *is = system::OpenStream("Tracer", "any");
	t_assert(is && is->good());
	if (!is || !is->good()) {
		return;
	}
	Anything a;
	t_assert(a.Import(*is));
	t_assert(a.GetSize() > 0);
	t_assert(a.IsDefined("RegexTest"));

	RE zeroes("0+");
	Anything res = zeroes.Grep(a["RegexTest"]);
	t_assert(res.GetSize() > 0);
	t_assert(res.IsDefined("DummyForGrepTest"));
	TraceAny(res, "zeroes matched");
}

void RegexTest::GrepSlotNamesTest()
{
	StartTrace(RegexTest.GrepSlotNamesTest);

	std::istream *is = system::OpenStream("Tracer", "any");
	t_assert(is && is->good());
	if (!is || !is->good()) {
		return;
	}
	Anything a;
	t_assert(a.Import(*is));
	t_assert(a.GetSize() > 0);
	t_assert(a.IsDefined("RegexTest"));

	RE tests("^.*Test$");
	Anything res = tests.GrepSlotNames(a);
	TraceAny(res, "slot names ending in Test");
	t_assert(res.GetSize() > 0);
	t_assert(res.IsDefined("RegexTest"));
}

// builds up a suite of testcases, add a line for each testmethod
Test *RegexTest::suite ()
{
	StartTrace(RegexTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, RegexTest, MatchLiteral);
	ADD_CASE(testSuite, RegexTest, MatchDot);
	ADD_CASE(testSuite, RegexTest, MatchDotDotDot);
	ADD_CASE(testSuite, RegexTest, MatchDotStar);
	ADD_CASE(testSuite, RegexTest, MatchAStar);
	ADD_CASE(testSuite, RegexTest, LargeLiteralTest);
	ADD_CASE(testSuite, RegexTest, BackRefTest);
	ADD_CASE(testSuite, RegexTest, ShortLiteralTest);
	ADD_CASE(testSuite, RegexTest, LargeDotStarTest);
	ADD_CASE(testSuite, RegexTest, MatchConfig);
	ADD_CASE(testSuite, RegexTest, MatchFlagsTest);
	ADD_CASE(testSuite, RegexTest, SplitTest);
	ADD_CASE(testSuite, RegexTest, SubstTest);
	ADD_CASE(testSuite, RegexTest, GrepTest);
	ADD_CASE(testSuite, RegexTest, GrepSlotNamesTest);
	ADD_CASE(testSuite, RegexTest, GetMatchTest);
	return testSuite;
}
