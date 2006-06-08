/*
 * Copyright (c) 2006, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _WDBaseTestPolicies_H
#define _WDBaseTestPolicies_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"
#include "AnythingConfigTestPolicy.h"
#include "AppBooter.h"
#include "Application.h"
#include "WDModule.h"

//---- forward declaration -----------------------------------------------

//---- WDBaseTestPolicies ----------------------------------------------------------
//! <B>really brief class description</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
namespace TestFramework
{

	template
	<
	class dummy
	>
	class AnythingConfigWithDllAndModuleLoadingTestPolicy
		: public AnythingConfigTestPolicy<dummy>
	{
		AppBooter fBooter;

	public:
		typedef AnythingConfigWithDllAndModuleLoadingTestPolicy<dummy> ConfigPolicyType;
		typedef AnythingConfigTestPolicy<dummy> BaseClassPolicyType;

		AnythingConfigWithDllAndModuleLoadingTestPolicy() {};
		virtual ~AnythingConfigWithDllAndModuleLoadingTestPolicy() {};

	protected:

		ROAnything getConfigForDllAndModuleLoading() {
			return DoGetConfigForDllAndModuleLoading();
		}

		AppBooter &getAppBooter() {
			return fBooter;
		}

		virtual ROAnything DoGetConfigForDllAndModuleLoading() {
			return BaseClassPolicyType::GetConfig();
		}

		virtual bool DoLoadConfig(TString strClassName, TString strTestName) {
			StartTrace(AnythingConfigWithDllAndModuleLoadingTestPolicy.DoLoadConfig);
			bool bRetCode = false;
			if ( ( bRetCode = BaseClassPolicyType::DoLoadConfig(strClassName, strTestName) ) ) {
				Anything anyConfig = getConfigForDllAndModuleLoading().DeepClone();
				Application::InitializeGlobalConfig(anyConfig);

				// load the shared objects defined in the config file
				// those are the client parts not known at link time of this
				// executable
				if ( ( bRetCode = fBooter.OpenLibs(anyConfig) ) ) {
					// add modules configured in the Config file
					bRetCode = ( WDModule::Install(anyConfig) == 0 );
				}
			}
			return bRetCode;
		}

		virtual void DoUnloadConfig() {
			WDModule::Terminate(getConfigForDllAndModuleLoading());
			fBooter.CloseLibs();
			Application::InitializeGlobalConfig(Anything());
			BaseClassPolicyType::DoUnloadConfig();
		}
	};

	template
	<
	class dummy
	>
	class AnythingConfigWithCaseDllAndModuleLoadingTestPolicy
		: public AnythingConfigWithDllAndModuleLoadingTestPolicy<dummy>
	{
	public:
		typedef AnythingConfigWithCaseDllAndModuleLoadingTestPolicy<dummy> ConfigPolicyType;
		typedef AnythingConfigWithDllAndModuleLoadingTestPolicy<dummy> BaseClassPolicyType;

		AnythingConfigWithCaseDllAndModuleLoadingTestPolicy() {};
		virtual ~AnythingConfigWithCaseDllAndModuleLoadingTestPolicy() {};

	protected:
		virtual ROAnything DoGetConfigForDllAndModuleLoading() {
			return BaseClassPolicyType::GetTestCaseConfig();
		}
	};

	typedef TestCaseT<AnythingConfigWithDllAndModuleLoadingTestPolicy, NoStatisticPolicy, int> TestCaseWithGlobalConfigDllAndModuleLoading;

	typedef TestCaseT<AnythingConfigWithCaseDllAndModuleLoadingTestPolicy, NoStatisticPolicy, int> TestCaseWithCaseConfigDllAndModuleLoading;

}	// end namespace TestFramework

#endif
