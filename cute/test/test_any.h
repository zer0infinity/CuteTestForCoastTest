/*
 * Copyright (c) 2015, David Tran, Faculty of Computer Science,
 * University of Applied Sciences Rapperswil (HSR),
 * 8640 Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file lgpl.txt.
 */

#ifndef TEST_ANY_H_
#define TEST_ANY_H_

#include "cute_case.h"
#include "AssertionAnything.h"

struct TestAny : public Assertion {
	void dummyTest();
	void anyTest();
	static void runAllTests(cute::suite &s);
};


#endif /* TEST_ANY_H_ */
