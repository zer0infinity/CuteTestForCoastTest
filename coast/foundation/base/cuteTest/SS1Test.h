#ifndef _SS1Test_h_
#define _SS1Test_h_

#include "cute.h"
#include "ide_listener.h"
#include "cute_runner.h"

class SS1Test {
public:
	void SimpleTest();
	void runAllTests(int argc, char const *argv[]);
};

#endif
