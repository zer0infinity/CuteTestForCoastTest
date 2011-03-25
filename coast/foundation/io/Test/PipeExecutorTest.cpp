/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "PipeExecutorTest.h"
#include "TestSuite.h"
#include "PipeExecutor.h"

using namespace Coast;

//---- PipeExecutorTest ----------------------------------------------------------------
PipeExecutorTest::PipeExecutorTest(TString className)
	: TestCaseType(className)
{
	StartTrace(PipeExecutorTest.Ctor);
}

PipeExecutorTest::~PipeExecutorTest()
{
	StartTrace(PipeExecutorTest.Dtor);
}

void PipeExecutorTest::EchoEnvTest()
{
	StartTrace(PipeExecutorTest.EchoEnvTest);
	Anything env;
	env["HALLO"] = "Peter";
	String fullname;
#if defined(WIN32)
	System::FindFile(fullname, "env.exe");
#else
	fullname << "/usr/bin/env";
#endif
	Trace("Path:[" << fullname << "]");
	PipeExecutor Execute(fullname, env);
	t_assert(Execute.Start());
	std::ostream *os = Execute.GetStream();
	t_assertm(os != NULL, "Execute env failed");
	std::istream *is = Execute.GetStream();
	t_assertm(is != NULL, "Execute env failed");

	if (is && os) {
		t_assertm(!!(*os), "Execute env failed");
		t_assertm(!!(*is), "Execute env failed");
		Execute.ShutDownWriting();

		std::istreambuf_iterator<char> end;
		String s1(std::istreambuf_iterator<char>(*is), end);

		Trace("ENV: " << s1);
		t_assertm((s1.Contains("HALLO=Peter") != -1), "exported env variable not contained");
	}
}

void PipeExecutorTest::FailExecTest()
{
	StartTrace(PipeExecutorTest.FailExecTest);
	Anything env;
	PipeExecutor Execute("/wuggiwaggi", env);
	bool res = Execute.Start();
	t_assertm(!res, "Execute.Start(/wuggiwaggi,env) succeeded");
	std::ostream *os = Execute.GetStream();
	t_assertm(0 == os, "oops Execute call succeeded");
	std::istream *is = Execute.GetStream();
	t_assertm(0 == is, "oops Execute call succeeded");
	if (res && is && os) {
		// show what is wrong, we didn't expect to...
		t_assertm(!!(*os), "Execute call failed");
		t_assertm(!!(*is), "Execute call failed");
		Execute.ShutDownWriting();
	}
}

void PipeExecutorTest::CatGugusErrTest()
{
	StartTrace(PipeExecutorTest.CatGugusErrTest);
	Anything env;
	String fullname;
#if defined(WIN32)
	System::FindFile(fullname, "cat.exe");
#else
	fullname << "/bin/cat";
#endif
	Trace("Path:[" << fullname << "]");
	fullname << " ./gugus";
	PipeExecutor Execute(fullname, env, ".", 3000L, true); // file doesn't exist
	t_assert(Execute.Start());
	std::istream *err = Execute.GetStderr();
	t_assertm(err != NULL, "Execute cat stderr failed");

	if (err) {
		t_assertm(!!(*err), "Execute cat failed");
		// may be we need to close it here...
		String s1;
		(*err) >> s1; // we expect an error messags
		t_assert(s1.Length() > 0);
	}
}

void PipeExecutorTest::CatWorkingDirTest()
{
	StartTrace(PipeExecutorTest.CatWorkingDirTest);
	Anything env;
	String wd = GetTestCaseConfig()["WorkingDir"].AsString();
	String filename(wd);
	const char *fileName = "PipeExecutorTest.txt";
	String msg( "Hello_world!" );
	filename << System::Sep() << fileName;
	std::ostream *os = System::OpenOStream(filename, 0);
	t_assertm(os && !!(*os), "opening test file for writing failed");
	if (os) {
		(*os) << msg << std::flush;
		delete os;
	}
	String cmd;
#if defined(WIN32)
	System::FindFile(cmd, "cat.exe");
#else
	cmd << "/bin/cat";
#endif
	Trace("Path:[" << cmd << "]");
	cmd << " " << fileName;
	PipeExecutor Execute(cmd, env, wd);
	t_assert(Execute.Start());
	std::istream *is = Execute.GetStream();
	t_assertm(is != NULL, "Execute cat failed");

	if (is ) {
		t_assertm(!!(*is), "Execute cat failed");
		Execute.ShutDownWriting();
		String s1;
		// may be we need to close it here...
		(*is) >> s1;
		assertEqual(msg, s1);
	}
	assertEqual(0, Execute.TerminateChild()); // everything is over
}
void PipeExecutorTest::EchoCatTest()
{
	StartTrace(PipeExecutorTest.EchoCatTest);
	Anything env;
	String fullname;
#if defined(WIN32)
	System::FindFile(fullname, "cat.exe");
#else
	fullname << "/bin/cat";
#endif
	Trace("Path:[" << fullname << "]");
	PipeExecutor Execute(fullname, env);
	t_assert(Execute.Start());
	std::ostream *os = Execute.GetStream();
	t_assertm(os != NULL, "Execute cat failed");
	std::istream *is = Execute.GetStream();
	t_assertm(is != NULL, "Execute cat failed");
	if (is && os) {
		t_assertm(!!(*os), "Execute cat failed");
		t_assertm(!!(*is), "Execute cat failed");
		String s0("hallo");
		(*os) << s0 << std::endl;
		t_assert(Execute.ShutDownWriting());
		String s1;
		// may be we need to close it here...
		(*is) >> s1;
		assertEqual(s0, s1);
	}
	assertEqual(0, Execute.TerminateChild()); // everything is over
}

void PipeExecutorTest::KillTest()
{
	StartTrace(PipeExecutorTest.KillTest);
	Anything env;
	String fullname;
#if defined(WIN32)
	System::FindFile(fullname, "cat.exe");
#else
	fullname << "/bin/cat";
#endif
	Trace("Path:[" << fullname << "]");
	PipeExecutor Execute(fullname, env, ".", 10000L);
	t_assert(Execute.Start());
	std::ostream *os = Execute.GetStream();
	t_assertm(os != NULL, "Execute cat failed");
	std::istream *is = Execute.GetStream();
	t_assertm(is != NULL, "Execute cat failed");

	if (is && os) {
		t_assertm(!!(*os), "writing to cat failed");
		String s0("hallo");
		(*os) << s0 << std::endl << std::flush;

		// kill the bastard --- FIXME this code runs well in the debugger
		// but fails in real life, either its timing or the way
		// the debugger routes signals
		int res = Execute.TerminateChild();
		t_assertm(res >= 0, "killing child cat failed somehow");

		t_assertm(!!(*is), "do not know death of cat");
		String s1;
		(*is) >> s1;
		// there is no guarantee that any stream is closed
		// after killing a process with -SIGKILL
		// so this test is HIGHLY timing and signal dependent
		// e.g. worthless
//		t_assertm(!(*is),"oops, reading from dead cat possible");
	}
}

void PipeExecutorTest::ParseParamTest()
{
	StartTrace(PipeExecutorTest.ParseParamTest);
	PipeExecutor Execute;
	Anything p;
	Execute.ParseParam("/bin/cat /etc/passwd", p);
	assertEqual("/bin/cat", p[0L].AsCharPtr());
	assertEqual("/etc/passwd", p[1L].AsCharPtr());
}

void PipeExecutorTest::PrepareParamTest()
{
	StartTrace(PipeExecutorTest.PrepareParamTest);
	PipeExecutor Execute;
	Anything pm;
	Execute.ParseParam("/bin/cat /etc/passwd", pm);
	PipeExecutor::CgiParam cgiParams(pm, Coast::Storage::Current());
	char **p = cgiParams.GetParams();
	assertCharPtrEqual("/bin/cat", p[0]);
	assertCharPtrEqual("/etc/passwd", p[1]);
	assertEqual(0, p[2]);
}

void PipeExecutorTest::PrepareEnvTest()
{
	StartTrace(PipeExecutorTest.PrepareEnvTest);
	PipeExecutor Execute;
	Anything pm;
	pm["Hallo"] = "Peter";
	pm["Servus"] = "Marcel";
	PipeExecutor::CgiEnv cgiEnv(pm, Coast::Storage::Current());
#if defined(WIN32)
	String expected("Hallo=Peter");
	expected.Append('\0').Append("Servus=Marcel").Append('\0').Append('\0');
	int length = 0;
	void *p = cgiEnv.GetEnv(length);
	assertEqual(expected.Length(), length);
	t_assertm(expected == String(p, length), "blocks not equal");
#else
	char **p = cgiEnv.GetEnv();
	assertCharPtrEqual("Hallo=Peter", p[0]);
	assertCharPtrEqual("Servus=Marcel", p[1]);
	assertEqual(0, p[2]);
#endif
}

void PipeExecutorTest::DummyKillTest()
{
	StartTrace(PipeExecutorTest.DummyKillTest);
	PipeExecutor Execute;
	assertEqual(-1, Execute.TerminateChild());
}

void PipeExecutorTest::ShellInvocationTest()
{
	StartTrace(PipeExecutorTest.ShellInvocationTest);
	ROAnything roaConfig;
	AnyExtensions::Iterator<ROAnything, ROAnything, TString> aEntryIterator(GetTestCaseConfig());
	while ( aEntryIterator.Next(roaConfig) ) {
		TString strCase;
		if ( !aEntryIterator.SlotName(strCase) ) {
			strCase << "idx:" << aEntryIterator.Index();
		}
		ROAnything roaExpected = roaConfig["Expected"], roaParams = roaConfig["Params"];
		Anything env;
		if ( roaParams.IsDefined("Env") ) {
			if ( roaParams["Env"].IsNull() ) {
				System::GetProcessEnvironment(env);
			} else {
				env = roaParams["Env"].DeepClone();
			}
		}
		TraceAny(env, "environment to use:");
		String fullname;
#if defined(WIN32)
		System::FindFile(fullname, roaParams["ExecutableWin"].AsString());
#else
		fullname << roaParams["Executable"].AsString();
#endif
		Trace("Executable [" << fullname << "]");
		bool bUseStderr = roaParams["UseStderr"].AsBool(false);
		PipeExecutor Execute(fullname, env, roaParams["Path"].AsString("."), roaParams["Timeout"].AsLong(1000L), bUseStderr);
		TString tstrMsg("Executing ");
		tstrMsg << fullname << " failed";
		if ( t_assertm( Execute.Start() == roaExpected["ExecOk"].AsBool(false), tstrMsg) && roaExpected["ExecOk"].AsBool(false) ) {
			std::ostream *os = Execute.GetStream();
			t_assertm(os != NULL, "could not get stdout to write to!");
			std::istream *is = Execute.GetStream(), *isErr = NULL;
			t_assertm(is != NULL, "could not get stdin to read from!");
			if ( bUseStderr ) {
				isErr = Execute.GetStderr();
				t_assertm(isErr != NULL, "could not get stderr to read from!");
			}
			if (is && os) {
				t_assertm(!!(*os), "expected stdout to be good");
				t_assertm(!!(*is), "expected stdin to be good");
				(*os) << roaParams["Command"].AsString("notdefined") << std::endl;
				t_assertm(Execute.ShutDownWriting(), "shutdown writing side failed");
				OStringStream aShellOutput, aErrOutput;
				long lRecv = 0, lToRecv = 2048;
				while ( NSStringStream::PlainCopyStream2Stream(is, aShellOutput, lRecv, lToRecv) && lRecv == lToRecv ) ;
				t_assert(lRecv > 0);
				Trace("Stdout [" << aShellOutput.str() << "]");
				assertCharPtrEqual(roaExpected["Output"].AsString(""), aShellOutput.str());
				if ( bUseStderr && isErr ) {
					while ( NSStringStream::PlainCopyStream2Stream(isErr, aErrOutput, lRecv, lToRecv) && lRecv == lToRecv ) ;
					assertEqual(0, lRecv);
					Trace("Stderr [" << aErrOutput.str() << "]");
					assertCharPtrEqual(roaExpected["Error"].AsString(""), aErrOutput.str());
				}
			}
			assertEqual(0, Execute.TerminateChild()); // everything is over
		}
	}
}

Test *PipeExecutorTest::suite ()
{
	StartTrace(PipeExecutorTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, PipeExecutorTest, ParseParamTest);
	ADD_CASE(testSuite, PipeExecutorTest, PrepareParamTest);
	ADD_CASE(testSuite, PipeExecutorTest, PrepareEnvTest);
	ADD_CASE(testSuite, PipeExecutorTest, EchoCatTest);
	ADD_CASE(testSuite, PipeExecutorTest, CatWorkingDirTest);
	ADD_CASE(testSuite, PipeExecutorTest, EchoEnvTest);
	ADD_CASE(testSuite, PipeExecutorTest, CatGugusErrTest);
	ADD_CASE(testSuite, PipeExecutorTest, FailExecTest);
	ADD_CASE(testSuite, PipeExecutorTest, DummyKillTest);
	ADD_CASE(testSuite, PipeExecutorTest, KillTest);
	ADD_CASE(testSuite, PipeExecutorTest, ShellInvocationTest);
	return testSuite;
}
