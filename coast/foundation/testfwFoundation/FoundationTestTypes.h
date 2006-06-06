/*
 * Copyright (c) 2006, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _FoundationTestTypes_H
#define _FoundationTestTypes_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"
#include "AnythingConfigTestPolicy.h"
#include "AnythingStatisticTestPolicy.h"

//---- forward declaration -----------------------------------------------

//---- specialized test types ----------------------------------------------------------
namespace TestFramework
{

	typedef TestCaseT<AnythingConfigTestPolicy, NoStatisticPolicy, int> TestCaseWithConfig;

	typedef TestCaseT<NoConfigPolicy, AnythingStatisticTestPolicy, int> TestCaseWithStatistics;

	typedef TestCaseT<AnythingConfigTestPolicy, AnythingStatisticTestPolicy, int> TestCaseWithConfigAndStatistics;

} // end namespace TestFramework

#endif
