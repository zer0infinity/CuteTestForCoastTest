#include "StringIteratorTest.h"
#include "TestSuite.h"

void StringIteratorTest::testEmptyStringBegin() {
	String a;
	t_assert(a.begin() == a.end());
	const String &b = a;
	t_assert(b.begin() == b.end());
}
void StringIteratorTest::testSimpleStringBegin() {
	String a("bla");
	t_assert(a.begin() != a.end());
	const String &b = a;
	t_assert(b.begin() != b.end());
}
void StringIteratorTest::testSimpleStringDeref() {
	String a("bla");
	t_assert(*a.begin() == a[0L]);
	const String &b = a;
	t_assert(*b.begin() == b[0L]);
}
void StringIteratorTest::testSimpleStringIncrement() {
	String a("b");
	String_iterator ai = a.begin();
	t_assert(++ai == a.end());
	t_assert(ai != a.begin());
	const String &b = a;
	String_const_iterator bi = b.begin();
	t_assert(++bi == b.end());
	t_assert(bi != b.begin());
}
void StringIteratorTest::testSimpleStringDecrement() {
	String a("b");
	String_iterator ai = a.end();
	t_assert(--ai == a.begin());
	t_assert(ai != a.end());
	const String &b = a;
	String_const_iterator bi = b.end();
	t_assert(--bi == b.begin());
	t_assert(bi != b.end());
}
void StringIteratorTest::testSimpleStringAssignment() {
	String a("bla");
	*a.begin() = 'x';
	String::const_iterator ai = a.begin();
	t_assert(*ai == 'x');
	t_assert(*++ai == 'l');
	t_assert(*++ai == 'a');
	t_assert(*++ai == '\0');
}
void StringIteratorTest::testStringIteration() {
	String a;
	a.Append(1L);
	a.Append(2L);
	a.Append(3L);
	String_iterator ai = a.begin();
	for (long i = 0L; i < a.size(); ++i, ++ai) {
		t_assert(*ai == a[i]);
	}
	t_assert(ai == a.end());
	const String &b = a;
	String_const_iterator bi = b.begin();
	for (long i = 0L; i < b.size(); ++i, ++bi) {
		t_assert(*bi == b[i]);
	}
	t_assert(bi == b.end());
}
void StringIteratorTest::testSimpleStringIncDec() {
	String a("b");
	String_iterator ai = a.begin();
	String_iterator ai1 = ++ai;
	t_assert(ai == ai1);
	t_assert(--ai == a.begin());
	t_assert(ai1 == a.end());
	t_assert(ai != ai1);
	t_assert(--ai1 == ai);
	const String &b = a;
	String_const_iterator bi = b.begin();
	String_const_iterator bi1 = ++bi;
	t_assert(bi == bi1);
	t_assert(--bi == b.begin());
	t_assert(bi1 == b.end());
	t_assert(bi1 != bi);
	t_assert(--bi == bi);
}
void StringIteratorTest::testStringIndex() {
	assertEqual(5, fStr5.size());
	String_iterator it = fStr5.begin();
	assertEqual(fStr5[0], it[0]);
	assertEqual(fStr5[4], it[4]);
	++it;
	assertEqual(fStr5[4], it[3]);
	++it;
	assertEqual(fStr5[4], it[2]);
	assertEqual(fStr5.begin()[2], *it);
	assertEqual(*fStr5.begin(), it[-2]);
	assertEqual(fStr5.end()[-3], *it);
	String_const_iterator cit = fStr5.begin();
	assertEqual(fStr5[0], cit[0]);
	assertEqual(fStr5[4], cit[4]);
	++cit;
	assertEqual(fStr5[4], cit[3]);
	++cit;
	assertEqual(fStr5[4], cit[2]);
	assertEqual(fStr5.begin()[2], *cit);
	assertEqual(*fStr5.begin(), cit[-2]);
	assertEqual(fStr5.end()[-3], *cit);
}
void StringIteratorTest::testIteratorSubstract() {
	assertEqual(fStr5.size(), fStr5.end() - fStr5.begin());
	const String &a5 = fStr5;
	assertEqual(a5.size(), a5.end() - a5.begin());
	String a;
	const String &b(a);
	assertEqual(a.size(), a.end() - a.begin());
	assertEqual(b.size(), b.end() - b.begin());
	a.Append(1L);
	assertEqual(a.size(), a.end() - a.begin());
	assertEqual(b.size(), b.end() - b.begin());
	a.Append(2L);
	assertEqual(a.size(), a.end() - a.begin());
	assertEqual(b.size(), b.end() - b.begin());
	String_iterator ai = a.begin();
	++ai;
	assertEqual(1, ai - a.begin());
	assertEqual(-1, a.begin() - ai);
	String_const_iterator bi = b.begin();
	++bi;
	assertEqual(1, bi - b.begin());
	assertEqual(-1, b.begin() - bi);
}
void StringIteratorTest::testIteratorIntAdd() {
	String_iterator ai = fStr5.begin();
	assertEqual(fStr5[4], *(ai + 4));
	++ai;
	t_assert(ai == (fStr5.begin() + 1));
	t_assert(ai == (fStr5.end() + (-4)));
	String_const_iterator bi = fStr5.begin();
	assertEqual(fStr5[4], *(bi + 4));
	++bi;
	t_assert(bi == (fStr5.begin() + 1));
	t_assert(bi == (fStr5.end() + (-4)));
}
void StringIteratorTest::testIteratorIntSubstract() {
	String_iterator ai = fStr5.end();
	assertEqual(fStr5[1], *(ai - 4));
	--ai;
	t_assert(ai == (fStr5.end() - 1));
	t_assert(ai == (fStr5.begin() - (-4)));
	String_const_iterator bi = fStr5.end();
	assertEqual(fStr5[1], *(bi - 4));
	--bi;
	t_assert(bi == (fStr5.end() - 1));
	t_assert(bi == (fStr5.begin() - (-4)));
}
void StringIteratorTest::testIteratorCompare() {
	String a;
	t_assert(a.begin() <= a.end());
	t_assert(a.end() >= a.begin());
	t_assert(a.begin() <= a.begin());
	t_assert(a.begin() >= a.begin());
	t_assert(!(a.begin() < a.begin()));
	t_assert(!(a.begin() > a.begin()));
	const String &b(a);
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

	t_assert(fStr5.end() > fStr5.begin());
}
void StringIteratorTest::setUp() {
	fStr5.clear();
	for (long i = 1; i <= 5; ++i) {
		fStr5.Append(i);
	}
}

Test *StringIteratorTest::suite() {
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, StringIteratorTest, testEmptyStringBegin);
	ADD_CASE(testSuite, StringIteratorTest, testSimpleStringBegin);
	ADD_CASE(testSuite, StringIteratorTest, testSimpleStringDeref);
	ADD_CASE(testSuite, StringIteratorTest, testSimpleStringIncrement);
	ADD_CASE(testSuite, StringIteratorTest, testSimpleStringDecrement);
	ADD_CASE(testSuite, StringIteratorTest, testSimpleStringIncDec);
	ADD_CASE(testSuite, StringIteratorTest, testSimpleStringAssignment);
	ADD_CASE(testSuite, StringIteratorTest, testStringIteration);
	ADD_CASE(testSuite, StringIteratorTest, testStringIndex);
	ADD_CASE(testSuite, StringIteratorTest, testIteratorSubstract);
	ADD_CASE(testSuite, StringIteratorTest, testIteratorIntAdd);
	ADD_CASE(testSuite, StringIteratorTest, testIteratorIntSubstract);
	ADD_CASE(testSuite, StringIteratorTest, testIteratorCompare);
	return testSuite;
}
