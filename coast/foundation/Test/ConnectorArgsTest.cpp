/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ConnectorArgsTest.h"

//--- module under test --------------------------------------------------------

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

//---- ConnectorArgsTest ----------------------------------------------------------------

ConnectorArgsTest::ConnectorArgsTest(TString tname) : TestCase(tname)
{
}

ConnectorArgsTest::~ConnectorArgsTest()
{
	StartTrace(ConnectorArgsTest.Dtor);
}

void ConnectorArgsTest::setUp ()
{
} // setUp

void ConnectorArgsTest::tearDown ()
{
} // tearDown

void ConnectorArgsTest::test()
{
	StartTrace(ConnectorArgsTest.test);
}

void ConnectorArgsTest::testConnectorArgs()
{
	StartTrace(ConnectorArgsTest.test);
	ConnectorArgs ca("192.168.1.1", 9999, 40);
	assertEqual(ca.IPAddress(), "192.168.1.1");
	assertEqual(ca.Port(), 9999);
	assertEqual(ca.ConnectTimeout(), 40);

	ConnectorArgs ca1("", 9999, 40);
	assertEqual(ca1.IPAddress(), "127.0.0.1");
	assertEqual(ca1.Port(), 9999);
	assertEqual(ca1.ConnectTimeout(), 40);
	// Default constructor
	ConnectorArgs  ca2;
	assertEqual(ca2.IPAddress(), "127.0.0.1");
	assertEqual(ca2.Port(), 80);
	assertEqual(ca2.ConnectTimeout(), 0);
	// assignment operator
	ConnectorArgs ca3;
	ca3 = ca;
	assertEqual(ca3.IPAddress(), "192.168.1.1");
	assertEqual(ca3.Port(), 9999);
	assertEqual(ca3.ConnectTimeout(), 40);
	// copy constructor
	ConnectorArgs ca4(ca);
	assertEqual(ca4.IPAddress(), "192.168.1.1");
	assertEqual(ca4.Port(), 9999);
	assertEqual(ca4.ConnectTimeout(), 40);
}

// builds up a suite of ConfiguredTestCases, add a line for each testmethod
Test *ConnectorArgsTest::suite ()
{
	StartTrace(ConnectorArgsTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, ConnectorArgsTest, test);
	ADD_CASE(testSuite, ConnectorArgsTest, testConnectorArgs);

	return testSuite;
}
