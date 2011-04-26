/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef IT_TESTFW_TEST_H
#define IT_TESTFW_TEST_H

#define IT_TESTFW_UNKNOWNFILENAME                "<unknown>"
#define IT_TESTFW_UNKNOWNLINENUMBER              (-1)

#include "TString.h"
#include <functional>
using std::equal_to;
using std::not_equal_to;
using std::less;
using std::less_equal;
using std::greater;
using std::greater_equal;

class TestResult;

//template
//<
//	typename T,
//	template <typename> class CompFunc = equal_to
//>
//struct myCompareEqual
//: public binary_function< T, T, bool >
//{
//	bool operator()(const T& left, const T& right) const
//	{
//		return CompFunc<T>()(left,right);
//	}
//};
//
//	template < typename T, template <typename> class CompFunc >
//	bool myassertCompare(T left, T right, CompFunc<T> &cFunc)
//	{
////		return myCompareEqual < T, less > ()(left, right);
//		return cFunc()(left, right);
//	}

/*
 * A Test can be run and collect its results.
 * See TestResult.
 *
 */
class Test
{
public:
	virtual				~Test () = 0;
	virtual	void		run (TestResult *result);
	virtual int			countTestCases ();
	virtual TString		toString ()	;
	const char *getClassName() {
		return fClassName;
	}
	Test *getSuite() {
		return this;
	}
	Test *setClassName(const char *aName) {
		fClassName = aName;
		return this;
	}

protected:
	TestResult *fResult;
	const char *fClassName;

	Test() : fResult(0), fClassName("") {}

	bool assertImplementation (bool condition,
							   TString conditionExpression,
							   long   lineNumber,
							   TString fileName,
							   TString message);

	bool assertImplementation (bool condition,
							   TString conditionExpression = "",
							   long   lineNumber = IT_TESTFW_UNKNOWNLINENUMBER,
							   TString fileName = IT_TESTFW_UNKNOWNFILENAME);

	bool assertEquals	(long expected,
						 long actual,
						 long   lineNumber,
						 TString fileName,
						 TString message);

	bool assertEquals	(long expected,
						 long actual,
						 long   lineNumber = IT_TESTFW_UNKNOWNLINENUMBER,
						 TString fileName = IT_TESTFW_UNKNOWNFILENAME);

	bool assertEquals	(double expected,
						 double actual,
						 double delta,
						 long   lineNumber,
						 TString fileName,
						 TString message);

	bool assertEquals	(double expected,
						 double actual,
						 double delta,
						 long   lineNumber = IT_TESTFW_UNKNOWNLINENUMBER,
						 TString fileName = IT_TESTFW_UNKNOWNFILENAME);

	bool assertEquals	(const char *expected,
						 const char *actual,
						 long   lineNumber = IT_TESTFW_UNKNOWNLINENUMBER,
						 TString fileName = IT_TESTFW_UNKNOWNFILENAME);

	bool assertEquals	(const char *expected,
						 long lengthExpected,
						 const char *actual,
						 long lengthActual,
						 long   lineNumber = IT_TESTFW_UNKNOWNLINENUMBER,
						 TString fileName = IT_TESTFW_UNKNOWNFILENAME);

	bool assertEquals	(const char *expected,
						 const char *actual,
						 long   lineNumber,
						 TString fileName,
						 TString message);

	bool assertEquals	(const char *expected,
						 long lengthExpected,
						 const char *actual,
						 long lengthActual,
						 long   lineNumber,
						 TString fileName,
						 TString message);

	TString notEqualsMessage(long expected, long actual);
	TString notEqualsMessage(double expected, double actual);
	TString notEqualsMessage(const char *expected, const char *actual);
	TString notEqualsMessage (const char *expected,
							  long lengthExpected,
							  const char *actual,
							  long lengthActual);

	bool 	assertEqualsIfNullPtr(const char *expected,
								  const char *actual,
								  long lineNumber,
								  TString fileName,
								  TString message);

	template < typename T > bool assert_equal_to(T left, T right, TString addexpr, long lineNumber, TString fileName, TString message) {
		TString strExpression;
		strExpression.Append((unsigned long long)left).Append(" equal_to ").Append((unsigned long long)right).Append(addexpr);
		return assertImplementation(equal_to<T>()(left, right), strExpression, lineNumber, fileName, message);
	}
	template < typename T > bool assert_not_equal_to(T left, T right, TString addexpr, long lineNumber, TString fileName, TString message) {
		TString strExpression;
		strExpression.Append((unsigned long long)left).Append(" not_equal_to ").Append((unsigned long long)right).Append(addexpr);
		return assertImplementation(not_equal_to<T>()(left, right), strExpression, lineNumber, fileName, message);
	}
	template < typename T > bool assert_less(T left, T right, TString addexpr, long lineNumber, TString fileName, TString message) {
		TString strExpression;
		strExpression.Append((unsigned long long)left).Append(" less ").Append((unsigned long long)right).Append(addexpr);
		return assertImplementation(less<T>()(left, right), strExpression, lineNumber, fileName, message);
	}
	template < typename T > bool assert_less_equal(T left, T right, TString addexpr, long lineNumber, TString fileName, TString message) {
		TString strExpression;
		strExpression.Append((unsigned long long)left).Append(" less_equal ").Append((unsigned long long)right).Append(addexpr);
		return assertImplementation(less_equal<T>()(left, right), strExpression, lineNumber, fileName, message);
	}
	template < typename T > bool assert_greater(T left, T right, TString addexpr, long lineNumber, TString fileName, TString message) {
		TString strExpression;
		strExpression.Append((unsigned long long)left).Append(" greater ").Append((unsigned long long)right).Append(addexpr);
		return assertImplementation(greater<T>()(left, right), strExpression, lineNumber, fileName, message);
	}
	template < typename T > bool assert_greater_equal(T left, T right, TString addexpr, long lineNumber, TString fileName, TString message) {
		TString strExpression;
		strExpression.Append((unsigned long long)left).Append(" greater_equal ").Append((unsigned long long)right).Append(addexpr);
		return assertImplementation(greater_equal<T>()(left, right), strExpression, lineNumber, fileName, message);
	}
};

inline Test::~Test ()
{}

// Runs a test and collects its result in a TestResult instance.
inline void Test::run (TestResult *result)
// This method does not actually do anything, however it should be called at
// the beginning of the implementation of a subclass
{
	fResult = result;
}

// Counts the number of test cases that will be run by this test.
inline int Test::countTestCases ()
{
	return 0;
}

// Returns the name of the test instance.
inline TString Test::toString ()
{
	return fClassName;
}

// A set of macros which allow us to get the line number
// and file name at the point of an error.
// Just goes to show that preprocessors do have some
// redeeming qualities.
#define t_assert(condition)\
(this->assertImplementation ((bool)(condition),(#condition),\
	__LINE__, __FILE__))

#define assertCompare(left, what, right)\
(this->assert_##what<>( left, right, " ( " #left " " #what " " #right " )", __LINE__, __FILE__, "" ) )

#define assertComparem(left, what, right, message)\
(this->assert_##what<>( left, right, " ( " #left " " #what " " #right " )", __LINE__, __FILE__, message ) )

#define assertDoublesEqual(expected,actual,delta)\
(this->assertEquals ((expected),\
		(actual),(delta),__LINE__,__FILE__,TString(" " #expected " == " #actual " ")))

#define assertLongsEqual(expected,actual)\
(this->assertEquals ((expected),\
		(actual),__LINE__,__FILE__,TString(" " #expected " == " #actual " ")))

#define assertEqual(expected,actual)\
(this->assertEquals ((expected),\
		(actual),__LINE__,__FILE__,TString(" " #expected " == " #actual " ")))

#define assertEqualRaw(expected,actual)\
(this->assertEquals ((expected),\
(expected.Length()),\
(actual),\
(actual.Length()),\
__LINE__,__FILE__,TString(" " #expected " == " #actual " ")))

#define assertCharPtrEqual(expected,actual)\
(this->assertEquals ((expected),\
		(actual),__LINE__,__FILE__,(" " #expected " == " #actual " ")))

// the same macros with an additional message
#define t_assertm(condition,message)\
(this->assertImplementation ((bool)(condition),(#condition),\
	__LINE__, __FILE__,message))

#define assertDoublesEqualm(expected,actual,delta,message)\
(this->assertEquals ((expected),\
		(actual),(delta),__LINE__,__FILE__,message))

#define assertEqualm(expected,actual,message)\
(this->assertEquals ((expected),\
		(actual),__LINE__,__FILE__,message))

#define assertEqualRawm(expected,actual,message)\
(this->assertEquals ((expected),\
(expected.Length()),\
(actual),\
(actual.Length()),\
__LINE__,__FILE__,message))

#define assertCharPtrEqualm(expected,actual,message)\
(this->assertEquals ((expected),\
		(actual),__LINE__,__FILE__,TString(" " #expected " == " #actual " ") << message))

#endif
