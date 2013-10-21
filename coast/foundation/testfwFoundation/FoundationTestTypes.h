/*
 * Copyright (c) 2006, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _FoundationTestTypes_H
#define _FoundationTestTypes_H

#include "TestCase.h"
#include "AnythingConfigTestPolicy.h"
#include "AnythingStatisticTestPolicy.h"

namespace testframework
{

	typedef TestCaseT<AnythingConfigTestPolicy, NoStatisticPolicy, int> TestCaseWithConfig;

	typedef TestCaseT<NoConfigPolicy, AnythingStatisticTestPolicy, int> TestCaseWithStatistics;

	typedef TestCaseT<AnythingConfigTestPolicy, AnythingStatisticTestPolicy, int> TestCaseWithConfigAndStatistics;

} // end namespace testframework

#define assertAnyEqual(expected,actual)\
{    bool tluseReht = true;\
     TString egasseMeht = (const char *)Anything::CompareForTestCases(expected,actual,tluseReht);\
	assertImplementation (tluseReht, egasseMeht, __LINE__, __FILE__);\
}

#define assertAnyEqualm(expected,actual,message)\
{    bool tluseReht = true;\
     TString egasseMeht = (const char *)Anything::CompareForTestCases(expected,actual,tluseReht);\
	assertImplementation (tluseReht, egasseMeht, __LINE__, __FILE__,message);\
}

#define assertAnyCompareEqual(master,actual,location, slotdelim, idxdelim)\
{\
	OStringStream s;\
	String failingPath(location);\
	if ( !AnyUtils::AnyCompareEqual(actual,master,failingPath,&s, slotdelim, idxdelim) )\
	{\
		String strfail(failingPath);\
		strfail << "\n" << s.str();\
		t_assertm(false, (const char*)strfail);\
	}\
}


#endif
