/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "TestLocation.h"
//--- standard modules used ----------------------------------------------------
#include "Test.h"

// Returns a short description of the failure.
TString TestLocation::toString ()
{
	return fMessage;
}

TestLocation::TestLocation (Test *aTest, TString &cause, TString &fileName,
							long lineNumber, TString &message)
// in: aTest: the test in question
//	   cause: the assertion text of the test
//	fileName: the filename of the file where the assertion is written
//lineNumber: the line number of the assertion
//   message:

{
	fMessage << cause;
	if (message.Length() > 0) {
		fMessage << "; " << message;
	} // if
	fFileName << fileName;
	fTestName << aTest->toString();
	fLineNumber = lineNumber;
}

//TestLocation  TestLocation::operator = (TestLocation aValue)
//{
//	fMessage	= aValue.fMessage;
//	fLineNumber	= aValue.fLineNumber;
//	fFileName	= aValue.fFileName;
//	return *this;
//}
