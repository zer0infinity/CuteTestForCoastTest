/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _StatisticTestCase_H
#define _StatisticTestCase_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"
#include "Anything.h"

//---- StatisticTestCase ----------------------------------------------------------
//!TestCases description
class StatisticTestCase : public TestCase
{
	Anything fStatistics;
	String fFilename, fDatetime;

public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	StatisticTestCase(TString name);

	//!destroys the test case
	~StatisticTestCase();

	//!sets the environment for this test
	void setUp ();

	//!deletes the environment for this test
	void tearDown ();

protected:
	void LoadData();
	void StoreData();
	String GenTimeStamp();
	void AddStatisticOutput(String strTestName, long lMilliTime);
	//! export timing values in csv style
	void ExportCsvStatistics(long lModulus);
};

#endif
