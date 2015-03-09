#include "SysLogTest.h"
#include "SystemLog.h"
#include "Tracer.h"

SysLogTest::SysLogTest() {
	StartTrace(SysLogTest.setUp);
	SystemLog::Init("SysLogTest");
	StartTrace(SysLogTest.Ctor);
}

SysLogTest::~SysLogTest() {
	StartTrace(SysLogTest.Dtor);
	StartTrace(SysLogTest.tearDown);
	SystemLog::Terminate();
}

void SysLogTest::TestFlags() {
	StartTrace(SysLogTest.TestFlags);
	ASSERTM(" expected creation of fgSysLog", SystemLog::getSysLog() != NULL);
}

void SysLogTest::runAllTests(int argc, char const *argv[]) {
	cute::suite s;
	s.push_back(CUTE_SMEMFUN(SysLogTest, TestFlags));
	cute::ide_listener<> lis;
	cute::makeRunner(lis,argc,argv)(s, "AllTests");
}
