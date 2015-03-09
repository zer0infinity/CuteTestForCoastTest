#ifndef _StrSpecialTest_h_
#define _StrSpecialTest_h_

#include "cute.h"
#include "ide_listener.h"
#include "cute_runner.h"

class StrSpecialTest {
public:
	void simpleAppendTest();
	void umlauteTest();
	void runAllTests(int argc, char const *argv[]);
};

#endif
