/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * Copyright (c) 2015, David Tran, Faculty of Computer Science,
 * University of Applied Sciences Rapperswil (HSR),
 * 8640 Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _TypeTraitsTest_H
#define _TypeTraitsTest_H

#include "cute.h"
#include "ide_listener.h"
#include "cute_runner.h"

class TypeTraitsTest {
public:
	static void runAllTests(cute::suite &s);
	void TraitsTest();
	void ClassTraitsTest();
};

#endif
