//--- interface include --------------------------------------------------------
#include "AnythingSTLTest.h"

#include "TestSuite.h"

AnythingSTLTest::AnythingSTLTest(TString name)
	: TestCaseType(name)
{
}

void AnythingSTLTest::setUp() {}

void AnythingSTLTest::testSimpleSwap()
{
	Anything first(1L);
	Anything second("two");
	first.swap(second);
	assertEqual(1L, second.AsLong());
	assertEqual("two", first.AsCharPtr());
}
void AnythingSTLTest::testSwapWithDifferentAllocator()
{
	GlobalAllocator ga;
	Anything first(1L, &ga);
	t_assert(&ga == first.GetAllocator());
	Anything second("two"); // normal allocator
	t_assert(Storage::Current() == second.GetAllocator());
	t_assert(&ga != Storage::Current());
	first.swap(second);
	assertEqual(1L, second.AsLong());
	assertEqual("two", first.AsCharPtr());
	t_assert(&ga == second.GetAllocator());
	t_assert(Storage::Current() == first.GetAllocator());
}
void AnythingSTLTest::testFrontBackPushPop()
{
	Anything a;
	a.push_back("two");
	a.push_front(1L);
	a.push_back(3.14);
	assertEqual(3, a.size());
	assertEqual(1L, a.front().AsLong());
	assertDoublesEqual(3.14, a.back().AsDouble(), 0.0000000001);
	a.pop_back();
	assertEqual("two", a.back().AsCharPtr());
	assertEqual(1L, a.front().AsLong());
	a.pop_front();
	assertEqual("two", a.front().AsCharPtr());
	assertEqual("two", a.back().AsCharPtr());
	a.pop_front();
	t_assert(a.empty());
	a.pop_back();
	t_assert(a.empty());
}
static const long rangeinput[] = {1, 2, 3, 4, 5};
const int rangeSize = (sizeof(rangeinput) / sizeof(rangeinput[0]));

void AnythingSTLTest::checkRange(const Anything &a, long n = 0, long lengthOfCopy = rangeSize)
{
	for (int i = 0; i < lengthOfCopy; ++i) {
		assertEqual(rangeinput[i], a[i+n].AsLong());
	}
}

void AnythingSTLTest::testRangeAssign()
{
	Anything a;
	a.assign(rangeinput, rangeinput + rangeSize);
	checkRange(a);
}
void AnythingSTLTest::testRangeCtor()
{
	Anything a(rangeinput, rangeinput + rangeSize);
	checkRange(a);
	Anything b(rangeinput, rangeinput + rangeSize, Storage::Current());
	checkRange(b);
}
const Anything::size_type nOfCopies = 4;
const char *valueToBeCopied = "a test";
void AnythingSTLTest::checkFill(const Anything &a)
{
	assertEqual(nOfCopies, a.size());
	for (int i = 0; i < nOfCopies; ++i) {
		assertEqual(valueToBeCopied, a[i].AsCharPtr());
	}
}
void AnythingSTLTest::checkFillSizeType(const Anything &a)
{
	for (long i = 0; i < nOfCopies; ++i) {
		assertEqual(42L, a[i].AsLong());
	}
}
void AnythingSTLTest::testFillAssign()
{
	Anything a;
	a.assign(nOfCopies, valueToBeCopied);
	checkFill(a);
}
void AnythingSTLTest::testFillAssignWithSizeType()
{
	Anything a;
	a.assign(nOfCopies, Anything::size_type(42L));
	checkFillSizeType(a);
}
void AnythingSTLTest::testFillCtor()
{
	Anything a(nOfCopies, valueToBeCopied);
	checkFill(a);
}
void AnythingSTLTest::testFillCtorWithSizeType()
{
	Anything a(nOfCopies, Anything::size_type(42L));
	checkFillSizeType(a);
	Anything b(nOfCopies, Anything::size_type(42L), Storage::Current());
	checkFillSizeType(b);
}
void AnythingSTLTest::testSimpleInsertToEmptyAny()
{
	Anything a;
	a.insert(a.begin(), 42L);
	assertEqual(42L, a[0L].AsLong());
	a.clear();
	t_assert(a.IsNull());
	a.insert(a.end(), "test");
	assertEqual("test", a[0L].AsCharPtr());
}
void AnythingSTLTest::checkSimpleInsert(const Anything &a, const char *m)
{
	assertEqualm(42L, a[0].AsLong(), m);
	assertEqualm("test", a[1].AsCharPtr(), m);
	assertEqualm(2, a.size(), m);
}

void AnythingSTLTest::testSimpleInsertAtEnd()
{
	Anything a(42L);
	a.insert(a.end(), "test");
	checkSimpleInsert(a, "testSimpleInsertAtEnd");
}
void AnythingSTLTest::testSimpleInsertAtBegin()
{
	Anything a("test");
	a.insert(a.begin(), 42L);
	checkSimpleInsert(a, "testSimpleInsertAtBegin");
}
void AnythingSTLTest::checkInsertInArray(const Anything &a, long testpos, const char *msg, long n)
{
	TString m(msg);
	m.Append(" pos: ");
	m.Append(testpos);
	assertEqualm(rangeSize + n, a.size(), m);
	for (long i = 0L; i < n; ++i) {
		assertEqualm(42L, a[testpos+i].AsLong(), m);
	}
	for (long i = 0L; i < testpos; ++i) {
		assertEqualm(rangeinput[i], a[i].AsLong(), m);
	}
	for (long i = testpos; i < rangeSize; ++i) {
		assertEqualm(rangeinput[i], a[i+n].AsLong(), m);
	}
}
void AnythingSTLTest::testSimpleInsertInArray()
{
	for (long testpos = 0; testpos <= rangeSize; ++testpos) {
		Anything a(rangeinput, rangeinput + rangeSize);
		a.insert(a.begin() + testpos, 42L);
		checkInsertInArray(a, testpos, "SimpleInsert");
	}
}
Anything makeAnyWithKeys()
{
	Anything a;
	a["eins"] = 1L;
	a["zwei"] = 2.0;
	a.Append("drei");
	a["vier"] = "vier";
	return a;
}
void AnythingSTLTest::checkInsertWithKeys(const Anything &a, const long testpos, const char *m, const long n)
{
	TString msg(m);
	msg.Append(":run ");
	msg.Append(testpos);
	long einspos = (testpos > 0 ? 0L : n);
	assertEqualm("eins", a.SlotName(einspos), msg);
	assertEqualm(1L, a["eins"].AsLong(), msg);
	assertDoublesEqualm(2.0, a["zwei"].AsDouble(), 0.000001, msg);
	long dreipos = a.size() - (testpos < 3 ? 2L : 2 + n);
	assertEqualm("drei", a[dreipos].AsCharPtr(), msg);
	long vierpos = a.size() - (testpos < 4 ? 1L : 1L + n);
	assertEqualm("vier", a[vierpos].AsCharPtr(), msg);
	assertEqualm("vier", a.SlotName(vierpos), msg);
}
void AnythingSTLTest::testSimpleInsertWithKeys()
{
	for (long testpos = 0; testpos <= 4; ++testpos) {
		Anything a(makeAnyWithKeys());
		a.insert(a.begin() + testpos, 42L);
		checkInsertWithKeys(a, testpos, "SimpleInsert");
		assertEqual(42L, a[testpos].AsLong());
	}
}
void AnythingSTLTest::testFillInsertToEmptyAny()
{
	Anything a;
	const Anything::size_type n = 4;
	const char *content = "foo";
	a.insert(a.begin(), n, content);
	assertEqual(n, a.size());
	for (long i = 0; i < n; ++i) {
		assertEqual(content, a[i].AsCharPtr());
	}
}
void AnythingSTLTest::testFillInsertInArray()
{
	for (long testpos = 0; testpos <= rangeSize; ++testpos) {
		Anything a(rangeinput, rangeinput + rangeSize);
		const long n = 3;
		a.insert(a.begin() + testpos, n, 42L);
		checkInsertInArray(a, testpos, "FillInsert", n);
	}
}
void AnythingSTLTest::testFillInsertWithKeys()
{
	for (long testpos = 0; testpos <= 4; ++testpos) {
		const long n = 5;
		Anything a(makeAnyWithKeys());
		a.insert(a.begin() + testpos, n, 42L);
		checkInsertWithKeys(a, testpos, "FillInsert", n);
		for (long i = testpos; i < testpos + n; ++i) {
			assertEqual(42L, a[i].AsLong());
		}
	}
}
void AnythingSTLTest::testFillInsertZero()
{
	Anything a;
	a.insert(a.begin(), 0, 42L);
	assertAnyEqual(Anything(), a);
	a.insert(a.begin(), 1L);
	assertEqual(1, a.size());
	a.insert(a.begin(), 0, 42L); // no op!
	assertEqual(1L, a.AsLong());
	a["two"] = "zwei";
	a.insert(a.begin() + 1, 0, 42L); // no op
	assertEqual(2, a.size());
	assertEqual("zwei", a[1L].AsCharPtr());
}

void AnythingSTLTest::testIterInsertToEmptyAny()
{
	Anything a;
	a.insert(a.begin(), rangeinput, rangeinput + rangeSize);
	checkRange(a);
}
void AnythingSTLTest::testIterInsertInArray()
{
	for (long testpos = 0; testpos <= rangeSize; ++testpos) {
		Anything a(rangeinput, rangeinput + rangeSize);
		const long lengthOfInsertion = rangeSize - 1;
		a.insert(a.begin() + testpos, rangeinput, rangeinput + lengthOfInsertion);
		TString m(" pos: ");
		m.Append(testpos);
		assertEqualm(rangeSize + lengthOfInsertion, a.size(), m);
		checkRange(a, testpos, lengthOfInsertion);
		for (long i = 0L; i < lengthOfInsertion; ++i) {
			assertEqualm(rangeinput[i], a[testpos+i].AsLong(), m);
		}
		for (long i = 0L; i < testpos; ++i) {
			assertEqualm(rangeinput[i], a[i].AsLong(), m);
		}
		for (long i = testpos; i < rangeSize; ++i) {
			assertEqualm(rangeinput[i], a[i+lengthOfInsertion].AsLong(), m);
		}
	}
}
void AnythingSTLTest::testIterInsertWithKeys()
{
	for (long testpos = 0; testpos <= 4; ++testpos) {
		const long lengthOfInsertion = rangeSize - 1;
		Anything a(makeAnyWithKeys());
		a.insert(a.begin() + testpos, rangeinput, rangeinput + lengthOfInsertion);
		checkInsertWithKeys(a, testpos, "FillInsert", lengthOfInsertion);
		checkRange(a, testpos, lengthOfInsertion);
	}
}
void AnythingSTLTest::testIterInsertZero()
{
	Anything a;
	a.insert(a.begin(), rangeinput, rangeinput); // no op!
	assertAnyEqual(Anything(), a);
	a.insert(a.begin(), rangeinput, rangeinput + 1);
	assertEqual(1, a.size());
	a.insert(a.begin(), rangeinput, rangeinput); // no op!
	assertEqual(1L, a.AsLong());
	a["two"] = "zwei";
	a.insert(a.begin() + 1, rangeinput, rangeinput); // no op
	assertEqual(2, a.size());
	assertEqual("zwei", a[1L].AsCharPtr());
}
Test *AnythingSTLTest::suite ()
{
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, AnythingSTLTest, testSimpleSwap);
	ADD_CASE(testSuite, AnythingSTLTest, testSwapWithDifferentAllocator);
	ADD_CASE(testSuite, AnythingSTLTest, testFrontBackPushPop);
	ADD_CASE(testSuite, AnythingSTLTest, testRangeAssign);
	ADD_CASE(testSuite, AnythingSTLTest, testFillAssign);
	ADD_CASE(testSuite, AnythingSTLTest, testFillAssignWithSizeType);
	ADD_CASE(testSuite, AnythingSTLTest, testRangeCtor);
	ADD_CASE(testSuite, AnythingSTLTest, testFillCtor);
	ADD_CASE(testSuite, AnythingSTLTest, testFillCtorWithSizeType);
	ADD_CASE(testSuite, AnythingSTLTest, testSimpleInsertToEmptyAny);
	ADD_CASE(testSuite, AnythingSTLTest, testSimpleInsertAtEnd);
	ADD_CASE(testSuite, AnythingSTLTest, testSimpleInsertAtBegin);
	ADD_CASE(testSuite, AnythingSTLTest, testSimpleInsertInArray);
	ADD_CASE(testSuite, AnythingSTLTest, testSimpleInsertWithKeys);
	ADD_CASE(testSuite, AnythingSTLTest, testFillInsertToEmptyAny);
	ADD_CASE(testSuite, AnythingSTLTest, testFillInsertInArray);
	ADD_CASE(testSuite, AnythingSTLTest, testFillInsertWithKeys);
	ADD_CASE(testSuite, AnythingSTLTest, testFillInsertZero);
	ADD_CASE(testSuite, AnythingSTLTest, testIterInsertToEmptyAny);
	ADD_CASE(testSuite, AnythingSTLTest, testIterInsertInArray);
	ADD_CASE(testSuite, AnythingSTLTest, testIterInsertWithKeys);
	ADD_CASE(testSuite, AnythingSTLTest, testIterInsertZero);

	return testSuite;
}
