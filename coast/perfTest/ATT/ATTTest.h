/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ATTTest_H
#define _ATTTest_H

//---- baseclass include -------------------------------------------------
#include "ConfiguredTestCase.h"

//---- ATTTest ----------------------------------------------------------
//!ConfiguredTestCases description
class ATTTest : public ConfiguredTestCase
{
public:
	//--- constructors

	//!ConfiguredTestCase constructor
	//! \param name name of the test
	ATTTest(TString name);

	//!destroys the test case
	~ATTTest();

	//--- public api

	//!builds up a suite of ConfiguredTestCases for this test
	static Test *suite ();

protected:
	//!runs a number of stressers
	void RunTheStressers();

	// Print the results for a single run
	void PrintResult(String &stresserName, Anything &result);
	// Prints the totals of all runs
	void PrintResultTotals();
	//!Prepare and prints results of a single run for distributing
	void PrepareResults(Anything result);

	long fNrOfTests;
	long fNrOfErrors;
	long fNrOfSteps;
	long fNrOfStepErrors;
	String fResultOutput;
	String fSummaryOutput;

};

//!DoneScenariosTest description
class DoneScenariosTest : public ATTTest
{
public:
	//!DoneScenariosTest constructor
	DoneScenariosTest(TString name): ATTTest(name) {};

	//!destroys the test case
	~DoneScenariosTest() {};

	//!builds up a suite of ConfiguredTestCases for this test
	static Test *suite ();

	//!Run the scenarios that are done and that should not fail anymore
	void DoneScenarios();

};

//!InWorkScenariosTest description
class InWorkScenariosTest : public ATTTest
{
public:
	//!InWorkScenariosTest constructor
	InWorkScenariosTest(TString name): ATTTest(name) {};

	//!destroys the test case
	~InWorkScenariosTest() {};

	//!builds up a suite of ConfiguredTestCases for this test
	static Test *suite ();

	//!Run the scenarios that are in work // can fail
	void InWorkScenarios();

};

//!RunOnlyScenariosTest description
class RunOnlyScenariosTest : public ATTTest
{
public:
	//!RunOnlyScenariosTest constructor
	RunOnlyScenariosTest(TString name): ATTTest(name) {};

	//!destroys the test case
	~RunOnlyScenariosTest() {};

	//!builds up a suite of ConfiguredTestCases for this test
	static Test *suite ();

	//!Run the scenarios that are in work // can fail
	void RunOnlyScenarios();

};

#endif
