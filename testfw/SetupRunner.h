/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _IT_TESTFW_SetupRunner_h_
#define _IT_TESTFW_SetupRunner_h_

//! implement this function in your test program in order to make all test cases
//! known (and initialize useful things if you want to).
void setupRunner(class TestRunner &runner);

#endif
