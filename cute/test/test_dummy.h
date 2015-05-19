/*
 * Copyright (c) 2015, David Tran, Faculty of Computer Science,
 * University of Applied Sciences Rapperswil (HSR),
 * 8640 Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file lgpl.txt.
 */

#ifndef TESTS_TEST_DUMMY_H_
#define TESTS_TEST_DUMMY_H_

#include "cute_case.h"

struct TestDummy {
	void dummyTest();
	static void runAllTests(cute::suite &s);
};


#endif /* TESTS_TEST_DUMMY_H_ */
