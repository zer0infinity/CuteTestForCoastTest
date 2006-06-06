//-*-Mode: C++;-*-
/* Some dummy Test cases to start with */
#ifndef _DummyTest_h_
#define _DummyTest_h_

#include "TestCase.h"

//---- DummyTest -----------------------------------------------------------
class DummyTest : public TestFramework::TestCase
{
public:
	DummyTest(TString tstrName);
	virtual ~DummyTest();

	static Test *suite ();
	void dummyTest ();
};

#endif
