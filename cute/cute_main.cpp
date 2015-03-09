#include "cute.h"
#include "ide_listener.h"
#include "cute_runner.h"

struct Test {
	bool TestFalse() {
		ASSERT(false);
	}

	bool TestTrue() {
		ASSERT(true);
	}
};

void runAllTests(int argc, char const *argv[]) {
	cute::suite s;
	s.push_back(CUTE_SMEMFUN(Test, TestFalse));
	s.push_back(CUTE_SMEMFUN(Test, TestTrue));
	cute::ide_listener<> lis;
	cute::makeRunner(lis,argc,argv)(s, "AllTests");
}

int main(int argc, char const *argv[]) {
	runAllTests(argc, argv);
	return 0;
}
