/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "Threads.h"
#include "Context.h"
#include "Dbg.h"
#include "SecurityModule.h"
#include "Application.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "URLFilter.h"

//--- interface include --------------------------------------------------------
#include "URLFilterTest.h"

//---- URLFilterTest ----------------------------------------------------------------
URLFilterTest::URLFilterTest(TString tname) : ConfiguredTestCase(tname, "Config")
{
}

URLFilterTest::~URLFilterTest()
{
}

void URLFilterTest::setUp ()
// setup connector for this TestCase
{
	ConfiguredTestCase::setUp();
	fConfig["Modules"].Append("SecurityModule");
	Application::InitializeGlobalConfig(fConfig);
	WDModule::Install(fConfig);
}

void URLFilterTest::tearDown ()
{
	t_assert(fConfig.IsDefined("Modules"));
	t_assert(fConfig["Modules"].Contains("SecurityModule"));
	WDModule::Terminate(fConfig);
	Application::InitializeGlobalConfig(Anything());
	ConfiguredTestCase::tearDown();
}

// handle simple things first
void URLFilterTest::DoFilterStateTest()
{
	StartTrace(URLFilterTest.DoFilterStateTest);
	Anything query;
	URLFilter urlFilter("Test");
	long querySz;

	query["slot1"] = "xxx";
	Anything dummy;
	Context ctx(dummy, query, 0, 0, 0, 0);

	// now try some simple cases
	// slot is there should be removed
	querySz = query.GetSize();
	t_assertm(urlFilter.DoFilterState(query, "slot1", ctx) && !query.IsDefined("slot1"), "expected slot1 to be removed");
	t_assertm(query.GetSize() == querySz - 1, "expected size to be one less");

	// slot is not there nothing should happen
	querySz = query.GetSize();
	t_assert(urlFilter.DoFilterState(query, "slot2", ctx));
	t_assertm(!query.IsDefined("slot2"), "expected slot2 not to be there");
	t_assertm(query.GetSize() == querySz, "expected size to be the same");

	// slot is null nothing should happen
	querySz = query.GetSize();
	t_assertm(!urlFilter.DoFilterState(query, 0, ctx), "expected query to be unchanged");
	t_assertm(query.GetSize() == querySz, "expected size to be the same");
}

// handle simple things first
void URLFilterTest::DoUnscrambleStateTest()
{
	StartTrace(URLFilterTest.DoUnscrambleStateTest);
	// first setup a state and scramble it
	Anything state;
	state["first"] = "test";
	state["second"] = "another test";
	String scrambledState;

	// scramble the state
	SecurityModule::ScrambleState(scrambledState, state);

	Anything query;
	query["first"] = "base";
	query["third"] = "derived";
	URLFilter urlFilter("Test");
	long querySz;
	Anything dummy;
	Context ctx(dummy, query, 0, 0, 0, 0);

	query["scrambledState"] = scrambledState;

	// use urlfilter to manipulate the query and extract the scrambled state
	// now try some simple cases
	// slot is there should be removed
	querySz = query.GetSize();
	t_assertm(urlFilter.DoUnscrambleState(query, "scrambledState", ctx) && !query.IsDefined("scrambledState"), "expected scrambledState to be removed");
	t_assertm(query.IsDefined("first"), "expected first to be there");
	t_assertm(query.IsDefined("second"), "expected second to be there");
	t_assertm(query.IsDefined("third"), "expected third to be there");
	assertEqual("base", query["first"].AsCharPtr(""));
	assertEqual("another test", query["second"].AsCharPtr(""));
	assertEqual("derived", query["third"].AsCharPtr(""));
	t_assert(query.GetSize() == querySz);

	// now try the failures
	// try unscramble a state that is not scrambled at all
	query["rottenScrambledState"] = "..xxx..xx.";
	querySz = query.GetSize();
	t_assert(!urlFilter.DoUnscrambleState(query, "rottenScrambledState", ctx));
	t_assertm(query.IsDefined("rottenScrambledState"), "expected rottenScrambledState to be there");
	t_assertm(query.GetSize() == querySz, "expected size to be the same");

	// try unscramble a state that is not there
	querySz = query.GetSize();
	t_assert(urlFilter.DoUnscrambleState(query, "slot2", ctx));
	t_assertm(!query.IsDefined("slot2"), "expected slot2 not to be there");
	t_assertm(query.GetSize() == querySz, "expected size to be the same");

	// try unscramble a null slot
	querySz = query.GetSize();
	t_assert(!urlFilter.DoUnscrambleState(query, 0, ctx));
	t_assertm(query.GetSize() == querySz, "expected size to be the same");

}

void URLFilterTest::FilterStateTest()
{
	StartTrace(URLFilterTest.FilterStateTest);

	Anything query;
	Anything filterTags;
	URLFilter urlFilter("Test");
	long querySz;

	// setup a query with some state
	query["slot1"] = "s1";
	query["slot2"] = "s2";
	query["slot3"] = "s3";
	Anything dummy;
	Context ctx(dummy, query, 0, 0, 0, 0);

	filterTags.Append("slot1");
	filterTags.Append("slot2");

	// now try some cases

	// slot are there and filterTags are correct should be removed
	querySz = query.GetSize();
	t_assert(urlFilter.FilterState(query, filterTags, ctx));
	t_assertm(!query.IsDefined("slot1"), "expected slot1 to be removed");
	t_assertm(!query.IsDefined("slot2"), "expected slot2 to be removed");
	t_assert(query.GetSize() == querySz - 2);

	// slot are not there and filterTags are correct nothing should happen
	// urlFilter accepts missing slots when filtering since the purpose
	// is security most of the time the specified slots won't be there
	querySz = query.GetSize();
	t_assert(urlFilter.FilterState(query, filterTags, ctx));
	t_assertm(!query.IsDefined("slot1"), "expected slot1 to be removed");
	t_assertm(!query.IsDefined("slot2"), "expected slot2 to be removed");
	t_assert(query.GetSize() == querySz);

	// try null anythings it should fail
	filterTags.Append(Anything());
	querySz = query.GetSize();
	t_assert(!urlFilter.FilterState(query, filterTags, ctx));
	t_assert(query.GetSize() == querySz);
	filterTags.Remove(filterTags.GetSize() - 1);

	// try complex filtering; filter value is equal to query value so it should be removed
	filterTags["slot3"] = "s3";
	querySz = query.GetSize();
	t_assert(urlFilter.FilterState(query, filterTags, ctx));
	t_assertm(!query.IsDefined("slot3"), "expected slot3 to be removed");
	t_assert(query.GetSize() == querySz - 1);

	// try complex filtering again; slot is not there so nothing should happen
	querySz = query.GetSize();
	t_assert(urlFilter.FilterState(query, filterTags, ctx));
	t_assertm(!query.IsDefined("slot3"), "expected slot3 to be removed");
	t_assert(query.GetSize() == querySz);

	// try more complex sample
	query["slot4"] = "s4";
	query["slot5"] = "s5";
	filterTags["slot4"].Append("s3");
	filterTags["slot4"].Append("s4");
	filterTags["slot4"].Append("s5");
	querySz = query.GetSize();
	t_assert(urlFilter.FilterState(query, filterTags, ctx));
	t_assertm(!query.IsDefined("slot4"), "expected slot4 to be removed");
	t_assert(query.GetSize() == querySz - 1);

}

void URLFilterTest::UnscrambleStateTest()
{
	StartTrace(URLFilterTest.UnscrambleStateTest);

	Anything query;
	Anything filterTags;
	URLFilter urlFilter("Test");
	long querySz;

	// first setup a state and scramble it
	Anything state;
	state["first"] = "test";
	state["second"] = "another test";
	String scrambledState;

	// scramble the state
	SecurityModule::ScrambleState(scrambledState, state);

	query["scrambledState"] = scrambledState;
	filterTags.Append("scrambledState");

	Anything dummy;
	Context ctx(dummy, query, 0, 0, 0, 0);
	// unscramble with filterTags
	querySz = query.GetSize();
	t_assert(urlFilter.UnscrambleState(query, filterTags, ctx));
	t_assertm(!query.IsDefined("scrambledState"), "expected scrambledState to be removed");
	t_assertm(query.IsDefined("first"), "expected first to be there");
	t_assertm(query.IsDefined("second"), "expected second to be there");
	t_assertm(query.GetSize() == (querySz - 1 + 2), "expected size to be one more");

	querySz = query.GetSize();
	t_assert(urlFilter.UnscrambleState(query, filterTags, ctx));
	t_assertm(query.GetSize() == querySz, "expected size to be equal");
}

void URLFilterTest::HandleCookieTest()
{
	StartTrace(URLFilterTest.HandleCookieTest);

	Anything env;
	Anything query;
	Anything cookieState;
	Anything cookieSpec;
	URLFilter urlFilter("Test");
	long querySz;
	Context ctx(env, query, 0, 0, 0, 0);

	// setting up the data
	cookieState["first"] = "test";
	cookieState["second"] = "another test";
	cookieState["sixth"] = "lookup test";
	env["WDCookies"] = cookieState;
	env["lookupTest"] = "sixth";

	query["first"] = "base";
	query["third"] = "derived";

	cookieSpec.Append("first");
	cookieSpec.Append("second");
	Anything lookupTest;
	lookupTest["Lookup"] = "sixth";
	cookieSpec.Append(lookupTest);

	// use urlfilter to manipulate the query and copy the cookie state
	querySz = query.GetSize();
	t_assert(urlFilter.HandleCookie(query, env, cookieSpec, ctx));
	t_assertm(query.IsDefined("first"), "expected first to be there");
	t_assertm(query.IsDefined("second"), "expected second to be there");
	t_assertm(query.IsDefined("third"), "expected third to be there");
	t_assertm(query.IsDefined("sixth"), "expected sixth to be there");
	assertEqual("base", query["first"].AsCharPtr(""));
	assertEqual("another test", query["second"].AsCharPtr(""));
	assertEqual("derived", query["third"].AsCharPtr(""));
	t_assert(query.GetSize() == querySz + 2);

	// use urlfilter to manipulate the query and copy the cookie state
	// no cookies defined should fail
	querySz = query.GetSize();
	t_assert(!urlFilter.HandleCookie(query, cookieState, cookieSpec, ctx));
	t_assert(query.GetSize() == querySz);

	// use urlfilter to manipulate the query and copy the cookie state
	// filter specifies elements that are not there nothing should happen
	cookieSpec.Append("fifth");
	querySz = query.GetSize();
	t_assert(urlFilter.HandleCookie(query, env, cookieSpec, ctx));
	t_assert(query.GetSize() == querySz);

}

void URLFilterTest::HandleMultipleCookiesTest()
{
	StartTrace(URLFilterTest.HandleMultipleCookiesTest);

	{
		Anything env;
		Anything query;
		Anything cookieState;
		Anything cookieSpec;
		URLFilter urlFilter("Test");
		Context ctx(env, query, 0, 0, 0, 0);

		// setting up the data
		String slotName1 = "FDState";
		cookieState[slotName1][0L] = "first";
		cookieState[slotName1][1L] = "second";
		cookieState[slotName1][2L] = "third";
		env["WDCookies"] = cookieState;

		cookieSpec.Append(slotName1);
		t_assert(urlFilter.HandleCookie(query, env, cookieSpec, ctx) == false);
		// more then one cookie for a given name, blank out
		assertEqual("", query[slotName1].AsString());
		TraceAny(env, "environment");
		assertEqual(3, env["NrOfCookies"][slotName1].AsLong(0));
	}
	{
		Anything env;
		Anything query;
		Anything cookieState;
		Anything cookieSpec;
		URLFilter urlFilter("Test");
		Context ctx(env, query, 0, 0, 0, 0);

		// setting up the data
		String slotName1 = "FDState";
		String slotName2 = "Hank";
		cookieState[slotName1] = "first";
		cookieState[slotName2] = "second";
		env["WDCookies"] = cookieState;

		cookieSpec.Append(slotName1);
		cookieSpec.Append(slotName2);
		t_assert(urlFilter.HandleCookie(query, env, cookieSpec, ctx) == true);
		assertEqual("first", query[slotName1].AsString());
		assertEqual("second", query[slotName2].AsString());
		TraceAny(env, "environment");
		TraceAny(query, "query");
		assertEqual(1, env["NrOfCookies"][slotName1].AsLong(0));
		assertEqual(1, env["NrOfCookies"][slotName2].AsLong(0));
	}
	{
		Anything env;
		Anything query;
		Anything cookieState;
		Anything cookieSpec;
		URLFilter urlFilter("Test");
		Context ctx(env, query, 0, 0, 0, 0);

		// setting up the data
		String slotName1 = "FDState";
		String slotName2 = "NotThere";
		cookieState[slotName1] = "first";
		env["WDCookies"] = cookieState;

		// Look for a cookie which is not there
		cookieSpec.Append(slotName2);
		t_assert(urlFilter.HandleCookie(query, env, cookieSpec, ctx) == true);
		TraceAny(env, "environment");
		TraceAny(query, "query");
		assertEqual(0, env["NrOfCookies"][slotName2].AsLong(3));
	}
	{
		Anything env;
		Anything query;
		Anything cookieState;
		Anything cookieSpec;
		URLFilter urlFilter("Test");
		Context ctx(env, query, 0, 0, 0, 0);

		// setting up the data
		String slotName1 = "FDState";
		String slotName2 = "Array";
		String slotName3 = "Hank";
		cookieState[slotName1] = "first";
		cookieState[slotName2][0] = "second_first";
		cookieState[slotName2][1] = "second_second";
		cookieState[slotName3] = "third";
		env["WDCookies"] = cookieState;

		cookieSpec.Append(slotName1);
		cookieSpec.Append(slotName2);
		cookieSpec.Append(slotName3);
		// HandleCookie extract all cookies, it does not stop if it finds more then one
		// cookies for a given name. Thus the valid cookies are handled as usual, only the
		// returncode is set to false.
		t_assert(urlFilter.HandleCookie(query, env, cookieSpec, ctx) == false);
		assertEqual("first", query[slotName1].AsString());
		// more then one cookie for a given name, blank out
		assertEqual("", query[slotName2].AsString());
		assertEqual("third", query[slotName3].AsString());
		TraceAny(env, "environment");
		TraceAny(query, "query");
		assertEqual(1, env["NrOfCookies"][slotName1].AsLong(0));
		assertEqual(2, env["NrOfCookies"][slotName2].AsLong(0));
		assertEqual(1, env["NrOfCookies"][slotName3].AsLong(0));
	}
}

void URLFilterTest::HandleQueryTest()
{
	StartTrace(URLFilterTest.HandleQueryTest);

	Anything query;
	Anything filterTags;
	URLFilter urlFilter("Test");
	long querySz;

	// first setup a state and scramble it
	Anything state;
	state["first"] = "test";
	state["second"] = "another test";
	String scrambledState;

	// scramble the state
	SecurityModule::ScrambleState(scrambledState, state);

	query["scrambledState"] = scrambledState;

	query["first"] = "base";
	query["third"] = "derived";
	query["fourth"] = "s4";
	query["fifth"] = "s5";
	Anything dummy;
	Context ctx(dummy, query, 0, 0, 0, 0);

	filterTags["Tags2Unscramble"].Append("scrambledState");
	filterTags["Tags2Suppress"].Append("fourth");
	filterTags["Tags2Suppress"].Append("fifth");

	// test query handling
	querySz = query.GetSize();
	t_assert(urlFilter.HandleQuery(query, filterTags, ctx));
	t_assertm(query.IsDefined("first"), "expected first to be there");
	t_assertm(query.IsDefined("second"), "expected second to be there");
	t_assertm(query.IsDefined("third"), "expected third to be there");
	t_assertm(!query.IsDefined("fourth"), "expected fourth not to be there");
	t_assertm(!query.IsDefined("fifth"), "expected fifth not to be there");
	assertEqual("base", query["first"].AsCharPtr(""));
	assertEqual("another test", query["second"].AsCharPtr(""));
	assertEqual("derived", query["third"].AsCharPtr(""));
	t_assert(query.GetSize() == querySz + 1 - 3);
}

Test *URLFilterTest::suite ()
// collect all test cases for the RegistryStream
{
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(URLFilterTest, DoFilterStateTest));
	testSuite->addTest (NEW_CASE(URLFilterTest, DoUnscrambleStateTest));
	testSuite->addTest (NEW_CASE(URLFilterTest, FilterStateTest));
	testSuite->addTest (NEW_CASE(URLFilterTest, UnscrambleStateTest));
	testSuite->addTest (NEW_CASE(URLFilterTest, HandleCookieTest));
	testSuite->addTest (NEW_CASE(URLFilterTest, HandleMultipleCookiesTest));
	testSuite->addTest (NEW_CASE(URLFilterTest, HandleQueryTest));

	return testSuite;
} // suite
