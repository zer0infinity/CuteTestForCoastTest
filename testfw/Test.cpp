/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "Test.h"
//--- standard modules used ----------------------------------------------------
#include "TestResult.h"
//--- c-library modules used ---------------------------------------------------
#include <math.h>

#if defined(fstream)
#undef fstream /* use plain fstream when unlocked is defined for it */
#endif

/* check for a failed general assertion */
bool Test::assertImplementation (bool condition,
								 TString conditionExpression,
								 long lineNumber,
								 TString fileName)
{
	return assertImplementation(condition, conditionExpression, lineNumber,
								fileName, "");
} // assertImplementation

bool Test::assertImplementation (bool condition,
								 TString conditionExpression,
								 long lineNumber,
								 TString fileName,
								 TString message)
{
	if (condition) {
		fResult->addSuccess(new TestLocation (this, conditionExpression,
											  fileName, lineNumber, message));
		return true;
	} else {
		fResult->addFailure(new TestLocation (this, conditionExpression,
											  fileName, lineNumber, message));
	} // if
	return false;
}

/* check for a failed equality assertion */
bool Test::assertEquals (long expected,
						 long actual,
						 long lineNumber,
						 TString fileName)
{
	return assertEquals (expected, actual, lineNumber, fileName, "");
} // assertImplementation

bool Test::assertEquals (long expected,
						 long actual,
						 long lineNumber,
						 TString fileName,
						 TString message)
{
	return assertImplementation ((expected == actual),
								 notEqualsMessage(expected, actual), lineNumber, fileName, message);
}

/* check for a failed equality assertion for char* */
bool Test::assertEquals (const char *expected,
						 const char *actual,
						 long lineNumber,
						 TString fileName)
{
	return assertEquals (expected, actual, lineNumber, fileName, "");
} // assertEquals

/* check for a failed equality assertion for char* */
bool Test::assertEquals (const char *expected, long lengthExpected,
						 const char *actual, long lengthActual,
						 long lineNumber,
						 TString fileName)
{
	return assertEquals (expected, lengthExpected, actual, lengthActual, lineNumber, fileName, "");
} // assertEquals

// At least one of expected or actual is NULL
bool Test::assertEqualsIfNullPtr(const char *expected,
								 const char *actual,
								 long lineNumber,
								 TString fileName,
								 TString message)
{
	if (expected == 0) {
		if (actual == 0) {	// assertion is true
			return assertImplementation(1,
										notEqualsMessage("<Null pointer>", "<Null pointer>"),
										lineNumber, fileName, message);
		} else {			// assertion fails
			return assertImplementation(0,
										notEqualsMessage("<Null pointer>", actual),
										lineNumber, fileName, message);

		} // if
	} else if (actual == 0) {	// this is assertion certainly fails
		return assertImplementation(0,
									notEqualsMessage(expected, "<Null pointer>"),
									lineNumber, fileName, message);
	}
	return false;
}

bool Test::assertEquals (const char *expected,
						 const char *actual,
						 long lineNumber,
						 TString fileName,
						 TString message)
{
	if (expected == 0 || actual == 0) {
		return assertEqualsIfNullPtr(expected, actual, lineNumber, fileName, message);
	} else {				// both strings are not 0
		return assertImplementation ((strcmp(expected, actual) == 0),
									 notEqualsMessage(expected, actual),
									 lineNumber, fileName, message);
	} // if
	return false;
}

bool Test::assertEquals (const char *expected, long lengthExpected,
						 const char *actual, long lengthActual,
						 long lineNumber,
						 TString fileName,
						 TString message)
{
	if (expected == 0 || actual == 0) {
		return assertEqualsIfNullPtr(expected, actual, lineNumber, fileName, message);
	} else if (lengthExpected != lengthActual) { // this is assertion certainly fails
		return assertImplementation(0,
									notEqualsMessage(expected, lengthExpected, actual, lengthActual),
									lineNumber, fileName, message);
	} else {				// both strings are not 0
		return assertImplementation ((memcmp(expected, actual, lengthActual) == 0),
									 notEqualsMessage(expected, lengthExpected, actual, lengthActual),
									 lineNumber, fileName, message);
	} // if
	return false;
}

/* check for a failed equality assertion */
bool Test::assertEquals (double expected,
						 double actual,
						 double delta,
						 long lineNumber,
						 TString fileName)
{
	return assertEquals (expected, actual, delta, lineNumber, fileName, "");
} //assertEquals
bool Test::assertEquals (double expected,
						 double actual,
						 double delta,
						 long   lineNumber,
						 TString fileName,
						 TString message)
{
	return assertImplementation ((fabs (expected - actual) < delta),
								 notEqualsMessage(expected, actual),
								 lineNumber, fileName, message);

}

/* build a message about a failed equality check */
//!@FIXME make the message a parameter
TString Test::notEqualsMessage (long expected, long actual)
{
	TString message("expected: ");
	message << expected << " but was: " << actual;
	return message;
}

/* build a message about a failed equality check */
TString Test::notEqualsMessage (const char *expected, const char *actual)
{
	TString message;
	// evaluate difference and show it; useful for very long and complicated strings
	TString strExpected(expected), outbuf, strActual(actual);
	long diffPos;
	if ((diffPos = strExpected.DiffDumpAsHex(outbuf, strActual)) >= 0L) {
		message << '\n';
		message << "Difference at position " << diffPos << '\n';
		message << outbuf;
	}
	return message;
}

/* build a message about a failed equality check */
TString Test::notEqualsMessage (const char *expected,
								long lengthExpected,
								const char *actual,
								long lengthActual)
{
	TString message;
	// evaluate difference and show it; useful for very long and complicated strings
	TString strExpected(expected, lengthExpected), outbuf, strActual(actual, lengthActual);
	long diffPos;
	if ((diffPos = strExpected.DiffDumpAsHex(outbuf, strActual)) >= 0L) {
		message << '\n';
		message << "Difference at position " << diffPos << '\n';
		message << outbuf;
	}
	return message;
}

/* build a message about a failed equality check */
TString Test::notEqualsMessage (double expected, double actual)
{
	TString message("expected: ");
	message << expected << " but was: " << actual;
	return message;
}
