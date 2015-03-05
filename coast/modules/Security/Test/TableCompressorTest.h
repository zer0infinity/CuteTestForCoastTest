/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _TableCompressorTest_H
#define _TableCompressorTest_H

#include "TestCase.h"

class TableCompressor;

class TableCompressorTest: public testframework::TestCase {
public:
	TableCompressorTest(TString tstrName) :
		TestCaseType(tstrName) {
	}
	static Test *suite();

	//!test working of configuration
	void InitTest();
	//!test working of configuration
	void InitWithExpansionTest();
	//!test working of compression/expansion
	void CompressExpandTest();
	//!test working of compression/expansion in case of misconfiguration
	void CompressExpandWithEmptyConfigTest();
	//!test the utilities
	void CalcAtTest();
	void CalcKeyTest();
	void MakeTableTest();
	void MakeRevTableTest();
	void InstallConfigTest();
	void ExpandConfigTest();

protected:
	//--- subclass api
	void MakeTestTable(TableCompressor *);
};

#endif
