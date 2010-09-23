#ifndef AnythingSTLTest_H
#define AnythingSTLTest_H

#include "TestCase.h"
#include "Anything.h"

class AnythingSTLTest : public TestFramework::TestCase
{
	void checkRange(const Anything &, long n, long length);
	void checkFill(const Anything &);
	void checkFillSizeType(const Anything &);
	void checkSimpleInsert(const Anything &, const char *msg);
	void checkInsertInArray(const Anything &a, long testpos, const char *msg, long n = 1);
	void checkInsertWithKeys(const Anything &a, const long testpos, const char *m, const long n = 1);

public:
	AnythingSTLTest (TString name);

	void	setUp ();
	static Test	*suite ();

	void testSimpleSwap();
	void testSwapWithDifferentAllocator();
	void testFrontBackPushPop();
	void testRangeAssign();
	void testFillAssign();
	void testFillAssignWithSizeType();
	void testRangeCtor();
	void testFillCtor();
	void testFillCtorWithSizeType();

	void testSimpleInsertToEmptyAny();
	void testSimpleInsertAtEnd();
	void testSimpleInsertAtBegin();
	void testSimpleInsertInArray();
	void testSimpleInsertWithKeys();

	void testFillInsertToEmptyAny();
	void testFillInsertInArray();
	void testFillInsertWithKeys();
	void testFillInsertZero();

	void testIterInsertToEmptyAny();
	void testIterInsertInArray();
	void testIterInsertWithKeys();
	void testIterInsertZero();
};
#endif
