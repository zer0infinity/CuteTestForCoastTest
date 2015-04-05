/*
 * Copyright (c) 2015, David Tran, Faculty of Computer Science,
 * University of Applied Sciences Rapperswil (HSR),
 * 8640 Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "cute_case.h"
#include "ROSimpleAnythingTest.h"
#include "StringIteratorTest.h"
#include "StringTestExtreme.h"
#include "StringTokenizerTest.h"
#include "StrSpecialTest.h"
#include "SysLogTest.h"
#include "TracerTest.h"
#include "TypeTraitsTest.h"
#include "AnythingIteratorTest.h"
#include "StringReverseIteratorTest.h"
#include "AnyImplsTest.h"
#include "AnythingLookupTest.h"
#include "StringSTLTest.h"
#include "StringTokenizer2Test.h"
#include "SystemBaseTest.h"
#include "AnythingDeepCloneTest.h"
#include "AnythingConstructorsTest.h"

void setupSuite(cute::suite &s) {
	ROSimpleAnythingTest::runAllTests(s);
	StringIteratorTest::runAllTests(s);
	StringTestExtreme::runAllTests(s);
	StringTokenizerTest::runAllTests(s);
	StrSpecialTest::runAllTests(s);
	SysLogTest::runAllTests(s);
	TracerTest::runAllTests(s);
	TypeTraitsTest::runAllTests(s);
	AnythingIteratorTest::runAllTests(s);
	StringReverseIteratorTest::runAllTests(s);
	AnyImplsTest::runAllTests(s);
	AnythingLookupTest::runAllTests(s);
	StringSTLTest::runAllTests(s);
	StringTokenizer2Test::runAllTests(s);
	SystemBaseTest::runAllTests(s);
	AnythingDeepCloneTest::runAllTests(s);
	AnythingConstructorsTest::runAllTests(s);
}
