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
#include "AnythingConstructorsTest.h"
#include "AnythingDeepCloneTest.h"
#include "AnythingImportExportTest.h"
#include "AnythingLookupTest.h"
#include "AnythingKeyIndexTest.h"
#include "AnythingTest.h"
#include "StringTest.h"
#include "StringTestExtreme.h"
#include "StringTokenizerTest.h"
#include "StringTokenizer2Test.h"
#include "AnythingParserSemanticTest.h"
#include "AnythingParserTest.h"
#include "StrSpecialTest.h"
#include "SysLogTest.h"
#include "TracerTest.h"
#include "SystemBaseTest.h"
#include "SystemFileTest.h"
#include "ROSimpleAnythingTest.h"
#include "AnyBuiltInSortTest.h"
#include "AnyImplsTest.h"
#include "TypeTraitsTest.h"
#include "AnythingIteratorTest.h"
#include "AnythingSTLTest.h"
#if !defined(WIN32)
#include "MmapTest.h"
#endif
#include "StringStreamTest.h"
#include "StringIteratorTest.h"
#include "StringReverseIteratorTest.h"
#include "StringSTLTest.h"
#include "AnythingImportExportTest.h"
#include "AnythingKeyIndexTest.h"

const char * setupSuite(cute::suite &s) {
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
	AnythingParserSemanticTest::runAllTests(s);
	StringTest::runAllTests(s);
#if !defined(WIN32)
	MmapTest::runAllTests(s);
#endif
	AnyBuiltInSortTest::runAllTests(s);
	AnythingTest::runAllTests(s);
	AnythingSTLTest::runAllTests(s);
	StringStreamTest::runAllTests(s);
	AnythingImportExportTest::runAllTests(s);
	AnythingKeyIndexTest::runAllTests(s);
	AnythingParserTest::runAllTests(s);
	SystemFileTest::runAllTests(s);
	return "CuteFoundationBaseTest";
}
