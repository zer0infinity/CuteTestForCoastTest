//-*-Mode: C++;-*-
/* Some dummy Test cases to start with */
#ifndef _DummyTest_h_
#define _DummyTest_h_

#include "TestCase.h"

//---- DummyTest -----------------------------------------------------------
class DummyTest : public TestCase
{
public:
	DummyTest(TString name);
	virtual ~DummyTest();

	virtual void setUp ();
	static Test *suite ();
	void dummyTest ();
};

#endif
