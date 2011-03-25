#include "AnythingIteratorTest.h"
#include "TestSuite.h"

#include "Dbg.h"

AnythingIteratorTest::AnythingIteratorTest (TString tname)
	: TestCaseType(tname)
{
}

void AnythingIteratorTest::testEmptyAnythingBegin()
{
	Anything a;
	t_assert(a.begin() == a.end());
	const Anything &b = a;
	t_assert(b.begin() == b.end());
}
void AnythingIteratorTest::testSimpleAnythingBegin()
{
	Anything a(1L);
	t_assert(a.begin() != a.end());
	const Anything &b = a;
	t_assert(b.begin() != b.end());
}
void AnythingIteratorTest::testSimpleAnythingDeref()
{
	Anything a(1L);
	t_assert(*a.begin() == a);
	const Anything &b = a;
	t_assert(*b.begin() == b);
}
void AnythingIteratorTest::testSimpleAnythingIncrement()
{
	Anything a(1L);
	Anything_iterator ai = a.begin();
	t_assert(++ai == a.end());
	t_assert(ai != a.begin());
	const Anything &b = a;
	Anything_const_iterator bi = b.begin();
	t_assert(++bi == b.end());
	t_assert(bi != b.begin());
}
void AnythingIteratorTest::testSimpleAnythingDecrement()
{
	Anything a(1L);
	Anything_iterator ai = a.end();
	t_assert(--ai == a.begin());
	t_assert(ai != a.end());
	const Anything &b = a;
	Anything_const_iterator bi = b.end();
	t_assert(--bi == b.begin());
	t_assert(bi != b.end());
}
void AnythingIteratorTest::testSimpleAnythingAssignment()
{
	Anything a(1L);
	*a.begin() = 5L;
	t_assert(*a.begin() == Anything(5L));

}
void AnythingIteratorTest::testAnythingIteration()
{
	Anything a;
	a.Append(1L);
	a.Append(2L);
	a.Append(3L);
	Anything_iterator ai = a.begin();
	for (long i = 0L; i < a.GetSize(); ++i, ++ai) {
		t_assert(*ai == a[i]);
	}
	t_assert(ai == a.end());
	const Anything &b = a;
	Anything_const_iterator bi = b.begin();
	for (long i = 0L; i < b.GetSize(); ++i, ++bi) {
		t_assert(*bi == b[i]);
	}
	t_assert(bi == b.end());
}
void AnythingIteratorTest::testSimpleAnythingIncDec()
{
	Anything a(1L);
	Anything_iterator ai = a.begin();
	Anything_iterator ai1 = ++ai;
	t_assert(ai == ai1);
	t_assert(--ai == a.begin());
	t_assert(ai1 == a.end());
	t_assert(ai != ai1);
	t_assert(--ai1 == ai);
	const Anything &b = a;
	Anything_const_iterator bi = b.begin();
	Anything_const_iterator bi1 = ++bi;
	t_assert(bi == bi1);
	t_assert(--bi == b.begin());
	t_assert(bi1 == b.end());
	t_assert(bi1 != bi);
	t_assert(--bi == bi);
}
void AnythingIteratorTest::testAnythingIndex()
{
	assertEqual(5, fAny5.GetSize());
	Anything_iterator it = fAny5.begin();
	assertAnyEqual(fAny5[0], it[0]);
	assertAnyEqual(fAny5[4], it[4]);
	++it;
	assertAnyEqual(fAny5[4], it[3]);
	++it;
	assertAnyEqual(fAny5[4], it[2]);
	assertAnyEqual(fAny5.begin()[2], *it);
	assertAnyEqual(*fAny5.begin(), it[-2]);
	assertAnyEqual(fAny5.end()[-3], *it);
	Anything_const_iterator cit = fAny5.begin();
	assertAnyEqual(fAny5[0], cit[0]);
	assertAnyEqual(fAny5[4], cit[4]);
	++cit;
	assertAnyEqual(fAny5[4], cit[3]);
	++cit;
	assertAnyEqual(fAny5[4], cit[2]);
	assertAnyEqual(fAny5.begin()[2], *cit);
	assertAnyEqual(*fAny5.begin(), cit[-2]);
	assertAnyEqual(fAny5.end()[-3], *cit);
}
void AnythingIteratorTest::testIteratorSubstract()
{
	assertEqual(fAny5.GetSize(), fAny5.end() - fAny5.begin());
	const Anything &a5 = fAny5;
	assertEqual(a5.GetSize(), a5.end() - a5.begin());
	Anything a;
	const Anything &b(a);
	assertEqual(a.GetSize(), a.end() - a.begin());
	assertEqual(b.GetSize(), b.end() - b.begin());
	a.Append(1L);
	assertEqual(a.GetSize(), a.end() - a.begin());
	assertEqual(b.GetSize(), b.end() - b.begin());
	a.Append(2L);
	assertEqual(a.GetSize(), a.end() - a.begin());
	assertEqual(b.GetSize(), b.end() - b.begin());
	Anything_iterator ai = a.begin();
	++ai;
	assertEqual(1, ai - a.begin());
	assertEqual(-1, a.begin() - ai);
	Anything_const_iterator bi = b.begin();
	++bi;
	assertEqual(1, bi - b.begin());
	assertEqual(-1, b.begin() - bi);
}
void AnythingIteratorTest::testIteratorIntAdd()
{
	Anything_iterator ai = fAny5.begin();
	assertAnyEqual(*(ai + 4), fAny5[4]);
	++ai;
	t_assert(ai == (fAny5.begin() + 1));
	t_assert(ai == (fAny5.end() + (-4)));
	Anything_const_iterator bi = fAny5.begin();
	assertAnyEqual(*(bi + 4), fAny5[4]);
	++bi;
	t_assert(bi == (fAny5.begin() + 1));
	t_assert(bi == (fAny5.end() + (-4)));
}
void AnythingIteratorTest::testIteratorIntSubstract()
{
	Anything_iterator ai = fAny5.end();
	assertAnyEqual(*(ai - 4), fAny5[1]);
	--ai;
	t_assert(ai == (fAny5.end() - 1));
	t_assert(ai == (fAny5.begin() - (-4)));
	Anything_const_iterator bi = fAny5.end();
	assertAnyEqual(*(bi - 4), fAny5[1]);
	--bi;
	t_assert(bi == (fAny5.end() - 1));
	t_assert(bi == (fAny5.begin() - (-4)));
}
void AnythingIteratorTest::testIteratorCompare()
{
	Anything a;
	t_assert(a.begin() <= a.end());
	t_assert(a.end() >= a.begin());
	t_assert(a.begin() <= a.begin());
	t_assert(a.begin() >= a.begin());
	t_assert(!(a.begin() < a.begin()));
	t_assert(!(a.begin() > a.begin()));
	const Anything &b(a);
	t_assert(b.begin() <= b.end());
	t_assert(b.end() >= b.begin());
	t_assert(b.begin() <= b.begin());
	t_assert(b.begin() >= b.begin());
	t_assert(!(b.begin() < b.begin()));
	t_assert(!(b.begin() > b.begin()));
	a.Append(1L);
	t_assert(a.begin() < a.end());
	t_assert(a.end() > a.begin());
	t_assert(a.begin() <= a.end());
	t_assert(a.end() >= a.begin());
	t_assert(a.begin() <= a.begin());
	t_assert(a.begin() >= a.begin());
	t_assert(!(a.begin() < a.begin()));
	t_assert(!(a.begin() > a.begin()));
	//- const_iterator
	t_assert(b.begin() < b.end());
	t_assert(b.end() > b.begin());
	t_assert(b.begin() <= b.end());
	t_assert(b.end() >= b.begin());
	t_assert(b.begin() <= b.begin());
	t_assert(b.begin() >= b.begin());
	t_assert(!(b.begin() < b.begin()));
	t_assert(!(b.begin() > b.begin()));

	t_assert(fAny5.end() > fAny5.begin());
}
// the following 2 tests should belong to AnythingSTLTest,
// but for convenience of setUp with fAny5 it is here
void AnythingIteratorTest::testAnythingSingleErase()
{
	Anything a = fAny5;
	assertEqual(5, a.end() - a.begin());
	t_assert(a.begin() == a.erase(a.begin()));
	assertEqual(4, a.end() - a.begin());
	t_assert(a.end() == a.erase(a.end()));// cannot delete past the end
	assertEqual(4, a.end() - a.begin());
	Anything_iterator aItEnd(a.end());
	t_assert( aItEnd - 1 == a.erase( aItEnd - 1 ) );
	assertEqual(3, a.end() - a.begin());
	t_assert(a.begin() + 1 == a.erase(a.begin() + 1));
	assertEqual(2, a.end() - a.begin());
	a.clear();
	assertEqual(0, a.end() - a.begin());
	t_assert(a.empty());
}
void AnythingIteratorTest::testAnythingRangeErase()
{
	Anything a = fAny5;
	assertEqual(5, a.end() - a.begin());
	t_assert(a.end() == a.erase(a.end(), a.end()));
	assertEqual(5, a.end() - a.begin());
	t_assert(a.begin() == a.erase(a.begin(), a.begin()));
	assertEqual(5, a.end() - a.begin());
	// until now, no-ops
	t_assert(a.begin() == a.erase(a.begin(), a.begin() + 1));
	assertEqual(4, a.end() - a.begin());

}
void AnythingIteratorTest::setUp ()
{
	fAny5.clear();
	for (long i = 1; i <= 5; ++i) {
		fAny5.Append(i);
	}
}
Test *AnythingIteratorTest::suite ()
{
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, AnythingIteratorTest, testEmptyAnythingBegin);
	ADD_CASE(testSuite, AnythingIteratorTest, testSimpleAnythingBegin);
	ADD_CASE(testSuite, AnythingIteratorTest, testSimpleAnythingDeref);
	ADD_CASE(testSuite, AnythingIteratorTest, testSimpleAnythingIncrement);
	ADD_CASE(testSuite, AnythingIteratorTest, testSimpleAnythingDecrement);
	ADD_CASE(testSuite, AnythingIteratorTest, testSimpleAnythingIncDec);
	ADD_CASE(testSuite, AnythingIteratorTest, testSimpleAnythingAssignment);
	ADD_CASE(testSuite, AnythingIteratorTest, testAnythingIteration);
	ADD_CASE(testSuite, AnythingIteratorTest, testAnythingIndex);
	ADD_CASE(testSuite, AnythingIteratorTest, testIteratorSubstract);
	ADD_CASE(testSuite, AnythingIteratorTest, testIteratorIntAdd);
	ADD_CASE(testSuite, AnythingIteratorTest, testIteratorIntSubstract);
	ADD_CASE(testSuite, AnythingIteratorTest, testIteratorCompare);
	ADD_CASE(testSuite, AnythingIteratorTest, testAnythingSingleErase);
	ADD_CASE(testSuite, AnythingIteratorTest, testAnythingRangeErase);
	return testSuite;
}
