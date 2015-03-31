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

#ifndef SYSTEMBASETEST_H_
#define SYSTEMBASETEST_H_

#include "cute.h"
#include "SystemFile.h"
#include "Tracer.h"
#include "SystemBase.h"

class SystemBaseTest {
public:
	static void runAllTests(cute::suite &s);

	void GetProcessEnvironmentTest();

#if !defined(WIN32)
	void LockFileTest();
#endif

	void DoSingleSelectTest();
	void MicroSleepTest();

	//!storage tests
	void allocFreeTests();

	void TimeTest();

	void SnPrintf_ReturnsBytesOfContentWrittenWithoutTerminatingZero();
	void SnPrintf_ReturnsBytesRequiredWithoutTerminatingZero();
	void SnPrintf_WritesTerminatingZeroEvenWhenTruncatingBuffer();
};

#endif
