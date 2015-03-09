#include "StringTestExtreme.h"
#include "SystemFile.h"

using namespace coast;

StringTestExtreme::StringTestExtreme() {
	char s;
	extremelyLongString = "";
	trueString = "";

	std::iostream *is = system::OpenStream("longString", "txt", std::ios::in);
	while (is && !is->eof()) {
		if (is->read(&s, 1)) {
			extremelyLongString.Append(s);
			trueString.append(1, s);
		}
	}
}

void StringTestExtreme::runAllTests(int argc, char const *argv[]) {
	cute::suite s;
	s.push_back(CUTE_SMEMFUN(StringTestExtreme, extremeLength));
	cute::ide_listener<> lis;
	cute::makeRunner(lis,argc,argv)(s, "AllTests");
}

void StringTestExtreme::extremeLength() {
	// Init a string without parameters
	ASSERT(extremelyLongString.Length()>0);
	ASSERT_EQUAL(trueString.length(), extremelyLongString.Length());
}
