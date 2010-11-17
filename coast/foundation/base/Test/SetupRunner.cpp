/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "TestRunner.h"
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------

//--- test cases -------------------------------------------------------------
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
#include "DbgTest.h"
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

void setupRunner(TestRunner &runner)
{
	ADD_SUITE(runner, StringTokenizerTest);
	ADD_SUITE(runner, StringTokenizer2Test);
	ADD_SUITE(runner, StringTest);
	ADD_SUITE(runner, AnyImplsTest);
	ADD_SUITE(runner, AnythingConstructorsTest);
	ADD_SUITE(runner, AnythingKeyIndexTest);
	ADD_SUITE(runner, AnythingTest);
	ADD_SUITE(runner, AnythingDeepCloneTest);
	ADD_SUITE(runner, AnythingImportExportTest);
	ADD_SUITE(runner, AnythingLookupTest);
	ADD_SUITE(runner, AnythingParserSemanticTest);
	ADD_SUITE(runner, AnythingParserTest);
	ADD_SUITE(runner, StrSpecialTest);
#if !defined(WIN32)
	ADD_SUITE(runner, MmapTest);
#endif
	ADD_SUITE(runner, StringStreamTest);
	ADD_SUITE(runner, DbgTest);
	ADD_SUITE(runner, ROSimpleAnythingTest);
	ADD_SUITE(runner, SysLogTest);
	ADD_SUITE(runner, AnyBuiltInSortTest);
	ADD_SUITE(runner, TypeTraitsTest);
	ADD_SUITE(runner, AnythingIteratorTest);
	ADD_SUITE(runner, AnythingSTLTest);

	// put last since cirtical system paths are manipulated that may
	// affect proper operation of other tests
	ADD_SUITE(runner, SystemBaseTest);
	ADD_SUITE(runner, SystemFileTest);
	ADD_SUITE(runner, StringTestExtreme);
} // setupRunner
