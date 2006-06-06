/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _RECompilerTest_H
#define _RECompilerTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"

//---- RECompilerTest ----------------------------------------------------------
//!TestCases description
class RECompilerTest : public TestFramework::TestCase
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	RECompilerTest(TString tstrName);

	//!destroys the test case
	~RECompilerTest();

	//--- public api

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!describe this testcase
	void SimpleCompile();
	void SuspectedCompile();
	//!test various ways to define a simple character class with [...]
	void TestCharClass();
	//!test some ways to define an atom (= literal) in an RE
	void TestAtom();
	//!test terminals that are not atoms or escapes: ^.$
	void TestTerminal();
	//!test for parsing of closure operators ?+*{n,m}{n}{n,}
	void TestClosure();
	//!test for concatenation within a branch
	void TestBranch();
	//!test expressions () with optional alternatives (.|a)
	void TestExpr();
	//!test escape handling with double backslashes
	void TestEscape();

	//!test internal auxiliary methods maintaining the instruction array
	void TestAppendNode();
	void TestInsertNode();
	void TestSetNextOfEnd();
	void TestRepeatTerminal();
	void TestGenerateQuestion();

	void TestBackRefMatch();
	void TestAllCompilationTests();

protected:
	void CheckInstr(class RECompiler &rc, long index, long opcode, long opdata, long opnext);
	void RunACompilationTest (long id, struct a_test &t);
};

#endif
