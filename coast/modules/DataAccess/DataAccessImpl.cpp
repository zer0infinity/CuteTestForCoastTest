/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "DataAccessImpl.h"

//--- standard modules used ----------------------------------------------------
#include "Registry.h"
#include "StringStream.h"
#include "SystemLog.h"
#include "Dbg.h"

#if defined(ONLY_STD_IOSTREAM)
using namespace std;
#endif

//---- DataAccessImplsModule -----------------------------------------------------------
RegisterModule(DataAccessImplsModule);

DataAccessImplsModule::DataAccessImplsModule(const char *name) : WDModule(name)
{
}

bool DataAccessImplsModule::Init(const ROAnything config)
{
	ROAnything roaImpls;
	if ( config.LookupPath(roaImpls, DataAccessImpl::gpcConfigPath) ) {
		HierarchyInstaller hi(DataAccessImpl::gpcCategory);
		return RegisterableObject::Install(roaImpls, DataAccessImpl::gpcCategory, &hi);
	}
	return false;
}

bool DataAccessImplsModule::ResetFinis(const ROAnything config)
{
	AliasTerminator at(DataAccessImpl::gpcCategory);
	return RegisterableObject::ResetTerminate(DataAccessImpl::gpcCategory, &at);
}

bool DataAccessImplsModule::Finis()
{
	return StdFinis(DataAccessImpl::gpcCategory, DataAccessImpl::gpcConfigPath);
}

//--- DataAccessImpl --------------------------------------------------
RegisterDataAccessImpl(DataAccessImpl);
RegCacheImpl(DataAccessImpl);

const char* DataAccessImpl::gpcCategory = "DataAccessImpl";
const char* DataAccessImpl::gpcConfigPath = "DataAccessImpls";
const char* DataAccessImpl::gpcConfigFileName = "DataAccessImplMeta";

DataAccessImpl::DataAccessImpl(const char *name) :
	HierarchConfNamed(name) {
}

IFAObject *DataAccessImpl::Clone() const
{
	Assert(false);
	return new DataAccessImpl(fName);
}

//--- Send/Receive
bool DataAccessImpl::Exec(Context &c, ParameterMapper *input, ResultMapper *output)
{
	StartTrace(DataAccessImpl.Exec);	// support for tracing

	Assert(false); // should not be called
	// use input and output mapper to define trx input and output needs
	// implement this method to do whatever necesary
	// this implementation never succeeds
	return false;
}

bool DataAccessImpl::DoLoadConfig(const char *category)
{
	StartTrace(DataAccessImpl.DoLoadConfig);
	if ( HierarchConfNamed::DoLoadConfig(category) && fConfig.IsDefined(fName) ) {
		// trx impls use only a subset of the whole configuration file
		fConfig = fConfig[fName];
		TraceAny(fConfig, "Config for [" << fName << "]");
		Assert(!fConfig.IsNull());
		return (!fConfig.IsNull());
	}
	fConfig = Anything();
	Trace("No specific " << fName << " config found, still returning true");
	return true;
}

bool DataAccessImpl::DoGetConfigName(const char *category, const char *objName, String &configFileName) const
{
	configFileName = DataAccessImpl::gpcConfigFileName;
	return true;
}

//---- LoopBackDAImpl ------------------------------------------------------
RegisterDataAccessImpl(LoopBackDAImpl);

IFAObject *LoopBackDAImpl::Clone() const
{
	return new LoopBackDAImpl(fName);
}

bool LoopBackDAImpl::Exec(Context &c, ParameterMapper *input, ResultMapper *output)
{
	StartTrace1(LoopBackDAImpl.Exec, "Name: " << fName);
	ROAnything config;
	TraceAny(fConfig, "fConfig: ");
	long streaming = Lookup("Streaming", 0L);

	if (Lookup("transfer", config)) {
		TraceAny(config, "Config: ");
		for (long i = 0, sz = config.GetSize(); i < sz; ++i) {
			const char *slotname = config.SlotName(i);
			if (slotname) {
				String inputStr;
				bool bGetCode, bPutCode = false;
				if (streaming == 1) {
					StringStream Ios(inputStr);
					bGetCode = input->Get(slotname, Ios, c);
					Ios << flush;
					if (bGetCode) {
						bPutCode = output->Put(config[i].AsCharPtr(), inputStr, c);
					}
				} else if (streaming == 2) {
					StringStream Ios(inputStr);
					bGetCode = input->Get(slotname, inputStr, c);
					Ios << flush;
					if (bGetCode) {
						bPutCode = output->Put(config[i].AsCharPtr(), Ios, c);
					}
				} else if (streaming == 3) {
					StringStream Ios(inputStr);
					bGetCode = input->Get(slotname, Ios, c);
					Ios << flush;
					if (bGetCode) {
						bPutCode = output->Put(config[i].AsCharPtr(), Ios, c);
					}
				} else {
					bGetCode = input->Get(slotname, inputStr, c);
					if (bGetCode) {
						bPutCode = output->Put(config[i].AsCharPtr(), inputStr, c);
					}
				}
				if (bGetCode) {
					Trace("From [" << slotname << "] to [" << config[i].AsCharPtr() << "]: <" << inputStr << ">");
				}
				Trace("GetCode: " << (bGetCode ? "true" : "false") << " PutCode: " << (bPutCode ? "true" : "false"));
			}
		}
		Trace("ret: true");
		return true;
	}
	Trace("ret: false");
	return false;
}
