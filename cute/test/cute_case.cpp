/*********************************************************************************
 * This file is part of CUTE.
 *
 * CUTE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * CUTE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with CUTE.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright 2008 Peter Sommerlad
 * Copyright (c) 2015, David Tran, Faculty of Computer Science,
 * University of Applied Sciences Rapperswil (HSR),
 * 8640 Rapperswil, Switzerland
 *
 *********************************************************************************/

#include "cute.h"
#include <iostream>

#include "cute_case.h"
#include "test_dummy.h"
#include "test_cute_equals.h"
#include "test_cute_expect.h"
#include "test_repeated_test.h"
#include "test_cute_runner.h"
#include "test_cute_suite_test.h"
#include "test_cute_suite.h"
#include "test_cute_test_incarnate.h"
#include "test_cute_test.h"
#include "test_cute_testmember.h"
#include "test_cute.h"
#include "test_cute_to_string.h"
#include "test_cute_to_string_embedded.h"
#include "test_xml_listener.h"
#include "test_xml_file_opener.h"
#include "test_cute_filter_runner.h"
#include "test_cute_relops.h"
#include "test_cute_data_driven.h""

using namespace cute;
// some brain dead test cases to find out my bug using function
// names without & as function pointers
static int simpleTestfunctionCalled=0;
void simpleTestFunction(){
	++simpleTestfunctionCalled;
	std::cerr << "simpleTestFunction run no:"<< simpleTestfunctionCalled << std::endl;
	ASSERT(true);
	throw std::exception();
}
struct SimpleTestFunctionCalledTest {
	void operator()() const{
		ASSERT_EQUALM("look at cute::test ctor overload",1,simpleTestfunctionCalled);
	}
};
void shouldFailButNotThrowStdException(){
	ASSERT(false);
	throw std::exception();
}
// demonstrates how to write equality tests
void test2(){
	ASSERT_EQUAL(1,1);
	ASSERT_EQUAL(1,2);
}
// demonstrates how to write test functors
struct test3{
	void operator()() const{
		throw std::exception();
	}
};
// demonstrates how test objects are incarnated
struct to_incarnate{
	std::ostream &out;
	to_incarnate(std::ostream &os):out(os){
		out << "born" << std::endl;
	}
	to_incarnate(to_incarnate &other):out(other.out){} // copy ctor would be deleted and is required by std::function ctor
	~to_incarnate() {
		out << "killed" << std::endl;
	}
	void operator()(){
		out << "tested" << std::endl;
	}
};
struct to_incarnate_without : to_incarnate {
	to_incarnate_without():to_incarnate(std::cout){}
};
void test_SimpleTestFunctionThrows(){
	ASSERT_THROWS(simpleTestFunction(),std::exception);
}
void test_shouldFailThrowsFailure(){
	ASSERT_THROWS(shouldFailButNotThrowStdException(),cute::test_failure);
}
const char * setupSuite(cute::suite &s) {
	using namespace std;
	TestDummy::runAllTests(s);
	s += make_suite_test_cute_data_driven();
	s += make_suite_test_cute_relops();
	s += make_suite_test_cute_filter_runner();
	s += make_suite_test_xml_listener();
	s += make_suite_test_xml_file_opener();
	s += make_suite_test_cute_to_string_embedded();
	s += test_cute_to_string();
	s += test_cute_equals();
	// the following test produces one of the 2 expected errors, since it throws
	//s += CUTE(simpleTestFunction);
	s += CUTE(test_SimpleTestFunctionThrows);
	s += SimpleTestFunctionCalledTest();
	s += CUTE(test_shouldFailThrowsFailure);
	suite throwing=test_cute_expect();
	s.insert(s.end(),throwing.begin(),throwing.end());
	s += CUTE_SUITE_TEST(test_repeated_test());
	s += CUTE(test_cute_runner);
	s += CUTE_SUITE_TEST(test_cute_suite_test());
	s += CUTE(test_cute_suite);
	s += CUTE(test_lambda_suite);
	s += CUTE_SUITE_TEST(test_cute_test_incarnate());
	s += CUTE_SUITE_TEST(test_cute_test());
	s += CUTE_SUITE_TEST(test_cute_testmember());
	s += CUTE_SUITE_TEST(test_cute());
	//---
	s += CUTE_INCARNATE(to_incarnate_without);
	s += CUTE_INCARNATE_WITH_CONTEXT(to_incarnate,boost_or_tr1::ref(std::cout));
	s += CUTE_CONTEXT_MEMFUN(boost_or_tr1::ref(std::cerr),to_incarnate,operator());
	return "cutetest";
}
