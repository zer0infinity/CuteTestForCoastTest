#include "StringStream.h"
#include "SystemFile.h"
#include "StrSpecialTest.h"

using namespace coast;

void StrSpecialTest::simpleAppendTest() {
	String s("Hallo");
	OStringStream os(&s);
	os << 5L << " = " << 2L << '+' << 3L;
	os.flush();
	ASSERT_EQUAL("Hallo5 = 2+3", s);
}

void StrSpecialTest::umlauteTest() {
	// standard query case
	Anything test;
	std::istream *is = system::OpenStream("UmlautTest", "any");
	ASSERT(is != 0);

	if (is) {
		test.Import(*is);
		String testString(test["fields"][0L].AsString());
		ASSERT_EQUAL("Ausführen", testString.SubString(0, 10));

		delete is;
	}
}

void StrSpecialTest::runAllTests(int argc, char const *argv[]) {
	cute::suite s;
	s.push_back(CUTE_SMEMFUN(StrSpecialTest, simpleAppendTest));
	s.push_back(CUTE_SMEMFUN(StrSpecialTest, umlauteTest));
	cute::ide_listener<> lis;
	cute::makeRunner(lis,argc,argv)(s, "AllTests");
}
