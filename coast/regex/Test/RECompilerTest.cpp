/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "TestSuite.h"
#include "RECompiler.h"
#include "RECompilerTest.h"
#include "Anything.h"
#include "StringStream.h"
#include "Dbg.h"
#include "RE.h"
#include "REBitSet.h"

//---- RECompilerTest ----------------------------------------------------------------
RECompilerTest::RECompilerTest(TString tstrName) : TestCaseType(tstrName)
{
	StartTrace(RECompilerTest.Ctor);
}

RECompilerTest::~RECompilerTest()
{
	StartTrace(RECompilerTest.Dtor);
}

struct a_test {
	long expected;
	const char *pattern;
	const /*unsigned*/ char *data;
};

struct a_test compilationTestCases[] = {
#include "CompilationTestCases.h"
	{ -1, 0, 0}
};

void RECompilerTest::RunACompilationTest (long id, struct a_test &t)
{
	StartTrace(RECompilerTest.RunACompilationTest);
	t_assert(t.pattern != NULL);
	if (!t.pattern) {
		return;
	}

	Trace("CompilationTestCases.h:" << id << "expecting " << t.expected
		  << " with pattern >" << t.pattern << "< and data >"
		  << t.data << "<\n");
	RECompiler rc;
	Anything p = rc.compile(t.pattern);
	if (t.expected >= 2) {
		t_assertm(!p.IsDefined("program"), TString("expected RE compilation to fail at\nCompilationTestCases.h:") << id << " of >" << t.pattern << "<");
		return;
	} else {
		t_assertm(p.IsDefined("program"), TString("expected successful compilation at\nCompilationTestCases.h:") << id << " of >" << t.pattern << "<");
	}
	if (!p.IsDefined("program")) {
		return;
	}
	TraceAny(p, "program");
	t_assert(t.data != NULL);
	String suspect(t.data);

	RE re(p);
	t_assert(re.IsValid());
	assertEqualm((bool)(t.expected == 0), re.ContainedIn(suspect),
				 TString("failed match expectation at\nCompilationTestCases.h:") << id << " with pattern >" << t.pattern << "< and data >" << t.data << "<");
}

void RECompilerTest::TestAllCompilationTests()
{
	StartTrace(RECompilerTest.TestAllCompilationTests);
	for (unsigned i = 1; i < sizeof(compilationTestCases) / sizeof(compilationTestCases[0]); i++) {
		RunACompilationTest(i, compilationTestCases[i-1]);
	}
}

void RECompilerTest::CheckInstr(RECompiler &rc, long index, long opcode, long opdata, long opnext = 0L)
{
	StartTrace(RECompilerTest.CheckInstr);
	t_assert(index < rc.GetInstruction().GetSize());
	Anything a = rc.GetInstruction()[index];
	assertEqual(3L, a.GetSize());
	assertEqualm(opcode, a[RE::offsetOpcode].AsLong(-1), TString("check opcode at ") << index << " when size is " << rc.GetInstruction().GetSize());
	assertEqual(opdata, a[RE::offsetOpdata].AsLong(-1));
	assertEqual(opnext, a[RE::offsetNext].AsLong(-1));

}

void RECompilerTest::TestAppendNode()
{
	StartTrace(RECompilerTest.TestAppendNode);

	RECompiler rc;

	assertEqual(0L, rc.GetInstruction().GetSize());
	rc.AppendNode(1, 2);
	long len = rc.GetInstruction().GetSize();
	assertEqual(1L, len);
	CheckInstr(rc, len - 1, 1L, 2L);

	rc.AppendNode(3, 4);
	len = rc.GetInstruction().GetSize();
	assertEqual(2L, len);
	CheckInstr(rc, 0L, 1L, 2L);
	CheckInstr(rc, len - 1, 3L, 4L);

}

void RECompilerTest::TestInsertNode()
{
	StartTrace(RECompilerTest.TestInsertNode);

	RECompiler rc;

	rc.InsertNode(1, 2, 0);
	long len = rc.GetInstruction().GetSize();
	assertEqual(1L, len);
	CheckInstr(rc, len - 1, 1L, 2L);

	rc.InsertNode(3, 4, 0);
	len = rc.GetInstruction().GetSize();
	assertEqual(2L, len);
	CheckInstr(rc, 0L, 3L, 4L);
	CheckInstr(rc, len - 1, 1L, 2L);

	rc.InsertNode(5, 6, 2);
	len = rc.GetInstruction().GetSize();
	assertEqual(3L, len);
	CheckInstr(rc, 0L, 3L, 4L);
	CheckInstr(rc, 1L, 1L, 2L);
	CheckInstr(rc, len - 1, 5L, 6L);

	rc.InsertNode(7, 8, 1);
	len = rc.GetInstruction().GetSize();
	assertEqual(4L, len);

	CheckInstr(rc, 0L, 3L, 4L);
	CheckInstr(rc, 1L, 7L, 8L);
	CheckInstr(rc, 2L, 1L, 2L);
	CheckInstr(rc, len - 1, 5L, 6L);
}

void RECompilerTest::TestSetNextOfEnd()
{
	StartTrace(RECompilerTest.TestSetNextOfEnd);

	RECompiler rc;
}

void RECompilerTest::SimpleCompile()
{
	StartTrace(RECompilerTest.SimpleCompile);

	RECompiler rc;
	Anything p = rc.compile("hallo");
	TraceAny(p, "program ");
	RE r(p);
	t_assert(r.ContainedIn("hallo peter"));
}

void RECompilerTest::SuspectedCompile()
{
	StartTrace(RECompilerTest.SuspectedCompile);

	RECompiler rc;
	Anything p = rc.compile("(^)*");
	TraceAny(p, "program ");

	p = rc.compile("^a$");

	RE r(p);
	t_assert(r.ContainedIn("a"));
	t_assert(!r.ContainedIn("aa"));

	p = rc.compile("^abc$");
	r.setProgram(p);
	t_assert(r.ContainedIn("abc"));
	t_assert(!r.ContainedIn("abc\ndef"));
	t_assert(!r.ContainedIn("xy\nabc\ndef"));
	t_assert(!r.ContainedIn("abccccc"));
	r.SetMatchFlags(RE::MATCH_MULTILINE);
	t_assert(r.ContainedIn("abc\ndef"));
	t_assert(r.ContainedIn("xy\nabc\ndef"));
	t_assert(!r.ContainedIn("abccccc"));

	p = rc.compile("^((ab)*)c\\1$");
	r.setProgram(p);
	t_assert(r.ContainedIn("ababcabab"));
	t_assert(!r.ContainedIn("acb"));
	t_assert(r.ContainedIn("abcab"));

}

void RECompilerTest::TestTerminal()
{
	StartTrace(RECompilerTest.TestTerminal);

	RECompiler rc;
	Anything p = rc.compile(".");
	TraceAny(p, "program ");
	assertEqual('.', p["program"][1L][RE::offsetOpcode].AsLong(-1));
	p = rc.compile("^.$");
	TraceAny(p, "program ");
	assertEqual('^', p["program"][1L][RE::offsetOpcode].AsLong(-1));
	assertEqual('.', p["program"][2L][RE::offsetOpcode].AsLong(-1));
	assertEqual('$', p["program"][3L][RE::offsetOpcode].AsLong(-1));

// do not match yet...
}

void RECompilerTest::TestRepeatTerminal()
{
	StartTrace(RECompilerTest.TestRepeatTerminal);

	RECompiler rc;
	long term = rc.AppendNode('A', 'a');
	assertEqual(0L, term);
	Anything terminal = rc.CopyProgram(term, 1);
	long last = rc.RepeatTerminal(terminal, term, 3);
	long len = rc.GetInstruction().GetSize();
	assertEqual(4L, len);
	assertEqual(3L, last);

	CheckInstr(rc, 0, 'A', 'a', 1);
	CheckInstr(rc, 1, 'A', 'a', 1);
	CheckInstr(rc, 2, 'A', 'a', 1);
	CheckInstr(rc, 3, 'A', 'a', 0);

}

void RECompilerTest::TestGenerateQuestion()
{
	StartTrace(RECompilerTest.TestGenerateQuestion);

	RECompiler rc;
	long term = rc.AppendNode('A', 'a');
	assertEqual(0L, term);
	rc.GenerateQuestion(term);
	long len = rc.GetInstruction().GetSize();
	assertEqual(4L, len);
	CheckInstr(rc, 0, RE::OP_BRANCH, 0, 2);
	CheckInstr(rc, 1, 'A', 'a', 2);
	CheckInstr(rc, 2, RE::OP_BRANCH, 0, 1);
	CheckInstr(rc, 3, RE::OP_NOTHING, 0, 0);
}

void RECompilerTest::TestClosure()
{
	StartTrace(RECompilerTest.TestClosure);

	RECompiler rc;
	Anything p = rc.compile("a*");
	TraceAny(p, "program ");
	t_assert(p.IsDefined("program"));
	p = rc.compile("b+");
	t_assert(p.IsDefined("program"));
	p = rc.compile("c?");
	t_assert(p.IsDefined("program"));
	p = rc.compile("cd{3,5}e");
	t_assert(p.IsDefined("program"));
	RE r(p);

	t_assert(!r.ContainedIn("abcef"));
	t_assert(!r.ContainedIn("abcdef"));
	t_assert(!r.ContainedIn("abcddef"));

	t_assert(r.ContainedIn("abcdddef"));
	t_assert(r.ContainedIn("abcddddef"));
	t_assert(r.ContainedIn("abcdddddef"));
	t_assert(!r.ContainedIn("abcdddddf"));
	t_assert(!r.ContainedIn("abcddddddef"));

	p = rc.compile("e{2,}");
	t_assert(p.IsDefined("program"));
	p = rc.compile("f{4}");
	t_assert(p.IsDefined("program"));

	p = rc.compile("a+?"); // non-greedy closure
	TraceAny(p, "program ");
	t_assert(p.IsDefined("program"));

	// test boundary cases: 0 doesn't seem to work (yet)
	p = rc.compile("ab{0,1}c");
	r.setProgram(p);
	t_assert(r.ContainedIn("ac"));
	t_assert(r.ContainedIn("abc"));
	t_assert(!r.ContainedIn("abbc"));
	p = rc.compile("ab{1}c");
	r.setProgram(p);
	t_assert(!r.ContainedIn("ac"));
	t_assert(r.ContainedIn("abc"));
	t_assert(!r.ContainedIn("abbc"));
}

void RECompilerTest::TestBranch()
{
	StartTrace(RECompilerTest.TestBranch);

	RECompiler rc;
	Anything p = rc.compile("a?b");
	TraceAny(p, "program ");
	assertEqual(RE::OP_BRANCH, p["program"][1L][RE::offsetOpcode].AsLong(-1));

// do not match yet...
}

void RECompilerTest::TestExpr()
{
	StartTrace(RECompilerTest.TestBranch);

	RECompiler rc;
	Anything p = rc.compile("(a|b)b");
	TraceAny(p, "program ");
	assertEqual(RE::OP_OPEN, p["program"][1L][RE::offsetOpcode].AsLong(-1));

// do not match yet...
}

void RECompilerTest::TestEscape()
{
	StartTrace(RECompilerTest.TestBranch);

	RECompiler rc;
	Anything p = rc.compile("\\040");
	TraceAny(p, "program ");
	t_assert(p["program"][1L][RE::offsetOpdata].GetType() == AnyCharPtrType);
	assertEqual(" ", p["program"][1L][RE::offsetOpdata].AsCharPtr());
	p = rc.compile("\\x21");
	t_assert(p["program"][1L][RE::offsetOpdata].GetType() == AnyCharPtrType);
	assertEqual("!", p["program"][1L][RE::offsetOpdata].AsCharPtr());
	p = rc.compile("\\t");
	t_assert(p["program"][1L][RE::offsetOpdata].GetType() == AnyCharPtrType);
	assertEqual("\t", p["program"][1L][RE::offsetOpdata].AsCharPtr());
	p = rc.compile("\\s");
	assertEqual(RE::OP_ANYOF, p["program"][1L][RE::offsetOpcode].AsLong(-1));

	p = rc.compile("(a+)\\1");
	t_assert(p.GetSize() > 0);
	long proglen = p["program"].GetSize() - 1;
	assertEqual(RE::OP_BACKREF, p["program"][proglen-1][RE::offsetOpcode].AsLong(-1));

// do not match yet...
}

void RECompilerTest::TestAtom()
{
	StartTrace(RECompilerTest.TestAtom);

	RECompiler rc;
	Anything p = rc.compile("a");
	TraceAny(p, "program ");
	t_assert(p["program"][1L][RE::offsetOpdata].GetType() == AnyCharPtrType);
	assertEqual("a", p["program"][1L][RE::offsetOpdata].AsCharPtr());
	p = rc.compile("abc*");
	TraceAny(p, "program ");
	t_assert(p["program"][1L][RE::offsetOpdata].GetType() == AnyCharPtrType);
	assertEqual("ab", p["program"][1L][RE::offsetOpdata].AsCharPtr());

// do not match yet...
}

void RECompilerTest::TestCharClass()
{
	StartTrace(RECompilerTest.TestCharClass);

	RECompiler rc;
	Anything p = rc.compile("[a]");
	TraceAny(p, "program ");
	t_assert(p["program"][1L][RE::offsetOpdata].GetType() == AnyVoidBufType);
	REBitSet *s = (REBitSet *)(p["program"][1L][RE::offsetOpdata].AsCharPtr(0));
	t_assert(s != NULL);
	if (s) {
		t_assert(s->IsMember('a'));
		t_assert(!s->IsMember('b'));
	}
	Anything cp = rc.compile("[^a]");
	t_assert(cp["program"][1L][RE::offsetOpdata].GetType() == AnyVoidBufType);

	REBitSet *scp = (REBitSet *)(cp["program"][1L][RE::offsetOpdata].AsCharPtr(0));
	t_assert(scp != NULL);
	if (scp) {
		t_assert(!scp->IsMember('a'));
		t_assert(scp->IsMember('b'));
		if (s) {
			REBitSet bset(*s);
			bset &= *scp;
			t_assert(REBitSet().IsEqual(bset));
			bset = *s;
			bset |= *scp;
			t_assert(REBitSet(true).IsEqual(bset));
		}
	}

	p = rc.compile("[abc]");
	t_assert(p["program"][1L][RE::offsetOpdata].GetType() == AnyVoidBufType);
	s = (REBitSet *)(p["program"][1L][RE::offsetOpdata].AsCharPtr(0));
	t_assert(s != NULL);
	if (s) {
		t_assert(s->IsMember('a'));
		t_assert(s->IsMember('b'));
		t_assert(s->IsMember('c'));
		t_assert(!s->IsMember('D'));
	}
	Anything q = rc.compile("[a-c]");
	t_assert(q["program"][1L][RE::offsetOpdata].GetType() == AnyVoidBufType);
	REBitSet *qs = (REBitSet *)(q["program"][1L][RE::offsetOpdata].AsCharPtr(0));
	t_assert(qs != NULL);
	if (qs && s) {
		t_assert(s->IsEqual(*qs));
		t_assert(!qs->IsMember('-'));
	}
	p = rc.compile("[a-]");
	assertCompare( 0L, less, p.GetSize() );
	t_assert(p["program"][1L][RE::offsetOpdata].GetType() == AnyVoidBufType);
	s = (REBitSet *)(p["program"][1L][RE::offsetOpdata].AsCharPtr(0));
	t_assert(s != NULL);
	if (s) {
		t_assert(s->IsMember('a'));
		t_assert(s->IsMember('-'));
		t_assert(!s->IsMember('b'));
		t_assert(!s->IsMember('D'));

	}

	p = rc.compile("[\\s\\w]");
	assertCompare( 0L, less, p.GetSize() );
	t_assert(p["program"][1L][RE::offsetOpdata].GetType() == AnyVoidBufType);
	s = (REBitSet *)(p["program"][1L][RE::offsetOpdata].AsCharPtr(0));
	t_assert(s != NULL);
	if (s) {
		t_assert(s->IsMember('a'));
		t_assert(!s->IsMember('-'));
		t_assert(s->IsMember(' '));
		t_assert(!s->IsMember('['));
	}
}

void RECompilerTest::TestBackRefMatch()
{
	StartTrace(RECompilerTest.TestBackRefMatch);

	RECompiler rc;
	Anything p = rc.compile("(a)*-\\1b");
	TraceAny(p, "program ");
	t_assert(p["program"].GetSize() > 0);
	RE r(p);
	t_assert(!r.ContainedIn("aaa-b"));

	p = rc.compile("((a|b{10,11})|(b))*-\\2");
	r.setProgram(p);
	t_assert(r.ContainedIn("aaab-a"));
	t_assert(r.ContainedIn("bbbbbbbbbbb-bbbbbbbbbbb"));
	t_assert(r.ContainedIn("abbbbbbbbbb-a"));
// do not match yet...
}

// builds up a suite of testcases, add a line for each testmethod
Test *RECompilerTest::suite ()
{
	StartTrace(RECompilerTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, RECompilerTest, TestAppendNode);
	ADD_CASE(testSuite, RECompilerTest, TestInsertNode);
	ADD_CASE(testSuite, RECompilerTest, TestSetNextOfEnd);
	ADD_CASE(testSuite, RECompilerTest, TestGenerateQuestion);
	ADD_CASE(testSuite, RECompilerTest, TestRepeatTerminal);
	ADD_CASE(testSuite, RECompilerTest, TestCharClass);
	ADD_CASE(testSuite, RECompilerTest, TestAtom);
	ADD_CASE(testSuite, RECompilerTest, TestTerminal);
	ADD_CASE(testSuite, RECompilerTest, TestClosure);
	ADD_CASE(testSuite, RECompilerTest, TestBranch);
	ADD_CASE(testSuite, RECompilerTest, TestExpr);
	ADD_CASE(testSuite, RECompilerTest, TestEscape);
	ADD_CASE(testSuite, RECompilerTest, SimpleCompile);
	ADD_CASE(testSuite, RECompilerTest, SuspectedCompile);
	ADD_CASE(testSuite, RECompilerTest, TestBackRefMatch);
	ADD_CASE(testSuite, RECompilerTest, TestAllCompilationTests);

	return testSuite;

}
