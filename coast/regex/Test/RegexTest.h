/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _RegexTest_H
#define _RegexTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"

//---- RegexTest ----------------------------------------------------------
//!TestCases description
class RegexTest : public TestCase
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	RegexTest(TString tstrName);

	//!destroys the test case
	~RegexTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	//!deletes the environment for this test
	void tearDown ();

	//!match literal
	void MatchLiteral();
	//!match '.'
	void MatchDot();
	//!match '...'
	void MatchDotDotDot();
	//!match '.*'
	void MatchDotStar();
	//!match 'a*'
	void MatchAStar();
	//!match 'a*'
	void MatchCharTest();
	//!match '(abc|abcd)(d|)' with "abcd"
	void LeftMostLongestTest();
	//! test the use of backward references, i.e. \1 \2
	void BackRefTest();
	//!test a literal against a long string
	//! data and test stolen from rx package
	void LargeLiteralTest();
	//!test a literal against a long list of strings
	//! data and test stolen from rx package
	void ShortLiteralTest();
	//!test a literal .* combined expr against a long string
	//! data and test stolen from rx package
	void LargeDotStarTest();
	//!test a single configured test
	//!uses up to 5 slots in the anything:
	//! [0L] --> 0,1,2 meaning match, compile but don't match, fail to compile
	//! [1L] --> the RE, remember Anything reader will need double backslashes
	//! [2L] --> text to match against
	//! [3L] --> optional, expected start of match
	//! [4L] --> expected length of match
	void MatchAnything(long identifier, Anything aCase);
	//!read config file and do all the tests
	void MatchConfig();
	//!test the flags
	void MatchFlagsTest();
	//!test splitting of a string
	void SplitTest();
	//!test substitution of RE with a String
	void SubstTest();
	//!test "grepping" for Anything slot values
	void GrepTest();
	//!test "grepping" for Anything slot names
	void GrepSlotNamesTest();
#if TESTWITHPOOLALLOCATOR
	//!run ShortLiteralTest twice with and without a poolallocator
	void TimingWithPoolAllocator();
	void TimeaTestWithPoolAllocator(CaseMemberPtr testtotime);
#endif
};

#endif
