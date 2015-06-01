/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ResolverTest_H
#define _ResolverTest_H

#include "cute.h"
#include "Tracer.h"
#include "AnyIterators.h"
#include "SystemFile.h"

class ResolverTest
{
public:
	//!builds up a suite of testcases for this test
	static void runAllTests(cute::suite &s);

	//!tests the Socket class with a fd from Connector
	void simpleDNS2IPTest();

	//!tests the Socket class with a invalid fd
	void simpleIP2DNSTest();
	ROAnything GetTestCaseConfig(String strClassName = "", String strTestName = "");
};

#endif
