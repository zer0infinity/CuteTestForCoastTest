/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "TestRunner.h"

//--- standard modules used ----------------------------------------------------
#include "config_att.h"

//--- test cases ---------------------------------------------------------------
#include "ATTTest.h"

#if defined(WIN32)
#include "config_bases.h"
void bases()
{
	RegisterBasesObjs();
}
#endif

void EXPORTDECL_ATT setupRunner(TestRunner &runner)
{
	ADD_SUITE(runner, InWorkScenariosTest);
	ADD_SUITE(runner, DoneScenariosTest);
	// Use this work suite to debug a failing TestCase
	ADD_SUITE(runner, RunOnlyScenariosTest);
} // setupRunner
