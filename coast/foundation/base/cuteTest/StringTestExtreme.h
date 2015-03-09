#ifndef _StringTestExtreme_h_
#define _StringTestExtreme_h_

#include "cute.h"
#include "ide_listener.h"
#include "cute_runner.h"
#include "ITOString.h"//lint !e537
class StringTestExtreme {
protected:
	String extremelyLongString;
	std::string trueString;
public:
	void runAllTests(int argc, char const *argv[]);
	StringTestExtreme();
	void extremeLength();
};

#endif
