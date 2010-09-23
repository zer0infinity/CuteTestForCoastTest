/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "TestRunner.h"
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------

//--- test cases -------------------------------------------------------------
#include "PipeTest.h"
#include "PipeStreamTest.h"
#include "PipeExecutorTest.h"
#include "SocketTest.h"
#include "SocketStreamTest.h"
#include "ResolverTest.h"
#include "URLUtilsTest.h"
#include "ConnectorTest.h"
#include "ConnectorArgsTest.h"
#include "AcceptorTest.h"

void setupRunner(TestRunner &runner)
{
	ADD_SUITE(runner, PipeTest);
	ADD_SUITE(runner, PipeStreamTest);
	ADD_SUITE(runner, PipeExecutorTest);
	ADD_SUITE(runner, SocketTest);
	ADD_SUITE(runner, SocketStreamTest);
	ADD_SUITE(runner, ResolverTest);
	ADD_SUITE(runner, URLUtilsTest);
	ADD_SUITE(runner, ConnectorTest);
	ADD_SUITE(runner, ConnectorArgsTest);
	ADD_SUITE(runner, AcceptorTest);
} // setupRunner
