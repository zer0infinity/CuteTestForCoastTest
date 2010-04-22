/*
 * Copyright (c) 2007, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _BoostPoolTest_H
#define _BoostPoolTest_H

//---- baseclass include -------------------------------------------------
#include "FoundationTestTypes.h"

#include <boost/preprocessor/seq.hpp>
#include <boost/preprocessor/seq/for_each_product.hpp>
#include <boost/preprocessor/seq/to_tuple.hpp>
#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/stringize.hpp>

class Allocator;

#define FUNCDECLARER(TSTFUNC, NTHREADS, NSIZE) \
	void BOOST_PP_SEQ_CAT((PoolPerfTest)(TSTFUNC)(NTHREADS)(NSIZE))();

#define FUNCDECLARERTUPLE(ARGSTUPLE) \
	FUNCDECLARER( BOOST_PP_TUPLE_ELEM(3,0,ARGSTUPLE), BOOST_PP_TUPLE_ELEM(3,1,ARGSTUPLE), BOOST_PP_TUPLE_ELEM(3,2,ARGSTUPLE) )

#define FUNCDECLARERMACRO(r,product)\
	FUNCDECLARERTUPLE(BOOST_PP_SEQ_TO_TUPLE(product))

#define NTHREADS_SEQ1	(1)(4)(8)
#define NTHREADS_SEQ2	(1)(2)(4)(8)(16)
#define NSIZE_SEQ1		(2)(3)(16)(17)
#define NSIZE_SEQ2		(2)(3)(8)(9)(16)(17)(32)(33)
#define TESTFUNC_SEQ	(delete_new)(delete_new_array)			// (new_del_on_arr)(new_del_a_on_a)

//#define EXPAND_SEQUENCE	(TESTFUNC_SEQ)((2))((17))
#define EXPAND_SEQUENCE	(TESTFUNC_SEQ)(NTHREADS_SEQ1)(NSIZE_SEQ1)
//#define EXPAND_SEQUENCE	(TESTFUNC_SEQ)(NTHREADS_SEQ2)(NSIZE_SEQ2)	// more extensive tests

//---- BoostPoolTest ----------------------------------------------------------
//! <B>really brief class description</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
class BoostPoolTest : public TestFramework::TestCaseWithStatistics
{
public:
	//--- constructors

	/*! \param tstrName name of the test */
	BoostPoolTest(TString tstrName);

	//! destroys the test case
	~BoostPoolTest();

	//--- public api

	//! builds up a suite of testcases for this test
	static Test *suite ();

// uncomment if really needed or delete, base class does almost everything
//	//! sets the environment for this test
//	void setUp ();
//
//	//! deletes the environment for this test
//	void tearDown ();

	//! describe this testcase
	void BasicsTest();
	void AllocationTest();
	void MemUsageTest();
	void GlobalStorageMemUsageTest();
	void CurrentStorageMemUsageTest();
	void TestFuncCurrent();

	BOOST_PP_SEQ_FOR_EACH_PRODUCT(FUNCDECLARERMACRO, EXPAND_SEQUENCE)
};

#endif
