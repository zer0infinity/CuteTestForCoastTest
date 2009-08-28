/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef IT_TESTFW_TEXTTESTRESULT_H
#define IT_TESTFW_TEXTTESTRESULT_H

#include "TestResult.h"

//--- c-library modules used ---------------------------------------------------

#ifdef ostream
#define RESTOREUNSAFEOSTREAM 1
#undef ostream
#endif

class TextTestResult : public TestResult
{
public:
	virtual void		addError	(TestLocation *loc);
	virtual void		addFailure	(TestLocation *loc);
	virtual void		startTest	(Test *test);
	virtual void		endTest		(Test *test);
	virtual void		print (ostream &stream);
	virtual void		printCauses (ostream &stream, TestLocList &causes);
	virtual void		printErrors (ostream &stream);
	virtual void		printFailures (ostream &stream);
	virtual void		printHeader (ostream &stream);
	virtual void		logSuccesses(ostream &stream);
};

/* insertion operator for easy output */
inline ostream &operator<< (ostream &stream, TextTestResult &result)
{
	result.print (stream);
	return stream;
}
#if defined(RESTOREUNSAFEOSTREAM)
#define ostream unsafe_ostream
#endif

#endif
