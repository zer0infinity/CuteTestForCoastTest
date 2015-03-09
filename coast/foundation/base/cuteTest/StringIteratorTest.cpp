#include "StringIteratorTest.h"

void StringIteratorTest::testEmptyStringBegin() {
	String a;
	ASSERT(a.begin() == a.end());
	const String &b = a;
	ASSERT(b.begin() == b.end());
}
void StringIteratorTest::testSimpleStringBegin() {
	String a("bla");
	ASSERT(a.begin() != a.end());
	const String &b = a;
	ASSERT(b.begin() != b.end());
}
void StringIteratorTest::testSimpleStringDeref() {
	String a("bla");
	ASSERT(*a.begin() == a[0L]);
	const String &b = a;
	ASSERT(*b.begin() == b[0L]);
}
void StringIteratorTest::testSimpleStringIncrement() {
	String a("b");
	String_iterator ai = a.begin();
	ASSERT(++ai == a.end());
	ASSERT(ai != a.begin());
	const String &b = a;
	String_const_iterator bi = b.begin();
	ASSERT(++bi == b.end());
	ASSERT(bi != b.begin());
}
void StringIteratorTest::testSimpleStringDecrement() {
	String a("b");
	String_iterator ai = a.end();
	ASSERT(--ai == a.begin());
	ASSERT(ai != a.end());
	const String &b = a;
	String_const_iterator bi = b.end();
	ASSERT(--bi == b.begin());
	ASSERT(bi != b.end());
}
void StringIteratorTest::testSimpleStringAssignment() {
	String a("bla");
	*a.begin() = 'x';
	String::const_iterator ai = a.begin();
	ASSERT(*ai == 'x');
	ASSERT(*++ai == 'l');
	ASSERT(*++ai == 'a');
	ASSERT(*++ai == '\0');
}
void StringIteratorTest::testStringIteration() {
	String a;
	a.Append(1L);
	a.Append(2L);
	a.Append(3L);
	String_iterator ai = a.begin();
	for (long i = 0L; i < a.size(); ++i, ++ai) {
		ASSERT(*ai == a[i]);
	}
	ASSERT(ai == a.end());
	const String &b = a;
	String_const_iterator bi = b.begin();
	for (long i = 0L; i < b.size(); ++i, ++bi) {
		ASSERT(*bi == b[i]);
	}
	ASSERT(bi == b.end());
}
void StringIteratorTest::testSimpleStringIncDec() {
	String a("b");
	String_iterator ai = a.begin();
	String_iterator ai1 = ++ai;
	ASSERT(ai == ai1);
	ASSERT(--ai == a.begin());
	ASSERT(ai1 == a.end());
	ASSERT(ai != ai1);
	ASSERT(--ai1 == ai);
	const String &b = a;
	String_const_iterator bi = b.begin();
	String_const_iterator bi1 = ++bi;
	ASSERT(bi == bi1);
	ASSERT(--bi == b.begin());
	ASSERT(bi1 == b.end());
	ASSERT(bi1 != bi);
	ASSERT(--bi == bi);
}
void StringIteratorTest::testStringIndex() {
	ASSERT_EQUAL(5, fStr5.size());
	String_iterator it = fStr5.begin();
	ASSERT_EQUAL(fStr5[0], it[0]);
	ASSERT_EQUAL(fStr5[4], it[4]);
	++it;
	ASSERT_EQUAL(fStr5[4], it[3]);
	++it;
	ASSERT_EQUAL(fStr5[4], it[2]);
	ASSERT_EQUAL(fStr5.begin()[2], *it);
	ASSERT_EQUAL(*fStr5.begin(), it[-2]);
	ASSERT_EQUAL(fStr5.end()[-3], *it);
	String_const_iterator cit = fStr5.begin();
	ASSERT_EQUAL(fStr5[0], cit[0]);
	ASSERT_EQUAL(fStr5[4], cit[4]);
	++cit;
	ASSERT_EQUAL(fStr5[4], cit[3]);
	++cit;
	ASSERT_EQUAL(fStr5[4], cit[2]);
	ASSERT_EQUAL(fStr5.begin()[2], *cit);
	ASSERT_EQUAL(*fStr5.begin(), cit[-2]);
	ASSERT_EQUAL(fStr5.end()[-3], *cit);
}
void StringIteratorTest::testIteratorSubstract() {
	ASSERT_EQUAL(fStr5.size(), fStr5.end() - fStr5.begin());
	const String &a5 = fStr5;
	ASSERT_EQUAL(a5.size(), a5.end() - a5.begin());
	String a;
	const String &b(a);
	ASSERT_EQUAL(a.size(), a.end() - a.begin());
	ASSERT_EQUAL(b.size(), b.end() - b.begin());
	a.Append(1L);
	ASSERT_EQUAL(a.size(), a.end() - a.begin());
	ASSERT_EQUAL(b.size(), b.end() - b.begin());
	a.Append(2L);
	ASSERT_EQUAL(a.size(), a.end() - a.begin());
	ASSERT_EQUAL(b.size(), b.end() - b.begin());
	String_iterator ai = a.begin();
	++ai;
	ASSERT_EQUAL(1, ai - a.begin());
	ASSERT_EQUAL(-1, a.begin() - ai);
	String_const_iterator bi = b.begin();
	++bi;
	ASSERT_EQUAL(1, bi - b.begin());
	ASSERT_EQUAL(-1, b.begin() - bi);
}
void StringIteratorTest::testIteratorIntAdd() {
	String_iterator ai = fStr5.begin();
	ASSERT_EQUAL(fStr5[4], *(ai + 4));
	++ai;
	ASSERT(ai == (fStr5.begin() + 1));
	ASSERT(ai == (fStr5.end() + (-4)));
	String_const_iterator bi = fStr5.begin();
	ASSERT_EQUAL(fStr5[4], *(bi + 4));
	++bi;
	ASSERT(bi == (fStr5.begin() + 1));
	ASSERT(bi == (fStr5.end() + (-4)));
}
void StringIteratorTest::testIteratorIntSubstract() {
	String_iterator ai = fStr5.end();
	ASSERT_EQUAL(fStr5[1], *(ai - 4));
	--ai;
	ASSERT(ai == (fStr5.end() - 1));
	ASSERT(ai == (fStr5.begin() - (-4)));
	String_const_iterator bi = fStr5.end();
	ASSERT_EQUAL(fStr5[1], *(bi - 4));
	--bi;
	ASSERT(bi == (fStr5.end() - 1));
	ASSERT(bi == (fStr5.begin() - (-4)));
}
void StringIteratorTest::testIteratorCompare() {
	String a;
	ASSERT(a.begin() <= a.end());
	ASSERT(a.end() >= a.begin());
	ASSERT(a.begin() <= a.begin());
	ASSERT(a.begin() >= a.begin());
	ASSERT(!(a.begin() < a.begin()));
	ASSERT(!(a.begin() > a.begin()));
	const String &b(a);
	ASSERT(b.begin() <= b.end());
	ASSERT(b.end() >= b.begin());
	ASSERT(b.begin() <= b.begin());
	ASSERT(b.begin() >= b.begin());
	ASSERT(!(b.begin() < b.begin()));
	ASSERT(!(b.begin() > b.begin()));
	a.Append(1L);
	ASSERT(a.begin() < a.end());
	ASSERT(a.end() > a.begin());
	ASSERT(a.begin() <= a.end());
	ASSERT(a.end() >= a.begin());
	ASSERT(a.begin() <= a.begin());
	ASSERT(a.begin() >= a.begin());
	ASSERT(!(a.begin() < a.begin()));
	ASSERT(!(a.begin() > a.begin()));
	//- const_iterator
	ASSERT(b.begin() < b.end());
	ASSERT(b.end() > b.begin());
	ASSERT(b.begin() <= b.end());
	ASSERT(b.end() >= b.begin());
	ASSERT(b.begin() <= b.begin());
	ASSERT(b.begin() >= b.begin());
	ASSERT(!(b.begin() < b.begin()));
	ASSERT(!(b.begin() > b.begin()));

	ASSERT(fStr5.end() > fStr5.begin());
}
StringIteratorTest::StringIteratorTest() {
	fStr5.clear();
	for (long i = 1; i <= 5; ++i) {
		fStr5.Append(i);
	}
}

void StringIteratorTest::runAllTests(int argc, char const *argv[]) {
	cute::suite s;
	s.push_back(CUTE_SMEMFUN(StringIteratorTest, testEmptyStringBegin));
	s.push_back(CUTE_SMEMFUN(StringIteratorTest, testSimpleStringBegin));
	s.push_back(CUTE_SMEMFUN(StringIteratorTest, testSimpleStringDeref));
	s.push_back(CUTE_SMEMFUN(StringIteratorTest, testSimpleStringIncrement));
	s.push_back(CUTE_SMEMFUN(StringIteratorTest, testSimpleStringDecrement));
	s.push_back(CUTE_SMEMFUN(StringIteratorTest, testSimpleStringIncDec));
	s.push_back(CUTE_SMEMFUN(StringIteratorTest, testSimpleStringAssignment));
	s.push_back(CUTE_SMEMFUN(StringIteratorTest, testStringIteration));
	s.push_back(CUTE_SMEMFUN(StringIteratorTest, testStringIndex));
	s.push_back(CUTE_SMEMFUN(StringIteratorTest, testIteratorSubstract));
	s.push_back(CUTE_SMEMFUN(StringIteratorTest, testIteratorIntAdd));
	s.push_back(CUTE_SMEMFUN(StringIteratorTest, testIteratorIntSubstract));
	s.push_back(CUTE_SMEMFUN(StringIteratorTest, testIteratorCompare));
	cute::ide_listener<> lis;
	cute::makeRunner(lis,argc,argv)(s, "AllTests");
}
