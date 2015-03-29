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

#include "AnyImplsTest.h"
#include "AnyImpls.h"
#include "Tracer.h"
#include "StringStream.h"
#include <iomanip>

AnyImplsTest::AnyImplsTest() {
	StartTrace(AnyImplsTest.Ctor);
}

AnyImplsTest::~AnyImplsTest()
{
	StartTrace(AnyImplsTest.Dtor);
}

void AnyImplsTest::ThisToHexTest()
{
	StartTrace(AnyImplsTest.ThisToHexTest);
	{
		std::auto_ptr<AnyImpl> d = std::auto_ptr<AnyImpl>(new(coast::storage::Current())AnyLongImpl(123L, coast::storage::Current()));
		String res = d->ThisToHex();
		ASSERT_EQUAL(sizeof(void *)*2L, static_cast<unsigned long>(res.Length()));
		StringStream os;
		os << std::hex << std::setw(sizeof(d.get())*2)<<std::setfill('0')<<std::noshowbase<<reinterpret_cast<unsigned long long>(static_cast<void*>(d.get()));
		ASSERT_EQUAL(os.str(),res);
	}
}

// builds up a suite of testcases, add a line for each testmethod
void AnyImplsTest::runAllTests(cute::suite &s) {
	StartTrace(AnyImplsTest.suite);
	s.push_back(CUTE_SMEMFUN(AnyImplsTest, ThisToHexTest));
}
