/*
 * Copyright (c) 2011, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 *
 * Author(s): m1huber
 */

#include "CheckStores.h"
#include "AnyUtils.h"
#include "StringStream.h"
#include "StringStreamBuf.h"
#include "Context.h"

namespace coast {
	namespace testframework {
		//! helper method to generate a list of paths out of an anything
		void GeneratePathList(Anything &pathList, ROAnything &input, String const &pathSoFar, char delimSlot) {
			StartTrace(ConfiguredTestHelper.GeneratePathList);
			for (long i = 0, sz = input.GetSize(); i < sz; ++i) {
				String path(pathSoFar);
				path << input.SlotName(i);
				if (input[i].GetType() == AnyArrayType) {
					// continue recursively
					ROAnything next(input[i]);
					path << delimSlot;
					GeneratePathList(pathList, next, path, delimSlot);
				} else {
					// leaf reached, add path to list
					pathList[pathList.GetSize()] = path;
				}
			}
		}

		//! Really compare the store using AnyUtils::AnyCompareEqual
		void CheckStoreContents(Anything &anyFailures, ROAnything anyInput, ROAnything anyMaster, const char *storeName, const char *testCaseName, char delimSlot, char delimIdx, coast::testframework::eResultCheckType rct) {
			StartTrace(ConfiguredTestHelper.CheckStoreContents);
			if (rct == coast::testframework::exists) {
				String strTestPath = storeName;
				strTestPath << "." << testCaseName;
				OStringStream s;
				if ( !AnyUtils::AnyCompareEqual(anyInput,anyMaster,strTestPath, &s, delimSlot, delimIdx) )
				{
					String strfail(strTestPath);
					strfail << "\n" << s.str();
					anyFailures.Append(strfail);
				}
			} else if (rct == coast::testframework::notExists) {
				// anyInput == ctxToBeChecked
				// anyMaster == notExpected
				// ------------------------------------------------------------
				// With this version, it is only possible to check for absence
				// of named slots (cannot have unnamed slots so far).
				// ------------------------------------------------------------

				// generate list of paths and check for existence
				Anything pathList;
				coast::testframework::GeneratePathList(pathList, anyMaster, "", delimSlot);
				TraceAny(anyInput, "Store to be checked (" << storeName << "):");
				TraceAny(pathList, "List of paths to check for non-existence:");
				ROAnything luResult;
				if (!pathList.IsNull()) {
					for (long i = 0, sz = pathList.GetSize(); i < sz; ++i) {
						if (anyInput.LookupPath(luResult, pathList[i].AsCharPtr())) {
							// error, if lookup succeeds
							String strfail(testCaseName);
							strfail << "\n\t" << "Slot [" << pathList[i].AsCharPtr() << "] should not exist in " << storeName << "!";
							anyFailures.Append(strfail);
						}
					}
				}
			}
		}

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
		void CheckStores(Anything &anyFailures, ROAnything expected, Context &ctxToCheck, const char *testCaseName, coast::testframework::eResultCheckType rct) {
			StartTrace(ConfiguredTestHelper.CheckStores);
			char delimSlot, delimIdx;
			delimSlot = expected["Delim"].AsCharPtr(".")[0L];
			delimIdx = expected["IndexDelim"].AsCharPtr(":")[0L];
			if (expected.IsDefined("SessionStore") ) {
				CheckStoreContents(anyFailures, ctxToCheck.GetSessionStore(), expected["SessionStore"], "SessionStore", testCaseName, delimSlot, delimIdx, rct);
			}
			if (expected.IsDefined("RoleStore") ) {
				CheckStoreContents(anyFailures, ctxToCheck.GetRoleStoreGlobal(), expected["RoleStore"], "RoleStore", testCaseName, delimSlot, delimIdx, rct);
			}
			if (expected.IsDefined("Query") ) {
				CheckStoreContents(anyFailures, ctxToCheck.GetQuery(), expected["Query"], "Query", testCaseName, delimSlot, delimIdx, rct);
			}
			if (expected.IsDefined("TmpStore") ) {
				CheckStoreContents(anyFailures, ctxToCheck.GetTmpStore(), expected["TmpStore"], "TmpStore", testCaseName, delimSlot, delimIdx, rct);
			}
		}
	}
}
