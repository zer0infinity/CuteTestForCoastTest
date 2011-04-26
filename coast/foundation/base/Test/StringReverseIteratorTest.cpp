#include "StringReverseIteratorTest.h"
#include "TestSuite.h"

void StringReverseIteratorTest::testEmptyStringBegin() {
	String a;
	t_assert(a.rbegin() == a.rend());
	const String &b = a;
	t_assert(b.rbegin() == b.rend());
}
void StringReverseIteratorTest::testSimpleStringBegin() {
	String a("bla");
	t_assert(a.rbegin() != a.rend());
	const String &b = a;
	t_assert(b.rbegin() != b.rend());
}
void StringReverseIteratorTest::testSimpleStringDeref() {
	String a("bla");
	t_assert(*a.rbegin() == a[2L]);
	const String &b = a;
	t_assert(*b.rbegin() == b[2L]);
}
void StringReverseIteratorTest::testSimpleStringIncrement() {
	String a("b");
	String::reverse_iterator ai = a.rbegin();
	t_assert(++ai == a.rend());
	t_assert(ai != a.rbegin());
	const String &b = a;
	String::const_reverse_iterator bi = b.rbegin();
	t_assert(++bi == b.rend());
	t_assert(bi != b.rbegin());
}
void StringReverseIteratorTest::testSimpleStringDecrement() {
	String a("b");
	String::reverse_iterator ai = a.rend();
	t_assert(--ai == a.rbegin());
	t_assert(ai != a.rend());
	const String &b = a;
	String::const_reverse_iterator bi = b.rend();
	t_assert(--bi == b.rbegin());
	t_assert(bi != b.rend());
}
void StringReverseIteratorTest::testSimpleStringAssignment() {
	String a("bla");
	*a.rbegin() = 'x';
	String::const_reverse_iterator ai = a.rbegin();
	t_assert(*ai == 'x');
	t_assert(*++ai == 'l');
	t_assert(*++ai == 'b');
	t_assert(*++ai == '\0');
}
void StringReverseIteratorTest::testStringIteration() {
	String a;
	a.Append(1L);
	a.Append(2L);
	a.Append(3L);
	String::reverse_iterator ai = a.rbegin();
	for (long i = a.size() - 1; i >= 0; --i, ++ai) {
		t_assert(*ai == a[i]);
	}
	t_assert(ai == a.rend());
	const String &b = a;
	String::const_reverse_iterator bi = b.rbegin();
	for (long i = b.size() - 1; i >= 0; --i, ++bi) {
		t_assert(*bi == b[i]);
	}
	t_assert(bi == b.rend());
}
void StringReverseIteratorTest::testSimpleStringIncDec() {
	String a("b");
	String::reverse_iterator ai = a.rbegin();
	String::reverse_iterator ai1 = ++ai;
	t_assert(ai == ai1);
	t_assert(--ai == a.rbegin());
	t_assert(ai1 == a.rend());
	t_assert(ai != ai1);
	t_assert(--ai1 == ai);
	const String &b = a;
	String::const_reverse_iterator bi = b.rbegin();
	String::const_reverse_iterator bi1 = ++bi;
	t_assert(bi == bi1);
	t_assert(--bi == b.rbegin());
	t_assert(bi1 == b.rend());
	t_assert(bi1 != bi);
	t_assert(--bi == bi);
}
void StringReverseIteratorTest::testStringIndex() {
	assertEqual(5, fStr5.size());
	String::reverse_iterator it = fStr5.rbegin();
	assertEqual(fStr5[4], it[0]);
	assertEqual(fStr5[0], it[4]);
	++it;
	assertEqual(fStr5[0], it[3]);
	++it;
	assertEqual(fStr5[0], it[2]);
	assertEqual(fStr5.rbegin()[2], *it);
	assertEqual(*fStr5.rbegin(), it[-2]);
	assertEqual(fStr5.rend()[-3], *it);
	String::const_reverse_iterator cit = fStr5.rbegin();
	assertEqual(fStr5[4], cit[0]);
	assertEqual(fStr5[0], cit[4]);
	++cit;
	assertEqual(fStr5[0], cit[3]);
	++cit;
	assertEqual(fStr5[0], cit[2]);
	assertEqual(fStr5.rbegin()[2], *cit);
	assertEqual(*fStr5.rbegin(), cit[-2]);
	assertEqual(fStr5.rend()[-3], *cit);
}
void StringReverseIteratorTest::testIteratorSubstract() {
	assertEqual(fStr5.size(), fStr5.rend() - fStr5.rbegin());
	const String &a5 = fStr5;
	assertEqual(a5.size(), a5.rend() - a5.rbegin());
	String a;
	const String &b(a);
	assertEqual(a.size(), a.rend() - a.rbegin());
	assertEqual(b.size(), b.rend() - b.rbegin());
	a.Append(1L);
	assertEqual(a.size(), a.rend() - a.rbegin());
	assertEqual(b.size(), b.rend() - b.rbegin());
	a.Append(2L);
	assertEqual(a.size(), a.rend() - a.rbegin());
	assertEqual(b.size(), b.rend() - b.rbegin());
	String::reverse_iterator ai = a.rbegin();
	++ai;
	assertEqual(1, ai - a.rbegin());
	assertEqual(-1, a.rbegin() - ai);
	String::const_reverse_iterator bi = b.rbegin();
	++bi;
	assertEqual(1, bi - b.rbegin());
	assertEqual(-1, b.rbegin() - bi);
}
void StringReverseIteratorTest::testIteratorIntAdd() {
	String::reverse_iterator ai = fStr5.rbegin();
	assertEqual(fStr5[0], *(ai + 4));
	++ai;
	t_assert(ai == (fStr5.rbegin() + 1));
	t_assert(ai == (fStr5.rend() + (-4)));
	String::const_reverse_iterator bi = fStr5.rbegin();
	assertEqual(fStr5[0], *(bi + 4));
	++bi;
	t_assert(bi == (fStr5.rbegin() + 1));
	t_assert(bi == (fStr5.rend() + (-4)));
}
void StringReverseIteratorTest::testIteratorIntSubstract() {
	String::reverse_iterator ai = fStr5.rend();
	assertEqual(fStr5[3], *(ai - 4));
	--ai;
	t_assert(ai == (fStr5.rend() - 1));
	t_assert(ai == (fStr5.rbegin() - (-4)));
	String::const_reverse_iterator bi = fStr5.rend();
	assertEqual(fStr5[3], *(bi - 4));
	--bi;
	t_assert(bi == (fStr5.rend() - 1));
	t_assert(bi == (fStr5.rbegin() - (-4)));
}
void StringReverseIteratorTest::testIteratorCompare() {
	String a;
	t_assert(a.rbegin() <= a.rend());
	t_assert(a.rend() >= a.rbegin());
	t_assert(a.rbegin() <= a.rbegin());
	t_assert(a.rbegin() >= a.rbegin());
	t_assert(!(a.rbegin() < a.rbegin()));
	t_assert(!(a.rbegin() > a.rbegin()));
	const String &b(a);
	t_assert(b.rbegin() <= b.rend());
	t_assert(b.rend() >= b.rbegin());
	t_assert(b.rbegin() <= b.rbegin());
	t_assert(b.rbegin() >= b.rbegin());
	t_assert(!(b.rbegin() < b.rbegin()));
	t_assert(!(b.rbegin() > b.rbegin()));
	a.Append(1L);
	t_assert(a.rbegin() < a.rend());
	t_assert(a.rend() > a.rbegin());
	t_assert(a.rbegin() <= a.rend());
	t_assert(a.rend() >= a.rbegin());
	t_assert(a.rbegin() <= a.rbegin());
	t_assert(a.rbegin() >= a.rbegin());
	t_assert(!(a.rbegin() < a.rbegin()));
	t_assert(!(a.rbegin() > a.rbegin()));
	//- const_iterator
	t_assert(b.rbegin() < b.rend());
	t_assert(b.rend() > b.rbegin());
	t_assert(b.rbegin() <= b.rend());
	t_assert(b.rend() >= b.rbegin());
	t_assert(b.rbegin() <= b.rbegin());
	t_assert(b.rbegin() >= b.rbegin());
	t_assert(!(b.rbegin() < b.rbegin()));
	t_assert(!(b.rbegin() > b.rbegin()));

	t_assert(fStr5.rend() > fStr5.rbegin());
}
void StringReverseIteratorTest::setUp() {
	fStr5.clear();
	for (long i = 1; i <= 5; ++i) {
		fStr5.Append(i);
	}
}
Test *StringReverseIteratorTest::suite() {
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, StringReverseIteratorTest, testEmptyStringBegin);
	ADD_CASE(testSuite, StringReverseIteratorTest, testSimpleStringBegin);
	ADD_CASE(testSuite, StringReverseIteratorTest, testSimpleStringDeref);
	ADD_CASE(testSuite, StringReverseIteratorTest, testSimpleStringIncrement);
	ADD_CASE(testSuite, StringReverseIteratorTest, testSimpleStringDecrement);
	ADD_CASE(testSuite, StringReverseIteratorTest, testSimpleStringIncDec);
	ADD_CASE(testSuite, StringReverseIteratorTest, testSimpleStringAssignment);
	ADD_CASE(testSuite, StringReverseIteratorTest, testStringIteration);
	ADD_CASE(testSuite, StringReverseIteratorTest, testStringIndex);
	ADD_CASE(testSuite, StringReverseIteratorTest, testIteratorSubstract);
	ADD_CASE(testSuite, StringReverseIteratorTest, testIteratorIntAdd);
	ADD_CASE(testSuite, StringReverseIteratorTest, testIteratorIntSubstract);
	ADD_CASE(testSuite, StringReverseIteratorTest, testIteratorCompare);
	return testSuite;
}
