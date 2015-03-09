#include "ROSimpleAnythingTest.h"
#include "Anything.h"
#include "IFAObject.h"

class DummyROIFAObj: public IFAObject
{
public:
	DummyROIFAObj(const char *tst = "foo") { }
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) DummyROIFAObj("dummy");
	}
};

void ROSimpleAnythingTest::ConstructorsAndConversions()
{
	EmptyConstructor();
	AnyConstructor();
} // ConstructorsAndConversions

void ROSimpleAnythingTest::AssignmentsAndConversions()
{
} // AssignmentsAndConversions

void ROSimpleAnythingTest::EmptyConstructor()
{
	ROAnything roTest;
	DummyROIFAObj dummyIFA;

	// test standard defaults
//	TODO: error: ISO C++ forbids comparison between pointer and integer [-fpermissive]
//	ASSERT_EQUAL(0L, roTest.AsCharPtr());
	ASSERT_EQUAL(0L, roTest.AsLong(0));
	ASSERT_EQUAL(false, roTest.AsBool(false));
	ASSERT_EQUAL(0L, (long)roTest.AsDouble());
	ASSERT_EQUAL(0L, (long)roTest.AsIFAObject());

	// test custom defaults
	ASSERT_EQUAL("test", roTest.AsCharPtr("test"));
	ASSERT_EQUAL(10L, roTest.AsLong(10));
	ASSERT_EQUAL(0L, roTest.AsLong(0L));
	ASSERT_EQUAL(-10L, roTest.AsLong(-10));
	ASSERT_EQUAL(true, roTest.AsBool(true));
	ASSERT_EQUAL(false, roTest.AsBool(false));
	ASSERT(1.0 < roTest.AsDouble(1.1));
	ASSERT_EQUAL((long)&dummyIFA, (long)roTest.AsIFAObject(&dummyIFA));
}

void ROSimpleAnythingTest::AnyConstructor()
{
	AnyIntConstructor();
	AnyBoolConstructor();
	AnyLongConstructor();
	AnyFloatConstructor();
	AnyDoubleConstructor();
	AnyIFAObjConstructor();
}

void ROSimpleAnythingTest::AnyIntConstructor()
{
	const int cTestVal = 7;
	Anything anyTest(cTestVal);
	ROAnything roTest(anyTest);
	DummyROIFAObj dummyIFA;

	// test standard defaults
	ASSERT_EQUAL("7", roTest.AsCharPtr());
	ASSERT_EQUAL(cTestVal, roTest.AsLong(0));
	ASSERT_EQUAL(true, roTest.AsBool());
	ASSERT_EQUAL(cTestVal, (long)roTest.AsDouble());
	ASSERT_EQUAL(0L, (long)roTest.AsIFAObject());

	// test custom defaults
	ASSERT_EQUAL("7", roTest.AsCharPtr("test"));
	ASSERT_EQUAL(cTestVal, roTest.AsLong(10));
	ASSERT_EQUAL(cTestVal, roTest.AsLong(0L));
	ASSERT_EQUAL(cTestVal, roTest.AsLong(-10));
	ASSERT_EQUAL(true, roTest.AsBool(true));
	ASSERT_EQUAL(true, roTest.AsBool(false));
	ASSERT(1.0 < roTest.AsDouble(1.1));
	ASSERT_EQUAL((long)&dummyIFA, (long)roTest.AsIFAObject(&dummyIFA));
}

void ROSimpleAnythingTest::AnyBoolConstructor()
{
	const bool cTestVal = true;
	Anything anyTest(cTestVal);
	ROAnything roTest(anyTest);
	DummyROIFAObj dummyIFA;

	// test standard defaults
	ASSERT_EQUAL("1", roTest.AsCharPtr());
	ASSERT_EQUAL(cTestVal, roTest.AsLong(0));
	ASSERT_EQUAL(true, roTest.AsBool());
	ASSERT_EQUAL(cTestVal, (long)roTest.AsDouble());
	ASSERT_EQUAL(0L, (long)roTest.AsIFAObject());

	// test custom defaults
	ASSERT_EQUAL("1", roTest.AsCharPtr("test"));
	ASSERT_EQUAL(cTestVal, roTest.AsLong(10));
	ASSERT_EQUAL(cTestVal, roTest.AsLong(0L));
	ASSERT_EQUAL(cTestVal, roTest.AsLong(-10));
	ASSERT_EQUAL(cTestVal, roTest.AsBool(true));
	ASSERT_EQUAL(cTestVal, roTest.AsBool(false));
	ASSERT_EQUAL(cTestVal, (long)roTest.AsDouble(1.1));
	ASSERT_EQUAL((long)&dummyIFA, (long)roTest.AsIFAObject(&dummyIFA));
}

void ROSimpleAnythingTest::AnyLongConstructor()
{
	const long cTestVal = 1234L;
	Anything anyTest(cTestVal);
	ROAnything roTest(anyTest);
	DummyROIFAObj dummyIFA;

	// test standard defaults
	ASSERT_EQUAL("1234", roTest.AsCharPtr());
	ASSERT_EQUAL(cTestVal, roTest.AsLong(0));
	ASSERT_EQUAL(true, roTest.AsBool());
	ASSERT_EQUAL(cTestVal, (long)roTest.AsDouble());
	ASSERT_EQUAL(0L, (long)roTest.AsIFAObject());

	// test custom defaults
	ASSERT_EQUAL("1234", roTest.AsCharPtr("test"));
	ASSERT_EQUAL(cTestVal, roTest.AsLong(10));
	ASSERT_EQUAL(cTestVal, roTest.AsLong(0L));
	ASSERT_EQUAL(cTestVal, roTest.AsLong(-10));
	ASSERT_EQUAL(true, roTest.AsBool(true));
	ASSERT_EQUAL(true, roTest.AsBool(false));
	ASSERT_EQUAL(cTestVal, (long)roTest.AsDouble(1.1));
	ASSERT_EQUAL((long)&dummyIFA, (long)roTest.AsIFAObject(&dummyIFA));
}

void ROSimpleAnythingTest::AnyFloatConstructor()
{
	const float cTestVal = float(22.22);
	Anything anyTest(cTestVal);
	ROAnything roTest(anyTest);
	DummyROIFAObj dummyIFA;

	// test standard defaults
	ASSERT_EQUAL("22.21", roTest.AsString().SubString(0, 5));
	ASSERT_EQUAL((long)cTestVal, roTest.AsLong(0));
	ASSERT_EQUAL(false, roTest.AsBool());
	ASSERT_EQUAL((long)cTestVal, (long)roTest.AsDouble());
	ASSERT_EQUAL(0L, (long)roTest.AsIFAObject());

	// test custom defaults
	ASSERT_EQUAL("22.21", roTest.AsString("test").SubString(0, 5));
	ASSERT_EQUAL((long)cTestVal, roTest.AsLong(10));
	ASSERT_EQUAL((long)cTestVal, roTest.AsLong(0L));
	ASSERT_EQUAL((long)cTestVal, roTest.AsLong(-10));
	ASSERT_EQUAL(true, roTest.AsBool(true));
	ASSERT_EQUAL(false, roTest.AsBool(false));
	ASSERT_EQUAL((long)cTestVal, (long)roTest.AsDouble(1.1));
	ASSERT_EQUAL((long)&dummyIFA, (long)roTest.AsIFAObject(&dummyIFA));
}

void ROSimpleAnythingTest::AnyDoubleConstructor()
{
	const float cTestVal = float(33.33);
	Anything anyTest(cTestVal);
	ROAnything roTest(anyTest);
	DummyROIFAObj dummyIFA;

	// test standard defaults
	ASSERT_EQUAL("33.33", roTest.AsString().SubString(0, 5));
	ASSERT_EQUAL((long)cTestVal, roTest.AsLong(0));
	ASSERT_EQUAL(false, roTest.AsBool());
	ASSERT_EQUAL((long)cTestVal, (long)roTest.AsDouble());
	ASSERT_EQUAL(0L, (long)roTest.AsIFAObject());

	// test custom defaults
	ASSERT_EQUAL("33.33", roTest.AsString("test").SubString(0, 5));
	ASSERT_EQUAL((long)cTestVal, roTest.AsLong(10));
	ASSERT_EQUAL((long)cTestVal, roTest.AsLong(0L));
	ASSERT_EQUAL((long)cTestVal, roTest.AsLong(-10));
	ASSERT_EQUAL(true, roTest.AsBool(true));
	ASSERT_EQUAL(false, roTest.AsBool(false));
	ASSERT_EQUAL((long)cTestVal, (long)roTest.AsDouble(1.1));
	ASSERT_EQUAL((long)&dummyIFA, (long)roTest.AsIFAObject(&dummyIFA));
}

void ROSimpleAnythingTest::AnyIFAObjConstructor()
{
	DummyROIFAObj cTestVal("foo");
	Anything anyTest(&cTestVal);
	ROAnything roTest(anyTest);
	DummyROIFAObj dummyIFA;

	// test standard defaults
	ASSERT_EQUAL("IFAObject", roTest.AsCharPtr());
	ASSERT_EQUAL((long)&cTestVal, roTest.AsLong(0));
	ASSERT_EQUAL(false, roTest.AsBool());
	ASSERT_EQUAL(0L, (long)roTest.AsDouble());
	ASSERT_EQUAL((long)&cTestVal, (long)roTest.AsIFAObject());

	// test custom defaults
	ASSERT_EQUAL("IFAObject", roTest.AsCharPtr("test"));
	ASSERT_EQUAL((long)&cTestVal, roTest.AsLong(10));
	ASSERT_EQUAL((long)&cTestVal, roTest.AsLong(0L));
	ASSERT_EQUAL((long)&cTestVal, roTest.AsLong(-10));
	ASSERT_EQUAL(true, roTest.AsBool(true));
	ASSERT_EQUAL(false, roTest.AsBool(false));
	ASSERT_EQUAL((long)1.1, (long)roTest.AsDouble(1.1));
	ASSERT_EQUAL((long)&cTestVal, (long)roTest.AsIFAObject(&dummyIFA));
}

void ROSimpleAnythingTest::runAllTests(int argc, char const *argv[]) {
	cute::suite s;
	s.push_back(CUTE_SMEMFUN(ROSimpleAnythingTest, ConstructorsAndConversions));
	s.push_back(CUTE_SMEMFUN(ROSimpleAnythingTest, AssignmentsAndConversions));
	cute::ide_listener<> lis;
	cute::makeRunner(lis,argc,argv)(s, "AllTests");
}
