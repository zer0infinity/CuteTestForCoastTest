/*
 * Copyright (c) 2006, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "TypeTraitsTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "ITOTypeTraits.h"

//--- standard modules used ----------------------------------------------------
#include "ITOString.h"
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------
#include <typeinfo>

using namespace std;

//---- TypeTraitsTest ----------------------------------------------------------------
TypeTraitsTest::TypeTraitsTest(TString tstrName)
	: TestCaseType(tstrName)
{
	StartTrace(TypeTraitsTest.Ctor);
}

TypeTraitsTest::~TypeTraitsTest()
{
	StartTrace(TypeTraitsTest.Dtor);
}

void TypeTraitsTest::TraitsTest()
{
	StartTrace(TypeTraitsTest.TraitsTest);
	{
		typedef String TestType;
		t_assertm(Loki::TypeTraits<TestType>::isPointer == false, "expected non-pointer type");
		t_assertm(Loki::TypeTraits<TestType>::isReference == false, "expected non-reference type");
		t_assertm(Loki::TypeTraits<TestType>::isConst == false, "expected non-const type");
		t_assertm(typeid(Loki::TypeTraits<TestType>::PointeeType) == typeid(NullType), "expected NullType");
		t_assertm(typeid(Loki::TypeTraits<TestType>::ReferredType) == typeid(NullType), "expected NullType");
		t_assertm(typeid(Loki::TypeTraits<TestType>::NonConstType) == typeid(String), "expected String-type");
		t_assertm( Loki::TypeTraits< Loki::TypeTraits<TestType>::NonConstType >::isPointer == false, "expected non String-ptr-type");
		t_assertm(typeid(Loki::TypeTraits<TestType>::PlainType) == typeid(String), "expected String-type");
		t_assertm( Loki::TypeTraits< Loki::TypeTraits<TestType>::PlainType >::isReference == false, "expected plain String-type");
	}
	{
		typedef const String TestType;
		t_assertm(Loki::TypeTraits<TestType>::isPointer == false, "expected non-pointer type");
		t_assertm(Loki::TypeTraits<TestType>::isReference == false, "expected non-reference type");
		t_assertm(Loki::TypeTraits<TestType>::isConst == true, "expected const type");
		t_assertm(typeid(Loki::TypeTraits<TestType>::PointeeType) == typeid(NullType), "expected NullType");
		t_assertm(typeid(Loki::TypeTraits<TestType>::ReferredType) == typeid(NullType), "expected NullType");
		t_assertm(typeid(Loki::TypeTraits<TestType>::NonConstType) == typeid(String), "expected String-type");
		t_assertm( Loki::TypeTraits< Loki::TypeTraits<TestType>::NonConstType >::isPointer == false, "expected non String-ptr-type");
		t_assertm(typeid(Loki::TypeTraits<TestType>::PlainType) == typeid(String), "expected String-type");
		t_assertm( Loki::TypeTraits< Loki::TypeTraits<TestType>::PlainType >::isReference == false, "expected plain String-type");
	}
	{
		typedef String *TestType;
		t_assertm(Loki::TypeTraits<TestType>::isPointer == true, "expected pointer type");
		t_assertm(Loki::TypeTraits<TestType>::isReference == false, "expected non-reference type");
		t_assertm(Loki::TypeTraits<TestType>::isConst == false, "expected non-const type");
		t_assertm(typeid(Loki::TypeTraits<TestType>::PointeeType) == typeid(String), "expected String-type");
		t_assertm(typeid(Loki::TypeTraits<TestType>::ReferredType) == typeid(NullType), "expected NullType");
		t_assertm(typeid(Loki::TypeTraits<TestType>::NonConstType) == typeid(String *), "expected String-ptr-type");
		t_assertm( Loki::TypeTraits< Loki::TypeTraits<TestType>::NonConstType >::isPointer == true, "expected String-ptr-type");
		t_assertm(typeid(Loki::TypeTraits<TestType>::PlainType) == typeid(String), "expected String-type");
		t_assertm( Loki::TypeTraits< Loki::TypeTraits<TestType>::PlainType >::isReference == false, "expected plain String-type");
	}
	{
		typedef const String *TestType;
		t_assertm(Loki::TypeTraits<TestType>::isPointer == true, "expected pointer type");
		t_assertm(Loki::TypeTraits<TestType>::isReference == false, "expected non-reference type");
		t_assertm(Loki::TypeTraits<TestType>::isConst == true, "expected const type");
		t_assertm(typeid(Loki::TypeTraits<TestType>::PointeeType) == typeid(String), "expected String-type");
		t_assertm(typeid(Loki::TypeTraits<TestType>::ReferredType) == typeid(NullType), "expected NullType");
		t_assertm(typeid(Loki::TypeTraits<TestType>::NonConstType) == typeid(String *), "expected String-ptr-type");
		t_assertm( Loki::TypeTraits< Loki::TypeTraits<TestType>::NonConstType >::isPointer == true, "expected String-ptr-type");
		t_assertm(typeid(Loki::TypeTraits<TestType>::PlainType) == typeid(String), "expected String-type");
		t_assertm( Loki::TypeTraits< Loki::TypeTraits<TestType>::PlainType >::isReference == false, "expected plain String-type");
	}
	{
		typedef String &TestType;
		t_assertm(Loki::TypeTraits<TestType>::isPointer == false, "expected non-pointer type");
		t_assertm(Loki::TypeTraits<TestType>::isReference == true, "expected reference type");
		t_assertm(Loki::TypeTraits<TestType>::isConst == false, "expected non-const type");
		t_assertm(typeid(Loki::TypeTraits<TestType>::PointeeType) == typeid(NullType), "expected NullType");
		t_assertm(typeid(Loki::TypeTraits<TestType>::ReferredType) == typeid(String), "expected String-type");
		t_assertm(typeid(Loki::TypeTraits<TestType>::NonConstType) == typeid(String &), "expected String-ref-type");
		t_assertm( Loki::TypeTraits< Loki::TypeTraits<TestType>::NonConstType >::isPointer == false, "expected non String-ptr-type");
		t_assertm(typeid(Loki::TypeTraits<TestType>::PlainType) == typeid(String), "expected String-type");
		t_assertm( Loki::TypeTraits< Loki::TypeTraits<TestType>::PlainType >::isReference == false, "expected plain String-type");
	}
	{
		typedef const String &TestType;
		t_assertm(Loki::TypeTraits<TestType>::isPointer == false, "expected non-pointer type");
		t_assertm(Loki::TypeTraits<TestType>::isReference == true, "expected reference type");
		t_assertm(Loki::TypeTraits<TestType>::isConst == true, "expected const type");
		t_assertm(typeid(Loki::TypeTraits<TestType>::PointeeType) == typeid(NullType), "expected NullType");
		t_assertm(typeid(Loki::TypeTraits<TestType>::ReferredType) == typeid(String), "expected String-type");
		t_assertm(typeid(Loki::TypeTraits<TestType>::NonConstType) == typeid(String &), "expected String-ref-type");
		t_assertm( Loki::TypeTraits< Loki::TypeTraits<TestType>::NonConstType >::isPointer == false, "expected non String-ptr-type");
		t_assertm(typeid(Loki::TypeTraits<TestType>::PlainType) == typeid(String), "expected String-type");
		t_assertm( Loki::TypeTraits< Loki::TypeTraits<TestType>::PlainType >::isReference == false, "expected plain String-type");
	}
}

template
<
typename InnerType
>
class ClassTraits
{
public:
	typedef typename Loki::TypeTraits<InnerType>::ConstPlainTypeRef ConstPlainTypeRef;
	typedef typename Loki::TypeTraits<InnerType>::PlainTypePtr PlainTypePtr;

	ClassTraits(ConstPlainTypeRef aParam)
		: fField(aParam) {
		StartTrace1(ClassTraits.ClassTraits, "ConstPlainTypeRef");
		SomeMethod(aParam);
	}
	ClassTraits(PlainTypePtr pParam)
		: fField(pParam) {
		StartTrace1(ClassTraits.ClassTraits, "PlainTypePtr");
		SomeMethod(pParam);
	}
	~ClassTraits() {};

	void SomeMethod(ConstPlainTypeRef aParam) {
		StartTrace1(ClassTraits.SomeMethod, "ConstPlainTypeRef");
	}

	void SomeMethod(PlainTypePtr pParam) {
		StartTrace1(ClassTraits.SomeMethod, "PlainTypePtr");
	}

	const type_info &GetTypeInfo() const {
		return typeid(fField);
	}

private:
	InnerType fField;
};

void TypeTraitsTest::ClassTraitsTest()
{
	StartTrace(TypeTraitsTest.ClassTraitsTest);
	{
		typedef const String &TestType;
		String aTestString;
		ClassTraits<TestType> aClass(aTestString);
		t_assert(aClass.GetTypeInfo() == typeid(TestType));
	}
	{
		typedef String *TestType;
		String aTestString;
		ClassTraits<TestType> aClass(&aTestString);
		t_assert(aClass.GetTypeInfo() == typeid(TestType));
	}
}

// builds up a suite of testcases, add a line for each testmethod
Test *TypeTraitsTest::suite ()
{
	StartTrace(TypeTraitsTest.suite);
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, TypeTraitsTest, TraitsTest);
	ADD_CASE(testSuite, TypeTraitsTest, ClassTraitsTest);
	return testSuite;
}
