/*
 * Copyright (c) 2008, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _StringTestExtreme_h_
#define _StringTestExtreme_h_

#include "TestCase.h"

//---- forward declaration -----------------------------------------------
#include "ITOString.h"
//---- StringTestExtreme -----------------------------------------------------------
//!testcases for String
class StringTestExtreme : public TestFramework::TestCase
{
protected:
	String extremelyLongString;
	std::string trueString;

public:
	StringTestExtreme (TString tstrName);
	virtual ~StringTestExtreme();

	virtual void			setUp ();
	static Test				*suite ();

	void			extremeLength ();
};

#endif
