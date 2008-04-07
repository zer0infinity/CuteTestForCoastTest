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
#include "config_foundation.h"	// for definition of EXPORTDECL_FOUNDATION
#include "ITOString.h"
//---- StringTestExtreme -----------------------------------------------------------
//!testcases for String
class StringTestExtreme : public TestCase
{
protected:
	String extremelyLongString;
	std::string trueString;

public:
	StringTestExtreme (TString name); // : TestCase (name) {}
	virtual ~StringTestExtreme();

	virtual void			setUp ();
	static Test				*worksuite ();
	static Test				*suite ();

	void			extremeLength ();
};

#endif
