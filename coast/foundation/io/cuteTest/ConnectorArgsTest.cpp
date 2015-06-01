/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * Copyright (c) 2015, David Tran, Faculty of Computer Science,
 * University of Applied Sciences Rapperswil (HSR),
 * 8640 Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "ConnectorArgsTest.h"

#include "Socket.h"
#include "Tracer.h"

ConnectorArgsTest::ConnectorArgsTest()
{
}

ConnectorArgsTest::~ConnectorArgsTest()
{
	StartTrace(ConnectorArgsTest.Dtor);
}

void ConnectorArgsTest::ArgsTest()
{
	StartTrace(ConnectorArgsTest.ArgsTest);
	ConnectorArgs ca("192.168.1.1", 9999, 40);
	ASSERT_EQUAL(ca.IPAddress(), "192.168.1.1");
	ASSERT_EQUAL(ca.Port(), 9999);
	ASSERT_EQUAL(ca.ConnectTimeout(), 40);

	ConnectorArgs ca1("", 9999, 40);
	ASSERT_EQUAL(ca1.IPAddress(), "127.0.0.1");
	ASSERT_EQUAL(ca1.Port(), 9999);
	ASSERT_EQUAL(ca1.ConnectTimeout(), 40);
	// Default constructor
	ConnectorArgs  ca2;
	ASSERT_EQUAL(ca2.IPAddress(), "127.0.0.1");
	ASSERT_EQUAL(ca2.Port(), 80);
	ASSERT_EQUAL(ca2.ConnectTimeout(), 0);
	// assignment operator
	ConnectorArgs ca3;
	ca3 = ca;
	ASSERT_EQUAL(ca3.IPAddress(), "192.168.1.1");
	ASSERT_EQUAL(ca3.Port(), 9999);
	ASSERT_EQUAL(ca3.ConnectTimeout(), 40);
	// copy constructor
	ConnectorArgs ca4(ca);
	ASSERT_EQUAL(ca4.IPAddress(), "192.168.1.1");
	ASSERT_EQUAL(ca4.Port(), 9999);
	ASSERT_EQUAL(ca4.ConnectTimeout(), 40);
}

// builds up a suite of tests, add a line for each testmethod
void ConnectorArgsTest::runAllTests(cute::suite &s) {
	StartTrace(ConnectorArgsTest.suite);
	s.push_back(CUTE_SMEMFUN(ConnectorArgsTest, ArgsTest));

}
