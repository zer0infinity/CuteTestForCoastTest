/*
 * Copyright (c) 2008, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * Copyright (c) 2015, David Tran, Faculty of Computer Science,
 * University of Applied Sciences Rapperswil (HSR),
 * 8640 Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _StringTestExtreme_h_
#define _StringTestExtreme_h_

#include "cute.h"
#include "ITOString.h"//lint !e537
class StringTestExtreme {
protected:
	String extremelyLongString;
	std::string trueString;
public:
	static void runAllTests(cute::suite &s);
	StringTestExtreme();
	void extremeLength();
};

#endif
