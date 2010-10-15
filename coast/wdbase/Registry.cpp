/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "Registry.h"

//--- standard modules used ----------------------------------------------------
#include "SystemLog.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//---- Registry -----------------------------------------------------------------------
void Registry::SetFinalize(bool finalize)
{
	fgFinalize = finalize;
}

Registry::Registry(const char *registryname)
	: NotCloned(registryname)
	, fTable(0)
{
	// don't set fTable to 0 because sometimes Register got called earlier
}

Registry::~Registry()
{
//	free the table if not already done
//  note: 	it is not possible to free the objects
//			in the table, since nothing is known about them
//			therefore we introduced the termination policy,
//			that can be supplied by clients
	if (fTable) {
		delete fTable;
		fTable = 0;
	}
}

bool Registry::Terminate(TerminationPolicy *terminator)
{
	StartTrace(Registry.Terminate);
	bool bRet = true;
	if ( terminator ) {
		bRet = terminator->Terminate(this);
	}
	return bRet;
}

bool Registry::Install(const ROAnything installerSpec, InstallerPolicy *ip)
{
	StartTrace1(Registry.Install, " in &" << (long)this);
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
	StatTrace(Registry.RegisterRegisterableObject, "name [" << NotNull(name) << "]", Storage::Current());
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

Anything &Registry::GetTable()
{
	if ( !fTable && !fgFinalize ) {
		fTable = new Anything(Anything::ArrayMarker(),Storage::Global());
	}
	static Anything fake; // just to let the compiler be happy
	if ( fgFinalize || !fTable ) {
		return fake;
	}

	return (*fTable);
}

//----- static accessor to global registry of Registries

Registry *Registry::GetRegistry(const char *category)
{
	StatTrace(Registry.GetRegistry, "category <" << NotNull(category) << ">", Storage::Current());
	Registry *r = (Registry *)GetRegROTable()[category].AsIFAObject(0);
	if ( !r && !fgFinalize ) {
		r = MakeRegistry(category);
	}
	return r;
}

Registry *Registry::MakeRegistry(const char *category)
{
	StatTrace(Registry.MakeRegistry, "category <" << NotNull(category) << ">", Storage::Current());
	String msg("Creating Registry for: <");
	msg.Append(NotNull(category)).Append('>');
	SystemLog::Info(msg);

	Registry *r = new (Storage::Global()) Registry(category);
	GetRegTable()[category] = Anything(r, Storage::Global()); // r stored as pointer to IFAObject (AB)
	return r;
}

Registry *Registry::RemoveRegistry(const char *category)
{
	String msg("Removing Registry for: <");
	msg << NotNull(category) << ">";
	StartTrace1(Registry.RemoveRegistry, "category <" << NotNull(category) << ">");

	SystemLog::Info(msg);

	Registry *r = 0;
	Anything a;
	if (GetRegTable().LookupPath(a, category)) {
		r = (Registry *)a.AsIFAObject(0);
		GetRegTable().Remove(category);
	}
	return r;
}

void Registry::FinalizeRegArray(Anything &registries)
{
	long sz = registries.GetSize();
	StartTrace(Registry.FinalizeRegArray);
	TraceAny(registries, "#" << sz << " registries to delete");
	for (long i = sz - 1; i >= 0; --i) {
		Registry *r = SafeCast(registries[i].AsIFAObject(0), Registry);
		if ( r ) {
			const char *pName = r->GetName();
			Trace("Registry[" << pName << "]->AliasTerminate()");
			const char *category = registries.SlotName(i);
			AliasTerminator at(NotNull(category));
			r->Terminate(&at);
			Trace("delete Registry[" << pName << "]");
			delete r;
			Trace("Registry deleted");
		}
		registries.Remove(i);
	}
}

Anything &Registry::GetRegTable()
{
	// fgRegistryArray and fgRORegistryArray have to be pointers because
	// of the initialization during the startup phase.
	// if they are simple static variables they get initialized in every
	// library (Coast, sbc and application)
	// because of that no root objects already registered are visible
	// during installation phase, which causes it to fail
	static Anything fake; // just to let the compiler be happy
	if ( !fgRegistryArray && !fgFinalize ) {
		fgRegistryArray = new Anything(Anything::ArrayMarker(),Storage::Global()); // this is the registry of Registry s
	}
	if ( fgFinalize || !fgRegistryArray ) {
		return fake;
	}

	return (*fgRegistryArray);
}

ROAnything &Registry::GetRegROTable()
{
	static ROAnything fake; //just let the compiler be happy
	if ( !fgRORegistryArray && !fgFinalize ) {
		fgRORegistryArray = new ROAnything(GetRegTable());
	}
	if ( fgFinalize || !fgRORegistryArray ) {
		return fake;
	}
	return (*fgRORegistryArray);
}

//---- Registry -----------------------------------------------------------------------
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
