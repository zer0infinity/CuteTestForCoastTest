#include "SS1Test.h"
#include "StringStream.h"

SS1Test::SS1Test()
{}

SS1Test::~SS1Test()
{}

void runAllTests(int argc, char const *argv[]){
	cute::suite s;
	s.push_back(CUTE_SMEMFUN(SS1Test, SimpleTest));
	cute::ide_listener<> lis;
	cute::makeRunner(lis,argc,argv)(s, "AllTests");
}

void SS1Test::SimpleTest()
// what: some very simplistic things to find out what does not work about flush
{
	String out;
	{
		OStringStream os(&out);

		os << "something very small" << std::flush;
		ASSERT_EQUAL("something very small", out);
		ASSERT_EQUAL(32, out.Capacity());
		ASSERT_EQUAL(20, out.Length());
	}
	String out2("something very small");
	ASSERT_EQUAL("something very small", out2);
	ASSERT_EQUAL(21, out2.Capacity());
	ASSERT_EQUAL(20, out2.Length());

	out << " a little bit more, this should now double the string";
	ASSERT_EQUAL("something very small a little bit more, this "
				"should now double the string", out);
	ASSERT_EQUAL(146, out.Capacity());
	ASSERT_EQUAL(73, out.Length());
} // SimpleTest

