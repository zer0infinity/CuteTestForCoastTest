/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#if defined(sun) || defined(WIN32) || defined(__linux__)

//--- c-library modules used ---------------------------------------------------
#include <stdlib.h>

//--- standard modules used ----------------------------------------------------
#include "StringStream.h"
#include "System.h"
#include "LDAPAPI.h"
#include "LDAPDAImpl.h"
#include "Dbg.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "LDAPMapper.h"

//--- interface include --------------------------------------------------------
#include "LDAPMapperTest.h"

//---- LDAPMapperTest ----------------------------------------------------------------
Test *LDAPMapperTest::suite ()
{
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, LDAPMapperTest, QueryFilterTest);
	ADD_CASE(testSuite, LDAPMapperTest, BasicParamTest);
	ADD_CASE(testSuite, LDAPMapperTest, DNameTest);
	ADD_CASE(testSuite, LDAPMapperTest, FieldMapperTest1);
	ADD_CASE(testSuite, LDAPMapperTest, FieldMapperTest2);
	ADD_CASE(testSuite, LDAPMapperTest, ModifyValsTest);
	ADD_CASE(testSuite, LDAPMapperTest, LDAPAddTest);

	return testSuite;
}

LDAPMapperTest::LDAPMapperTest(TString tname) : TestCaseType(tname)
{
}

LDAPMapperTest::~LDAPMapperTest()
{
}

void LDAPMapperTest::QueryFilterTest()
{
	StartTrace(LDAPMapperTest.QueryFilterTest);
	Anything inputArgs;

	Context ctx(inputArgs, inputArgs, (Server *)0, (Session *)0, (Role *)0);
	ParameterMapper querymapper("lqmTest0");
	querymapper.CheckConfig("ParameterMapper");

	String input;
	OStringStream Ios(&input);

	t_assert(querymapper.Get("Input", Ios, ctx));
	Ios << flush;

	Trace("Input: " << input);
	assertEqual("(&(objectclass=person)(uid=birrer))", input);
}

void LDAPMapperTest::FieldMapperTest1()
{
	StartTrace(LDAPMapperTest.FieldMapperTest);
	{
		Anything inputArgs;
		inputArgs["password"] = "aaaaaaaaaaa"; //SHA1  11 byte: '755c001f4ae3c8843e5a50dd6aa2fa23893dd3ad'

		Context ctx(inputArgs, inputArgs, (Server *)0, (Session *)0, (Role *)0);
		ParameterMapper fieldmapper("FieldMapperTest1_fixed");
		fieldmapper.CheckConfig("ParameterMapper");

		String input;
		OStringStream Ios(&input);

		t_assert(fieldmapper.Get("Input", Ios, ctx));
		Ios << flush;

		Trace("Input: " << input);
		assertEqual("{SHA}dVwAH0rjyIQ+WlDdaqL6I4k9060=", input);
	}
}

void LDAPMapperTest::FieldMapperTest2()
{
	// Exercises base64 only
	StartTrace(LDAPMapperTest.FieldMapperTest);
	Anything inputArgs;
	inputArgs["password"] = "welcome";

	Context ctx(inputArgs, inputArgs, (Server *)0, (Session *)0, (Role *)0);
	ParameterMapper fieldmapper("FieldMapperTest2");
	fieldmapper.CheckConfig("ParameterMapper");

	String input;
	OStringStream Ios(&input);

	t_assert(fieldmapper.Get("Input", Ios, ctx));
	Ios << flush;

	Trace("Input: " << input);
	assertEqual("d2VsY29tZQ==", input);

}

void LDAPMapperTest::BasicParamTest()
{
	StartTrace(LDAPMapperTest.BasicParamTest);
	Anything inputArgs;
	inputArgs["Base"] = "o=ifs, c=CH";

	Context ctx(inputArgs, inputArgs, (Server *)0, (Session *)0, (Role *)0);
	EagerParameterMapper ldapparammapper("BasicParams");
	ldapparammapper.CheckConfig("ParameterMapper");

	Anything ldapparams;
	t_assert(ldapparammapper.Get("Input", ldapparams, ctx));
	TraceAny(ldapparams, "ldapparams after");

	assertEqual("o=ifs, c=CH", ldapparams["Base"].AsCharPtr());
	assertEqual(100, ldapparams["SizeLimit"].AsLong());
	assertEqual(10, ldapparams["Timeout"].AsLong());
	assertEqual("uid", ldapparams["Attributes"][0L].AsCharPtr());
	assertEqual("services", ldapparams["Attributes"][1L].AsCharPtr());
}

void LDAPMapperTest::DNameTest()
{
	StartTrace(LDAPMapperTest.DNameTest);
	Anything inputArgs;
	inputArgs["userid"] = "stossel";

	Context ctx(inputArgs, inputArgs, (Server *)0, (Session *)0, (Role *)0);
	ParameterMapper dnmapper("DName");
	dnmapper.CheckConfig("ParameterMapper");

	String input;
	OStringStream Ios(&input);

	t_assert(dnmapper.Get("Input", Ios, ctx));
	Ios << flush;

	Trace("Input: " << input);
	assertEqual("uid=stossel,ou=tkfu,", input);
}

void LDAPMapperTest::ModifyValsTest()
{

	StartTrace(LDAPMapperTest.ModifyValsTest);
	{
		Anything inputArgs;
		// Wrong password string handling cuts password off at first hex 00
		inputArgs["Password"] = "aaaaaaaaaaa"; //SHA1  11 byte: '755c001f4ae3c8843e5a50dd6aa2fa23893dd3ad'

		Context ctx(inputArgs, inputArgs, (Server *)0, (Session *)0, (Role *)0);
		EagerParameterMapper modifymapper("ModifyValsFixed");
		modifymapper.CheckConfig("ParameterMapper");

		Anything vals;
		t_assert(modifymapper.Get("Modify", vals, ctx));

		TraceAny(vals, "My modify values: ");

		assertEqual(2, vals[0L]["Mode"].AsLong());
		assertEqual("userpassword", vals[0L]["Attribut"].AsCharPtr());
		assertEqual("{SHA}dVwAH0rjyIQ+WlDdaqL6I4k9060=", vals[0L]["Values"][0L].AsCharPtr());
		assertEqual(2, vals[1L]["Mode"].AsLong());
		assertEqual("initials", vals[1L]["Attribut"].AsCharPtr());
		assertEqual(1, vals[1L]["Values"][0L].AsLong());
		assertEqual(5, vals[1L]["Values"][1L].AsLong());
	}
}

void LDAPMapperTest::LDAPAddTest()
{
	StartTrace(LDAPMapperTest.ModifyValsTest);
	{
		Anything inputArgs;
		inputArgs["FD_REMOTE_USER"] = "tkgoa@10601.CH";
		inputArgs["Input"]["TKFModTimeValue"] = "19990520_16:54:18";
		inputArgs["Input"]["Passwd"] = "aaaaaaaaaaa"; //SHA1  11 byte: '755c001f4ae3c8843e5a50dd6aa2fa23893dd3ad'

		inputArgs["Input"]["UniversalId"] = "userpassword";

		Context ctx(inputArgs, inputArgs, (Server *)0, (Session *)0, (Role *)0);
		ParameterMapper addmapper("LDAPAddTest_fixed");

		addmapper.CheckConfig("ParameterMapper");

		String dname;
		{
			OStringStream os(&dname);
			t_assert(addmapper.Get("DName", os, ctx));
		}
		assertEqual("uid=tkgoa@10601.CH,", dname);

		ParameterMapper addvalsmapper("AddVals_fixed");
		addvalsmapper.CheckConfig("ParameterMapper");

		Anything vals;
		t_assert(addvalsmapper.Get("Modify", vals, ctx));

		TraceAny(vals, "My modify values: ");

		// uid
		assertEqual(0, vals[0L]["Mode"].AsLong());
		assertEqual("uid", vals[0L]["Attribut"].AsCharPtr());
		assertEqual("userpassword", vals[0L]["Values"][0L].AsCharPtr());

		// userpasword
		assertEqual(0, vals[1L]["Mode"].AsLong());
		assertEqual("userpassword", vals[1L]["Attribut"].AsCharPtr());
		assertEqual("{SHA}dVwAH0rjyIQ+WlDdaqL6I4k9060=", vals[1L]["Values"][0L].AsCharPtr());

		// tkfmodtime
		assertEqual(0, vals[2L]["Mode"].AsLong());
		assertEqual("tkfmodtime", vals[2L]["Attribut"].AsCharPtr());
		assertEqual("19990520_16:54:18", vals[2L]["Values"][0L].AsCharPtr());
	}
}

#endif
