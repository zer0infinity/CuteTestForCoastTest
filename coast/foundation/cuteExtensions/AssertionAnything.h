/*
 * Copyright (c) 2015, David Tran, Faculty of Computer Science,
 * University of Applied Sciences Rapperswil (HSR),
 * 8640 Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AssertionAnything_H
#define _AssertionAnything_H

#include "cute.h"
#include "StringStream.h"
#include "AnyIterators.h"

struct Assertion {
	void ASSERT_ANY_EQUAL(const ROAnything &expected, const ROAnything &actual);
	void ASSERT_ANY_EQUALM(std::string msg, const ROAnything &expected, const ROAnything &actual);
private:
	String sexp, act;
};

#endif
