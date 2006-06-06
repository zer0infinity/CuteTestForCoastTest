/*
 * Copyright (c) 2006, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AnythingConfigTestPolicy_H
#define _AnythingConfigTestPolicy_H

//---- baseclass include -------------------------------------------------
#include "StringStream.h"
#include "System.h"
#include "Dbg.h"

//---- forward declaration -----------------------------------------------

//---- AnythingConfigTestPolicy ----------------------------------------------------------
//! <B>really brief class description</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
namespace TestFramework
{

	template
	<
	typename dummy
	>
	class AnythingConfigTestPolicy
	{
		Anything fConfig;
		ROAnything fTestCaseConfig;
		TString fCfgFileName;

	public:
		typedef AnythingConfigTestPolicy<dummy> ConfigPolicyType;

		AnythingConfigTestPolicy() {};
		virtual ~AnythingConfigTestPolicy() {};

	protected:
		template< class InputType >
		void PutInStore(InputType source, Anything &dest) {
			StartTrace(ConfigPolicy.PutInStore);

			long sz = source.GetSize();
			for (long i = 0; i < sz; ++i) {
				dest[source.SlotName(i)] = source[i].DeepClone();
			}
		}

		virtual TString getConfigFileName() {
			return fCfgFileName;
		}

		ROAnything GetConfig() {
			return fConfig;
		}
		ROAnything GetTestCaseConfig() {
			return fTestCaseConfig;
		}
		bool loadConfig(TString strClassName, TString strTestName) {
			StartTrace(ConfigPolicy.setUp);
			bool bRetCode = false;
			fCfgFileName = strClassName;
			if ( fCfgFileName != getConfigFileName() ) {
				fCfgFileName = getConfigFileName();
			}
			if ( ( bRetCode = System::LoadConfigFile(fConfig, fCfgFileName, "any") ) ) {
				fTestCaseConfig = fConfig[strTestName];
			}
			TraceAny(fConfig, "whole Config of [" << fCfgFileName << "]");
			TraceAny(fTestCaseConfig, "config of TestCase [" << strTestName << "]");
			return bRetCode;
		}
		void unloadConfig() {
			fTestCaseConfig = ROAnything();
			fConfig = Anything();
		}
	};

}	// end namespace TestFramework

#endif
