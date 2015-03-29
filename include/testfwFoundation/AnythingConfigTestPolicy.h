/*
 * Copyright (c) 2006, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AnythingConfigTestPolicy_H
#define _AnythingConfigTestPolicy_H

#include "SystemFile.h"
#include "Tracer.h"
#include "TString.h"

namespace coast {
	namespace testframework {
		template< class InputType >
		void PutInStore(InputType source, Anything &dest) {
			long sz = source.GetSize();
			for (long i = 0; i < sz; ++i) {
				dest[source.SlotName(i)] = source[i].DeepClone();
			}
		}
	}
}
//! <B>really brief class description</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
namespace testframework {
	template
	<
	class dummy
	>
	class AnythingConfigTestPolicy
	{
	public:
		typedef AnythingConfigTestPolicy<dummy> ConfigPolicyType;

		AnythingConfigTestPolicy() {};
		virtual ~AnythingConfigTestPolicy() {};

		bool loadConfig(TString strClassName, TString strTestName) {
			return DoLoadConfig(strClassName, strTestName);
		}

		void unloadConfig() {
			DoUnloadConfig();
		}

	protected:
		Anything fConfig;
		ROAnything fTestCaseConfig;
		TString fCfgFileName, fTestCaseName;

		virtual TString getConfigFileName() {
			return fCfgFileName;
		}

		ROAnything GetConfig() {
			return fConfig;
		}

		ROAnything GetTestCaseConfig() {
			return fTestCaseConfig;
		}

		virtual bool DoLoadConfig(TString strClassName, TString strTestName) {
			StartTrace(AnythingConfigTestPolicy.DoLoadConfig);
			bool bRetCode = false;
			fCfgFileName = strClassName;
			fTestCaseName = strTestName;
			if ( fCfgFileName != getConfigFileName() ) {
				fCfgFileName = getConfigFileName();
			}
			if ( ( bRetCode = coast::system::LoadConfigFile(fConfig, fCfgFileName, "any") ) ) {
				fTestCaseConfig = fConfig[strTestName];
			}
			TraceAny(fConfig, "whole Config of [" << fCfgFileName << "]");
			TraceAny(fTestCaseConfig, "config of TestCase [" << strTestName << "]");
			return bRetCode;
		}

		virtual void DoUnloadConfig() {
			fTestCaseConfig = ROAnything();
			fConfig = Anything();
		}
	};

} // end namespace testframework

#endif
