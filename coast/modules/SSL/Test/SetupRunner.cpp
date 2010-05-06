/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "Socket.h"

//--- interface include --------------------------------------------------------
#include "TestRunner.h"

//--- test cases ---------------------------------------------------------------
#include "SSLConnectorTest.h"
#include "SSLListenerPoolTest.h"
#include "SSLCertificateTest.h"
#include "SSLSocketArgsTest.h"
#include "SSLSocketUtilsTest.h"
#include "SSLObjectManagerTest.h"
#include "SSLModuleTest.h"

void setupRunner(TestRunner &runner)
{
	ADD_SUITE(runner, SSLSocketArgsTest);
	ADD_SUITE(runner, SSLSocketUtilsTest);
	ADD_SUITE(runner, SSLModuleTest);
	ADD_SUITE(runner, SSLConnectorTest);
	ADD_SUITE(runner, SSLListenerPoolTest);
	ADD_SUITE(runner, SSLCertificateTest);
	ADD_SUITE(runner, SSLObjectManagerTest);
} // setupRunner
