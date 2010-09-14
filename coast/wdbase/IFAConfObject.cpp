/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "IFAConfObject.h"

//--- standard modules used ----------------------------------------------------
#include "SystemLog.h"
#include "Registry.h"
#include "CacheHandler.h"
#include "System.h"
#include "Dbg.h"

//---- RegisterableObject ----------------------------------------------------------
void RegisterableObject::ResetCache(bool resetCache)
{
	fgResetCache = resetCache;
}
bool RegisterableObject::fgResetCache = false;

RegisterableObject::RegisterableObject(const char *name)
	: NamedObject()
	, fName(name, -1, Storage::Global())
	, fStaticallyInitialized(false)
	, fbInitialized(false)
{
}

// support for registerable named objects
// class API
bool RegisterableObject::Install(const ROAnything installerSpec, const char *category, InstallerPolicy *installer)
{
	// this method installs a list of clones or aliases into the registry
	// the basic assumption is that at least one object is already registered
	// with the register macro
	StartTrace1(RegisterableObject.Install, "cat <" << NotNull(category) << ">");
	Registry *reg = Registry::GetRegistry(category);
	TraceAny(installerSpec, "Installing Spec:");
	if ( reg ) {
		return reg->Install(installerSpec, installer);
	}
	return false;
}

bool RegisterableObject::Reset(const ROAnything installerSpec, const char *category, InstallerPolicy *installer, TerminationPolicy *terminator)
{
	if ( ResetTerminate(category, terminator) ) {
		return Install(installerSpec, category, installer);
	}
	return false;
}

bool RegisterableObject::ResetTerminate(const char *category, TerminationPolicy *terminator)
{
	StartTrace1(RegisterableObject.ResetTerminate, "category <" << category << ">");
	Registry *r;
	r = Registry::GetRegistry(category);
	if ( r ) {
		Trace("found registry, terminating it");
		//!@FIXME how to delete registered alias without
		// crashing the shutdown
		r->Terminate(terminator);
		return true;
	}
	return false;
}

bool RegisterableObject::Terminate(const char *category, TerminationPolicy *terminator)
{
	StartTrace1(RegisterableObject.Terminate, "category <" << category << ">");
	if ( ResetTerminate(category, terminator) ) {
		//SOP: do not delete registries, they might contain "static" objects for further use
		//		Registry *r= Registry::RemoveRegistry(category);
		//		delete r;
		return true;
	}
	return false;
}

void RegisterableObject::Register(const char *name, const char *category)
{
	StartTrace1(RegisterableObject.Register, "cat <" << NotNull(category) << "> name <" << NotNull(name) << ">");
	fCategory = category;
	Registry *reg = Registry::GetRegistry(category);
	if (reg) {
		Trace("Registering: <" << name << "> of category <" << category << ">");
		reg->RegisterRegisterableObject(name, this);
	}
}

void RegisterableObject::Unregister(const char *name, const char *category)
{
	StartTrace1(RegisterableObject.Unregister, "cat <" << NotNull(category) << "> name <" << NotNull(name) << ">");
	Registry *reg = Registry::GetRegistry(category);
	if (reg) {
		Trace("Unregistering: <" << name << "> of category <" << category << ">");
		reg->UnregisterRegisterableObject(name);
	}
}

bool RegisterableObject::Initialize(const char *category)
{
	StatTrace(RegisterableObject.Initialize, "cat <" << NotNull(category) << "> fCat <" << fCategory << ">", Storage::Current());
	if ( category != NULL ) {
		if ( !fCategory.Length() ) {
			fCategory = category;
		}
	}
	return ( fbInitialized = IntInitialize(category) );
}

bool RegisterableObject::Finalize()
{
	return !( fbInitialized = !IntFinalize() );
}

bool RegisterableObject::IntInitialize(const char *category)
{
	StartTrace1(RegisterableObject.IntInitialize, "cat <" << NotNull(category) << "> fCat <" << fCategory << ">");
	return DoInitialize();
}

bool RegisterableObject::IntFinalize()
{
	StartTrace1(RegisterableObject.IntFinalize, "cat <" << fCategory << "> name <" << fName << ">");
	return DoFinalize();
}

//---- ConfNamedObject ----------------------------------------------------------
bool ConfNamedObject::IntInitialize(const char *category)
{
	StartTrace1(ConfNamedObject.IntInitialize, "cat <" << NotNull(category) << "> fCat <" << fCategory << ">");
	// initialize only once
	return ( fbConfigLoaded || ( ( fbConfigLoaded = DoCheckConfig(category, !fbConfigLoaded) ) && DoInitialize() ) );
}

bool ConfNamedObject::IntFinalize()
{
	StartTrace1(ConfNamedObject.IntFinalize, "cat <" << fCategory << "> name <" << fName << ">");
	return ( DoFinalize() && !( fbConfigLoaded = !DoUnloadConfig() ) );
}

bool ConfNamedObject::DoInitialize()
{
	StartTrace1(ConfNamedObject.DoInitialize, "cat <" << fCategory << "> name <" << fName << ">");
	return true;
}

bool ConfNamedObject::DoFinalize()
{
	StartTrace1(ConfNamedObject.DoFinalize, "cat <" << fCategory << "> name <" << fName << ">");
	return true;
}

ConfNamedObject *ConfNamedObject::ConfiguredClone(const char *category, const char *name, bool bInitializeConfig)
{
	StartTrace1(ConfNamedObject.ConfiguredClone, "cat <" << NotNull(category) << "> name <" << name << "> clone-base <" << fName << ">");
	// allow subclasses to do specific cloning stuff
	ConfNamedObject *cno = DoConfiguredClone(category, name, bInitializeConfig);
	if ( cno && bInitializeConfig ) {
		// store cloned objects name for configuration loading in case we re-initialize it
		cno->SetConfigName(fName);
		// do a Initialize on demand for the newly created clone to initialize its configuration based on the config name of the cloned object
		cno->Initialize(category);
	}
	return cno;
}

ConfNamedObject *ConfNamedObject::DoConfiguredClone(const char *category, const char *name, bool bInitializeConfig)
{
	StartTrace1(ConfNamedObject.DoConfiguredClone, "cat <" << NotNull(category) << "> name <" << fName << ">");
	ConfNamedObject *cno = (ConfNamedObject *)this->Clone();
	if ( cno ) {
		cno->SetName(name);
		cno->fCategory = fCategory;
	}
	return cno;
}

bool ConfNamedObject::DoCheckConfig(const char *category, bool bInitializeConfig)
{
	StartTrace1(ConfNamedObject.DoCheckConfig, "cat <" << NotNull(category) << "> fName <" << fName << "> &" << (long)(IFAObject *)this);
	bool bRet = false;
	if ( category ) {
		bRet = true;
		if ( bInitializeConfig || fConfig.IsNull() ) {
			Trace((bInitializeConfig ? "force-re" : "initial-") << "loading config");
			bRet = DoLoadConfig(category);
		}
	}
	return bRet;
}

bool ConfNamedObject::DoUnloadConfig()
{
	StartTrace1(ConfNamedObject.DoUnloadConfig, "cat <" << fCategory << "> fName <" << fName << "> &" << (long)(IFAObject *)this);
	// ensure that we do not access data out of CacheHandler anymore
	fConfig = ROAnything();
	return true;
}

void ConfNamedObject::SetConfig(const char *category, const char *key, ROAnything newConfig)
{
	StartTrace( ConfNamedObject.SetConfig);
	TraceAny(fConfig, "currentConfig:");
	TraceAny(newConfig, "newConfig:");
	CacheHandler *cache = CacheHandler::Get();

	AnythingLoaderPolicy alp(newConfig);
	fConfig = cache->Reload(category, key, &alp);
	TraceAny(fConfig, "adjustedConfig:");
	// ensure that the next call to CheckConfig succeeds without loading config again
	fbConfigLoaded = true;
}

bool ConfNamedObject::DoGetConfigName(const char *category, const char *objName, String &configFileName) const
{
	StartTrace1(ConfNamedObject.DoGetConfigName, "cat <" << NotNull(category) << "> fName <" << fName << "> objName <" << NotNull(objName) << ">");
	// generate config file name from object name
	if ( fConfigName.Length() ) {
		configFileName = fConfigName;
		return true;
	} else if ( objName != NULL ) {
		configFileName = objName;
		return true;
	}
	return false;
}

bool ConfNamedObject::DoLoadConfig(const char *category)
{
	StartTrace( ConfNamedObject.DoLoadConfig);
	bool bRet = false;
	String configFileName("None");

	// try to load only if a config name exists
	if ( DoGetConfigName(category, fName, configFileName) ) {
		Trace("loading cat <" << category << "> name <" << configFileName << ">");
		CacheHandler *cache = CacheHandler::Get();

		SimpleAnyLoader sal;
		fConfig = cache->Load(category, configFileName, &sal);

		SubTraceAny(TraceConfig, fConfig, "Config &" << (long)&fConfig);
		// always return true because even an empty config is a valid config
		bRet = true;
	}
	return bRet;
}

bool ConfNamedObject::DoLookup(const char *key, ROAnything &result, char delim, char indexdelim) const
{
	StartTrace1(ConfNamedObject.DoLookup, "key: <" << NotNull(key) << ">" << " Name: <" << fName << ">" );
	bool bSuccess(IsConfigLoaded());
	if ( !bSuccess ) {
		SystemLog::Warning(String("ConfNamedObject::DoLookup: failed, object <") << fName << "> of registry category <" << fCategory << "> not initialized!\n");
	} else {
		bSuccess = fConfig.LookupPath(result, key, delim, indexdelim);
	}
	TraceAny(result, "lookup for key [" << key << "] " << (bSuccess?"succeeded":"failed"));
	return bSuccess;
}

//---- HierarchConfNamed ----------------------------------------------------------

ConfNamedObject *HierarchConfNamed::DoConfiguredClone(const char *category, const char *name, bool bInitializeConfig)
{
	StartTrace1(HierarchConfNamed.DoConfiguredClone, "using [" << fName << "] as clone-base for [" << name << "]" );
	HierarchConfNamed *cno = (HierarchConfNamed *)ConfNamedObject::DoConfiguredClone(category, name, bInitializeConfig);
	if ( cno ) {
		// assign superclass, this is important to retrieve hierarchic configuration through Lookup()
		Trace("my superclass is [" << (fSuper ? fSuper->GetName() : "<unknown>") << "]");
		Trace("setting myself [" << GetName() << "] as superclass for [" << name << "]");
		cno->SetSuper(this);
	}
	return cno;
}

void HierarchConfNamed::SetSuper(HierarchConfNamed *super)
{
	fSuper = super;
}

const HierarchConfNamed *HierarchConfNamed::GetSuper() const
{
	return fSuper;
}

bool HierarchConfNamed::DoLookup(const char *key, class ROAnything &result, char delim, char indexdelim) const
{
	StartTrace1(HierarchConfNamed.DoLookup, "key: <" << NotNull(key) << ">" << " Name: <" << fName << ">" );
	if ( ConfNamedObject::DoLookup(key, result, delim, indexdelim) ) {
		TraceAny(result, "value for key [" << key << "]");
		return true;
	}
	if (fSuper) {
		return fSuper->DoLookup(key, result, delim, indexdelim);
	}
	Trace("key [" << key << "] not found");
	return false;
}

RegisterableObjectInstaller::RegisterableObjectInstaller(const char *name, const char *category, RegisterableObject *r)
	: fObject(r)
	, fCategory(category)
{
	if (fObject) {
		static bool bTrace = (System::EnvGet("COAST_TRACE_STATICALLOC") == "1");
		if ( bTrace ) {
			SystemLog::WriteToStderr(String("installing <") << name << "> into <" << category << ">\n");
		}
		fObject->Register(name, category);
		fObject->MarkStatic();
	}
}

RegisterableObjectInstaller::~RegisterableObjectInstaller()
{
	StartTrace(RegisterableObjectInstaller.~RegisterableObjectInstaller);
	if (fObject) {
		static bool bTrace = (System::EnvGet("COAST_TRACE_STATICALLOC") == "1");
		if ( bTrace ) {
			SystemLog::WriteToStderr(String("deleting <") << fObject->GetName() << "> from <" << fCategory << ">\n");
		}
		fObject->Unregister(fObject->GetName(), fCategory);
		delete fObject;
		fObject = 0;
	}
}
