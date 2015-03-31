/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "Registry.h"
#include "SystemLog.h"
#include "Tracer.h"
#include "Policy.h"
#include "InitFinisManager.h"
#include "CacheHandler.h"

Registry::Registry(const char *category) :
		NotCloned(category), fTable(Anything::ArrayMarker(), coast::storage::Global()) {
}

Registry::~Registry() {
}

bool Registry::Terminate(TerminationPolicy *terminator)
{
	StartTrace1(Registry.Terminate, "category <" << GetName() << ">");
	bool bRet = true;
	if ( terminator ) {
		bRet = terminator->Terminate(this);
	}
	return bRet;
}

bool Registry::Install(const ROAnything installerSpec, InstallerPolicy *ip)
{
	StartTrace1(Registry.Install, "category <" << GetName() << "> in &" << (long)this);
	TraceAny(GetTable(), "Table");
	TraceAny(installerSpec, "Installer Specification");
	bool ret = false;
	if ( ip ) {
		ret = ip->Install(installerSpec, this);
	}
	TraceAny(GetTable(), "Table after Install in &" << (long)this);
	return ret;
}

RegisterableObject *Registry::Find(const char *name)
{
	// try to find object with name
	Assert(name);
	RegisterableObject *r = 0;
	// make it robust
	if ( name && GetTable().IsDefined(name) ) {
		r = (RegisterableObject *)GetTable()[name].AsIFAObject(0);
	}
	return r;
}

void Registry::RegisterRegisterableObject(const char *name, RegisterableObject *o)
{
	StatTrace(Registry.RegisterRegisterableObject, "name [" << NotNull(name) << "]", coast::storage::Current());
	Assert(name && o);
	// make it robust
	if ( name && o ) {
		GetTable()[name] = Anything(o);
	}
}

void Registry::UnregisterRegisterableObject(const char *name)
{
	StartTrace1(Registry.UnregisterRegisterableObject, "name [" << NotNull(name) << "]");
	RegisterableObject *o = Find(name);
	if ( o ) {
		Trace("object with name [" << NotNull(name) << "] found, trying to remove aliases");
		GetTable().Remove(name);
		// removing aliases
		RemoveAliases(o);
	}
}

void Registry::RemoveAliases(RegisterableObject *obj)
{
	StartTrace(Registry.RemoveAliases);
	if (obj) {
		RegistryIterator ri(this, false);
		while (ri.HasMore()) {
			String strAlias;
			RegisterableObject *alias = ri.Next(strAlias);
			if (alias && alias == obj) {
				Trace("removing object alias [" << strAlias << "]");
				GetTable().Remove(strAlias);
			}
		}
	}
}

Anything &Registry::GetTable() {
	return fTable;
}

MetaRegistryImpl::MetaRegistryImpl() :
		fRegistryArray(Anything::ArrayMarker(), coast::storage::Global()) {
	// force initializing cache handler before us
	CacheHandler::instance();
	InitFinisManager::IFMTrace("MetaRegistry::Initialized\n");
}

MetaRegistryImpl::~MetaRegistryImpl() {
	FinalizeRegArray();
	InitFinisManager::IFMTrace("MetaRegistry::Finalized\n");
}

Anything &MetaRegistryImpl::GetRegTable() {
	return fRegistryArray;
}

Registry *MetaRegistryImpl::GetRegistry(const char *category) {
	StatTrace(Registry.GetRegistry, "category <" << NotNull(category) << ">", coast::storage::Current());
	Registry *r = dynamic_cast<Registry *>(ROAnything(fRegistryArray)[category].AsIFAObject(0));
	if (not r) {
		r = MakeRegistry(category);
	}
	return r;
}

Registry *MetaRegistryImpl::MakeRegistry(const char *category) {
	StatTrace(Registry.MakeRegistry, "category <" << NotNull(category) << ">", coast::storage::Current());
	String msg("Creating Registry for: <");
	msg.Append(NotNull(category)).Append('>');
	SystemLog::Info(msg);
	Registry *r = new (coast::storage::Global()) Registry(category);
	GetRegTable()[category] = Anything(r, coast::storage::Global()); // r stored as pointer to IFAObject (AB)
	return r;
}

Registry *MetaRegistryImpl::RemoveRegistry(const char *category) {
	String msg("Removing Registry for: <");
	msg << NotNull(category) << ">";
	StartTrace1(Registry.RemoveRegistry, "category <" << NotNull(category) << ">");
	SystemLog::Info(msg);
	Registry *r = 0;
	Anything a;
	if (GetRegTable().LookupPath(a, category)) {
		r = dynamic_cast<Registry *>(a.AsIFAObject(0));
		GetRegTable().Remove(category);
	}
	return r;
}

void MetaRegistryImpl::FinalizeRegArray() {
	StartTrace(Registry.FinalizeRegArray);
	long sz = fRegistryArray.GetSize();
	TraceAny(fRegistryArray, "#" << sz << " fRegistryArray to delete");
	for (long i = sz - 1; i >= 0; --i) {
		Registry *r = dynamic_cast<Registry *>(fRegistryArray[i].AsIFAObject(0));
		if (r) {
			const char *pName = r->GetName();
			Trace("Registry[" << pName << "]->AliasTerminate()");
			const char *category = fRegistryArray.SlotName(i);
			AliasTerminator at(NotNull(category));
			r->Terminate(&at);
			Trace("delete Registry[" << pName << "]");
			delete r;
			Trace("Registry deleted");
		}
		fRegistryArray.Remove(i);
	}
}

RegistryIterator::RegistryIterator(Registry *rg, bool forward)
	: fRegistry(rg)
	, fForward(forward)
	, fStart((forward) ? -1 : -2)
	, fEnd((forward) ? -2 : -1)
{
	StartTrace(RegistryIterator.Ctor);
	if (fRegistry) {
		Anything table(fRegistry->GetTable());
		if ( fForward ) {
			fStart = 0;
			fEnd = table.GetSize() - 1;
		} else {
			fStart = table.GetSize() - 1;
			fEnd = 0;
		}
		TraceAny(table, "registry table");
	}
	Trace("fStart[" << fStart << "] fEnd[" << fEnd << "]");
}

RegistryIterator::~RegistryIterator()
{
	StartTrace(RegistryIterator.Dtor);
}

bool RegistryIterator::HasMore()
{
	StartTrace(RegistryIterator.HasMore);
	Trace("fStart[" << fStart << "] fEnd[" << fEnd << "]");
	if ( fForward ) {
		return fStart <= fEnd;
	} else {
		return fStart >= fEnd;
	}
}

RegisterableObject *RegistryIterator::Next(String &key)
{
	StartTrace(RegistryIterator.Next);
	RegisterableObject *obj = 0;
	if (fRegistry) {
		Anything table(fRegistry->GetTable());
		if (fStart > 0 && fStart > table.GetSize()) {
			fStart = table.GetSize() - 1;
		}
		if (fEnd > 0 && fEnd > table.GetSize() - 1) {
			fEnd = table.GetSize() - 1;
		}
		if (HasMore()) {
			Trace("Setting key: fStart[" << fStart << "] fEnd[" << fEnd << "]");
			key = table.SlotName(fStart);
			Trace("key<" << key << ">");
			obj = (RegisterableObject *)table[fStart].AsIFAObject(0);
			(fForward) ? ++fStart : --fStart;
		}
	}
	Trace("fStart[" << fStart << "] fEnd[" << fEnd << "]");

	return obj;
}

RegisterableObject *RegistryIterator::RemoveNext(String &key)
{
	StartTrace(RegistryIterator.RemoveNext);
	RegisterableObject *obj = Next(key);
	if (fRegistry) {
		if (HasMore()) {
			Anything table(fRegistry->GetTable());

			if (fForward) {
				--fStart;
			}
			table.Remove((fForward) ? fStart : fStart + 1);
		}
	}
	return obj;
}
