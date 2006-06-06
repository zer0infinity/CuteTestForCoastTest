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
#include "SysLog.h"
#include "Registry.h"
#include "CacheHandler.h"
#include "System.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//---- RegisterableObject ----------------------------------------------------------
void RegisterableObject::ResetCache(bool resetCache)
{
	fgResetCache = resetCache;
}
bool RegisterableObject::fgResetCache = false;

RegisterableObject::RegisterableObject(const char *name)
	: NamedObject(name), fName(name, -1, Storage::Global()), fStaticallyInitialized(false)
{
}

// support for registerable named objects
// class API
bool RegisterableObject::Install(const ROAnything installerSpec, const char *category, InstallerPolicy *installer)
{
	// this method installs a list of clones or aliases into the registry
	// the basic assumption is that at least one object is already registered
	// with the register macro
	StartTrace(RegisterableObject.Install);
	Registry *reg = Registry::GetRegistry(category);
	TraceAny(installerSpec, "Installing Spec:");
	Trace("for Category <" << category << "> in &" << (long)reg);
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
	Registry *r;
	r = Registry::GetRegistry(category);
	if ( r ) {
		// FIXME: how to delete registered alias without
		// crashing the shutdown
		r->Terminate(terminator);
		return true;
	}
	return false;
}

bool RegisterableObject::Terminate(const char *category, TerminationPolicy *terminator)
{
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
#if !defined (_AIX) && !defined(__sun)   //static Initialisation problem
	StartTrace(RegisterableObject.Register);
#endif
	Registry *reg = Registry::GetRegistry(category);
#if !defined (_AIX) && !defined(__sun)   //static Initialisation problem
	Trace("Registering: <" << name << "> of category <" << category << "> in &" << (long)reg);
#endif
	reg->RegisterRegisterableObject(name, this);
}

void RegisterableObject::Unregister(const char *name, const char *category)
{
#if !defined (_AIX)   //static Initialisation problem
	StartTrace(RegisterableObject.Unregister);
#endif
	Registry *reg = Registry::GetRegistry(category);
	if (reg) {
#if !defined (_AIX)   //static Initialisation problem
		Trace("Unregistering: <" << name << "> of category <" << category << "> in &" << (long)reg);
#endif
		reg->UnregisterRegisterableObject(name);
	}
}

//---- ConfNamedObject ----------------------------------------------------------
ConfNamedObject::ConfNamedObject(const char *name)
	: RegisterableObject(name)
{
}

ConfNamedObject::~ConfNamedObject()
{
}

void ConfNamedObject::Register(const char *name, const char *category)
{
	StartTrace1(ConfNamedObject.Register, "cat <" << NotNull(category) << "> name <" << fName << ">");
	RegisterableObject::Register(name, category);
	CheckConfig(category);
}

ConfNamedObject *ConfNamedObject::ConfiguredClone(const char *category, const char *name, bool forceReload)
{
	StartTrace1(ConfNamedObject.ConfiguredClone, "cat <" << NotNull(category) << "> name <" << fName << ">");
	ConfNamedObject *cno = (ConfNamedObject *)this->Clone();
	if ( cno ) {
		cno->SetName(name);
		cno->CheckConfig(category, forceReload);
	}
	return cno;
}

bool ConfNamedObject::CheckConfig(const char *category, bool forceReload)
{
	StartTrace1(ConfNamedObject.CheckConfig, "cat <" << NotNull(category) << "> fName <" << fName << ">");
	if ( category ) {
		if ( forceReload || fConfig.IsNull() ) {
			Trace((forceReload ? "force-re" : "initial-") << "loading config");
			return DoLoadConfig(category);
		}
		return true;
	}
	return false;
}

bool ConfNamedObject::DoGetConfigName(const char *category, const char *objName, String &configFileName)
{
	StartTrace1(ConfNamedObject.DoGetConfigName, "cat <" << NotNull(category) << "> fName <" << fName << "> objName <" << NotNull(objName) << ">");
	if ( objName ) {
		configFileName = objName;
		return true;
	}
	return false;
}

bool ConfNamedObject::DoLoadConfig(const char *category)
{
#if !defined (_AIX)   //static Initialisation problem
	StartTrace( ConfNamedObject.DoLoadConfig);
#endif
	String configFileName("None");
	// generate config file name from object name

	if ( DoGetConfigName(category, fName, configFileName) ) {
		Trace("loading cat <" << category << "> name <" << configFileName << ">");
		// try to load only if a config name exists
		CacheHandler *cache = CacheHandler::Get();

		SimpleAnyLoader sal;
		fConfig = cache->Load(category, configFileName, &sal);
#if !defined (_AIX)   //static Initialisation problem
		SubTraceAny(TraceConfig, fConfig, "Config:");
#endif
		return (!fConfig.IsNull());
	}
	return false;
}

bool ConfNamedObject::DoLookup(const char *key, ROAnything &result, char delim, char indexdelim) const
{
	StartTrace1(ConfNamedObject.DoLookup, "key: <" << NotNull(key) << ">" << " Name: <" << fName << ">" );
	return fConfig.LookupPath(result, key, delim, indexdelim);
}

//---- HierarchConfNamed ----------------------------------------------------------

HierarchConfNamed::HierarchConfNamed(const char *name) : ConfNamedObject(name)
{
	fSuper = 0;
}

ConfNamedObject *HierarchConfNamed::ConfiguredClone(const char *category, const char *name, bool forceReload)
{
	HierarchConfNamed *cno = (HierarchConfNamed *)ConfNamedObject::ConfiguredClone(category, name);

	if ( cno ) {
		cno->SetSuper(fSuper);
	}
	return cno;
}

// hierarchical relationship API
void HierarchConfNamed::SetSuper(HierarchConfNamed *super)
{
	fSuper = super;
}

HierarchConfNamed *HierarchConfNamed::GetSuper() const
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
	: fObject(r), fName(name), fCategory(category)
{
	if (fObject) {
		if (System::EnvGet("TRACE_STATICALLOC") == "1") {
			SysLog::WriteToStderr(String("installing:<") << fName << "> into <" << fCategory << ">" << "\n");
		}
		fObject->Register(fName, fCategory);
		fObject->MarkStatic();
	}
}

RegisterableObjectInstaller::~RegisterableObjectInstaller()
{
	StartTrace(RegisterableObjectInstaller.~RegisterableObjectInstaller);
	if (fObject) {
		if (System::EnvGet("TRACE_STATICALLOC") == "1") {
			SysLog::WriteToStderr(String("deleting:<") << fName << "> from <" << fCategory << ">" << "\n");
		}
		fObject->Unregister(fName, fCategory);
		delete fObject;
		fObject = 0;
	}
}
