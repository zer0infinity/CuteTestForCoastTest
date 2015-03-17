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

#include "SysLogTest.h"
#include "SystemLog.h"
#include "Tracer.h"

SysLogTest::SysLogTest() {
	StartTrace(SysLogTest.setUp);
	SystemLog::Init("SysLogTest");
	StartTrace(SysLogTest.Ctor);
}

SysLogTest::~SysLogTest() {
	StartTrace(SysLogTest.Dtor);
	StartTrace(SysLogTest.tearDown);
	SystemLog::Terminate();
}

void SysLogTest::TestFlags() {
	StartTrace(SysLogTest.TestFlags);
	ASSERTM(" expected creation of fgSysLog", SystemLog::getSysLog() != NULL);
}

void SysLogTest::runAllTests(cute::suite &s) {
	s.push_back(CUTE_SMEMFUN(SysLogTest, TestFlags));
}
