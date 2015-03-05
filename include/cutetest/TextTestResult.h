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

class TextTestResult : public TestResult
{
public:
	virtual void		addError	(TestLocation *loc);
	virtual void		addFailure	(TestLocation *loc);
	virtual void		startTest	(Test *test);
	virtual void		endTest		(Test *test);
	virtual void		print (std::ostream &stream);
	virtual void		printCauses (std::ostream &stream, TestLocList &causes);
	virtual void		printErrors (std::ostream &stream);
	virtual void		printFailures (std::ostream &stream);
	virtual void		printHeader (std::ostream &stream);
	virtual void		logSuccesses(std::ostream &stream);
private:
	long fStartTime;
};

/* insertion operator for easy output */
inline std::ostream &operator<< (std::ostream &stream, TextTestResult &result)
{
	result.print (stream);
	return stream;
}

#endif
