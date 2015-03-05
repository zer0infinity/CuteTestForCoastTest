/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "TestRunner.h"

// this is just included to test the definition of the setupRunner function
#include "SetupRunner.h"
#include "sys/times.h"
#if !defined(WIN32) && !defined(gettimes)
HRTESTTIME  gettimes()
{
	struct tms tt;
	return times(&tt);
}
#endif

int main (int ac, const char **av)
{
	TestRunner runner;

	setupRunner(runner);
	runner.run (ac, av);
	return runner.getNumberOfFailures();
}
