/*
 * Copyright (c) 2011, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 *
 * Author(s): m1huber
 */

#ifndef CHECKSTORES_H_
#define CHECKSTORES_H_

#include "StatUtils.h"
#include "Anything.h"
#include "Action.h"

namespace coast {
	namespace testframework {
		//! type switch for store checks
		enum eResultCheckType { exists, notExists };

		//! helper method to generate a list of paths out of an anything
		void GeneratePathList(Anything &pathList, ROAnything &input, String const &pathSoFar, char delimSlot);

		//! Really compare the store using AnyUtils::AnyCompareEqual
		void CheckStoreContents(Anything &anyFailures, ROAnything anyInput, ROAnything anyMaster, const char *storeName, const char *testCaseName, char delimSlot = '.', char delimIdx = ':', coast::testframework::eResultCheckType rct = coast::testframework::exists);

		/*!	utility method to perform Checks in ctx stores
			expected has the format
			<PRE>
			{
				/SessionStore	{ .. }		# The whole content is compared to the ctxToCheck's SessionStore
				/RoleStore	{ .. }			# The whole content is compared to the ctxToCheck's RoleStore
				/TmpStore	{				# Each slot is compared to the slot with the same name
					/Slot1	{ .. }			# in ctxToCheck's TmpStore
					/Slot2	*
				}
			}</PRE>
			\param expected Anything containing expected results
			\param ctxToCheck contexts that supplies the SessionStore and TmpStore to be checked
			\param testCaseName String that is printed with failure messages
			\param rct result check type, either eResultCheckType::exists or eResultCheckType::notExists */
		void CheckStores(Anything &anyFailures, ROAnything expected, Context &ctxToCheck, const char *testCaseName, coast::testframework::eResultCheckType rct = coast::testframework::exists);
	}
}

#endif /* CHECKSTORES_H_ */
