#ifndef _SysLogTest_H
#define _SysLogTest_H

#include "cute.h"
#include "ide_listener.h"
#include "cute_runner.h"

class SysLogTest {
public:
	void runAllTests(int argc, char const *argv[]);
	SysLogTest();
	~SysLogTest();
	void TestFlags();
};

#endif
