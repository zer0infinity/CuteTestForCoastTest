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
#include "AnythingTest.h"
#include "AnySorterTest.h"
#include "AcceptorTest.h"
#include "ConnectorTest.h"
#include "StringTest.h"
#include "StringTokenizerTest.h"
#include "StringTokenizer2Test.h"
#include "ParserTest.h"
#include "URLUtilsTest.h"
#include "AnyUtilsTest.h"
#include "SocketStreamTest.h"
#include "StrSpecialTest.h"
#include "SocketTest.h"
#include "ResolverTest.h"
#include "SysLogTest.h"
#include "DiffTimerTest.h"
#include "DbgTest.h"
#include "SystemTest.h"
#include "MemTrackerTest.h"
#include "StorageTest.h"
#include "ROSimpleAnythingTest.h"
#include "GenericXMLParserTest.h"
#include "AnyBuiltInSortTest.h"
#include "PipeTest.h"
#include "PipeStreamTest.h"
#include "PipeExecutorTest.h"
#include "ConnectorArgsTest.h"
#include "ObjectListTest.h"
#include "TimeStampTest.h"
#include "ConversionUtilsTest.h"
#include "AnyImplsTest.h"
#include "TypeTraitsTest.h"
#include "PoolAllocatorTest.h"
#include "AnythingIteratorTest.h"
#include "AnythingSTLTest.h"
#include "STLStorageTest.h"

#ifndef __370__
#if !defined(WIN32)
#include "MmapTest.h"
#endif
#include "StringStreamTest.h"
#endif

void setupRunner(TestRunner &runner)
{
#if !defined(__370__)
	ADD_SUITE(runner, StringTokenizerTest);
	// removed because of name clash
#endif
	ADD_SUITE(runner, StringTokenizer2Test);
	ADD_SUITE(runner, PipeTest);
	ADD_SUITE(runner, PipeStreamTest);
	ADD_SUITE(runner, PipeExecutorTest);
	ADD_SUITE(runner, DiffTimerTest);
	ADD_SUITE(runner, StringTest);
	ADD_SUITE(runner, AnyImplsTest);
	ADD_SUITE(runner, AnythingTest);
	ADD_SUITE(runner, AnySorterTest);
	ADD_SUITE(runner, AcceptorTest);
	ADD_SUITE(runner, ParserTest);
	ADD_SUITE(runner, ResolverTest);
	ADD_SUITE(runner, URLUtilsTest);
	ADD_SUITE(runner, AnyUtilsTest);
	ADD_SUITE(runner, StrSpecialTest);
#if !defined(__370__) && !defined(WIN32)
	ADD_SUITE(runner, MmapTest);
#endif
	ADD_SUITE(runner, SocketTest);
	ADD_SUITE(runner, SocketStreamTest);
	ADD_SUITE(runner, ConnectorTest);
#if !defined(__370__)
	ADD_SUITE(runner, StringStreamTest);
#endif
	ADD_SUITE(runner, DbgTest);
	ADD_SUITE(runner, MemTrackerTest);
	ADD_SUITE(runner, StorageTest);
	ADD_SUITE(runner, PoolAllocatorTest);
	ADD_SUITE(runner, STLStorageTest);
	ADD_SUITE(runner, ROSimpleAnythingTest);
	ADD_SUITE(runner, SysLogTest);
	ADD_SUITE(runner, GenericXMLParserTest);
	ADD_SUITE(runner, AnyBuiltInSortTest);
	ADD_SUITE(runner, ConnectorArgsTest);
	ADD_SUITE(runner, ObjectListTest);
	ADD_SUITE(runner, TimeStampTest);
	ADD_SUITE(runner, ConversionUtilsTest);
	ADD_SUITE(runner, TypeTraitsTest);
	ADD_SUITE(runner, AnythingIteratorTest);
	ADD_SUITE(runner, AnythingSTLTest);

	// put last since cirtical system paths are manipulated that may
	// affect proper operation of other tests
	ADD_SUITE(runner, SystemTest);
} // setupRunner
