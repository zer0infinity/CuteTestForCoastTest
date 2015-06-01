/*
 * Copyright (c) 2015, David Tran, Faculty of Computer Science,
 * University of Applied Sciences Rapperswil (HSR),
 * 8640 Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "cute_case.h"
//#include "AcceptorTest.h"
#include "ConnectorArgsTest.h"
#include "ResolverTest.h"

const char * setupSuite(cute::suite &s) {
//	AcceptorTest::runAllTests(s);
	ConnectorArgsTest::runAllTests(s);
	ResolverTest::runAllTests(s);
	return "CuteFoundationIOTest";
}
