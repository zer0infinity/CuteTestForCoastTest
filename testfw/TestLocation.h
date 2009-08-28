/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _IT_TESTFW_TEST_LOCATION_H
#define _IT_TESTFW_TEST_LOCATION_H

#include "TString.h"
class Test;
//! <B>A TestLocation collects information about a test result (success or failure)</B>
/*!
 see TestResult and TestSuite
*/
class TestLocation
{
protected:
	TString fMessage;
	long 	fLineNumber;
	TString fFileName;
	TString fTestName;

public:
	TestLocation (Test *aTest, TString &cause, TString &fileName,
				  long lineNumber, TString &message);

	// We do not own the test ...
	~TestLocation () {};

	TString	toString ();

	TString	&getMessage() {
		return(fMessage);
	};
	long	getLineNumber() {
		return(fLineNumber);
	};
	TString	&getFileName() {
		return(fFileName);
	};
	TString	&getTestName() {
		return(fTestName);
	};

	// TestLocation		operator = (TestLocation);
private:
	TestLocation();		// not implemented, must not be used
};

#endif
