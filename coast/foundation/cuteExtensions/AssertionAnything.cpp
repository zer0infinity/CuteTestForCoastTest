/*
 * Copyright (c) 2015, David Tran, Faculty of Computer Science,
 * University of Applied Sciences Rapperswil (HSR),
 * 8640 Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "AssertionAnything.h"
#include "AnyUtils.h"

void Assertion::ASSERT_ANY_EQUAL(const ROAnything &expected, const ROAnything &actual) {
	OStringStream oexp(&sexp), oact(&act);
	expected.Export(oexp, false);
	actual.Export(oact, false);
	ASSERT_EQUAL(sexp, act);
}

void Assertion::ASSERT_ANY_EQUALM(std::string msg, const ROAnything &expected, const ROAnything &actual) {
	OStringStream oexp(&sexp), oact(&act);
	expected.Export(oexp, false);
	actual.Export(oact, false);
	ASSERT_EQUALM(msg, sexp, act);
}

void Assertion::ASSERT_ANY_COMPARE(const ROAnything &inputAny, const ROAnything &masterAny, String location, char delimSlot, char idxdelim) {
	OStringStream s;
	String failingPath(location);
	if(!AnyUtils::AnyCompareEqual(inputAny, masterAny, failingPath,&s, delimSlot, idxdelim)) {
		String strfail(failingPath);
		strfail << "\n" << s.str();
		ASSERTM((const char*)strfail, false);
	}
}
