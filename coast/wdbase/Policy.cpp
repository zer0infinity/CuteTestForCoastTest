/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "Policy.h"

//--- standard modules used ----------------------------------------------------
#include "SysLog.h"
#include "Registry.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//---- InstallerPolicy ------------------------------------------------------
bool InstallerPolicy::Install(const ROAnything installerSpec, Registry *r)
{
	StartTrace1(InstallerPolicy.DoInstall, "Category: " << GetCategory());
	SysLog::WriteToStderr(String("\t") << GetCategory());
	// let specific handler install its configured objects first
	bool bRet = DoInstall(installerSpec, r);
	bRet = IntInitialize(r);
	TellSuccess(bRet);
	return bRet;
}

bool InstallerPolicy::IntInitialize(Registry *r)
{
	StartTrace(InstallerPolicy.IntInitialize);
	// initialize remaining objects of this category
	bool bRet = true;
	RegistryIterator ri(r);
	String name;
	RegisterableObject *ro;
	while ( ri.HasMore() ) {
		ro = ri.Next(name);
		if ( ro && !ro->IsInitialized() ) {
			bool bSucc = ro->Initialize(GetCategory());
			Trace("initializing <" << name << "> was " << (bSucc ? "" : "not ") << "successful");
			if ( !bSucc ) {
				SysLog::Warning(String("initializing <") << name << "> was not successful!");
			}
			bRet = bRet && bSucc;
		}
	}
	return bRet;
}

void InstallerPolicy::TellSuccess(bool success)
{
	if ( success ) {
		SysLog::WriteToStderr(" done\n");
	} else {
		SysLog::WriteToStderr(" failed\n");
	}
}

//---- TerminationPolicy ------------------------------------------------------
bool TerminationPolicy::Terminate(Registry *r)
{
	StartTrace(TerminationPolicy.Terminate);
	bool bRet = DoTerminate(r);
	bRet = IntFinalize(r);
	return bRet;
}

bool TerminationPolicy::IntFinalize(Registry *r)
{
	StartTrace(TerminationPolicy.IntFinalize);
	// finalize remaining objects of this category
	bool bRet = true;
	RegistryIterator ri(r);
	String name;
	RegisterableObject *ro;
	while ( ri.HasMore() && ( ( ro = ri.Next(name) ) != NULL ) ) {
		Trace("checking initialized state of <" << name << ">");
		if ( ro->IsInitialized() ) {
			bool bSucc = ro->Finalize();
			Trace("finalizing <" << name << "> was " << (bSucc ? "" : "not ") << "successful");
			if ( !bSucc ) {
				SysLog::Warning(String("finalizing <") << name << "> was not successful!");
			}
			bRet = bRet && bSucc;
		}
	}
	return bRet;
}

//---- AliasInstaller ------------------------------------------------------
bool AliasInstaller::DoInstall(const ROAnything installerSpec, Registry *r)
{
	StartTrace1(AliasInstaller.DoInstall, "Category: " << GetCategory());
	long isSz = installerSpec.GetSize();
	bool installationSuccess = true;
	for (long l = 0; l < isSz; ++l) {
		const char *s = installerSpec.SlotName(l);

		Trace("Installing <" << NotNull(s) << ">");

		ROAnything a = installerSpec[l];
		for (long j = 0, sz = a.GetSize(); j < sz; ++j) {
			ROAnything k = a[j];
			RegisterableObject *t = (RegisterableObject *) r->Find(s);

			if (t) {
				const char *alias = k.AsCharPtr("");

				// clone the object with the correct name
				String origName;
				t->GetName(origName);
				t->SetName(alias);

				// installs a clone or a simple alias
				// depending on subclass of Registerable
				RegisterableObject *reg = (RegisterableObject *)t->Clone();
				Trace("\talias <" << NotNull(alias) << ">");
				reg->Register(alias, GetCategory());
				reg->Initialize(GetCategory());
				t->SetName(origName);
				SysLog::WriteToStderr(".", 1);
			} else {
				String logMsg("can't find base ");
				logMsg << s;
				SYSERROR(logMsg);
				installationSuccess = false;
			}
		}
	}
	return installationSuccess;
}

//---- AliasTerminator ------------------------------------------------------
bool AliasTerminator::DoTerminate(Registry *r)
{
	StartTrace1(AliasTerminator.DoTerminate, "Category: " << GetCategory());
	// iterate backwards over registry, since we're removing the objects
	RegistryIterator ri(r, false);

	String name;
	while ( ri.HasMore() ) {
		RegisterableObject *ro = ri.Next(name);
		if ( ro && !ro->IsStatic() ) {
			Trace("Terminating <" << name << ">");
			ro->Finalize();
			r->UnregisterRegisterableObject(name);
			delete ro;
		}
	}
	return true;
}

//---- HierarchyInstaller ------------------------------------------------------
bool HierarchyInstaller::DoInstall(const ROAnything installerSpec, Registry *r)
{
	StartTrace(HierarchyInstaller.DoInstall);
	TraceAny(installerSpec, "");
	long isSz = installerSpec.GetSize();
	bool installSuccess = true;
	for (long l = 0; l < isSz; ++l) {
		const char *s = installerSpec.SlotName(l);
		installSuccess = InstallTree(GetLeaf(s, 0, r), s, installerSpec[l], r);
	}

	return installSuccess;
}

bool HierarchyInstaller::HasSuper(HierarchConfNamed *super, const char *name)
{
	StartTrace1(HierarchyInstaller.HasSuper, "name [" << NotNull(name) << "] addr:" << (long)super);
	if (!name) {
		return false;
	}

	// get the name
	String superName;

	// check for equal names in the parent relationship
	while (super) {
		super->GetName(superName);
		Trace("current class is [" << superName << "] addr:" << (long)super);
		if ( superName == name ) {
			Trace("super class found with same name");
			return true;
		}
		super = super->GetSuper();
		Trace("next super-class is [" << (super ? super->GetName() : "<null>") << "] addr:" << (long)super);
	}
	return false;
}

HierarchConfNamed *HierarchyInstaller::Find(const char *name, Registry *r)
{
	return (HierarchConfNamed *)r->Find(name);
}

bool HierarchyInstaller::InstallRoot(HierarchConfNamed *root, const char *name)
{
	StartTrace1(HierarchyInstaller.InstallRoot, "root [" << NotNull(name) << "]" << (root ? "" : " is null!"));
	if ( root ) {
		// AB: don't create cycles in inheritance tree
		if ( HasSuper(root->GetSuper(), name) ) {
			Trace("");
			return false;
		}
		return true;
	}
	return false;
}

bool HierarchyInstaller::InstallTree(HierarchConfNamed *root, const char *rootName, const ROAnything tree, Registry *r)
{
	StartTrace(HierarchyInstaller.InstallTree);
	bool installSuccess = InstallRoot(root, rootName);
	Trace("root [" << rootName << "] success: " << (installSuccess ? "true" : "false"));
	long subTreeSz = tree.GetSize();
	for (int i = 0; i < subTreeSz; ++i) {
		bool subtree = true;
		const char *leafName = tree.SlotName(i);
		if (!leafName) {
			leafName = tree[i].AsCharPtr(0);
			subtree = false; // no more hierarchy
		}
		if (leafName) {	// there is something to install
			HierarchConfNamed *leaf = GetLeaf(leafName, root, r);
			if (leaf) {
				leaf->SetSuper(root);
				if (subtree) {
					installSuccess = InstallTree(leaf, leafName, tree[i], r) && installSuccess;
				}
			} else {
				Trace("failed leaf:" << leafName);
				installSuccess = false;
			}
		}
	}

	return installSuccess;
}

HierarchConfNamed *HierarchyInstaller::GetLeaf(const char *leafName, HierarchConfNamed *root, Registry *r)
{
	StartTrace1(HierarchyInstaller.GetLeaf, (root ? "" : "root is null!"));
	HierarchConfNamed *leaf = Find(leafName, r);
	Trace("leaf [" << leafName << "] " << (leaf ? "" : "not ") << "found in registry");
	String rootName("//");

	if ( !leaf ) {
		if (root) {
			root->GetName(rootName);
			Trace("cloning [" << rootName << "]");
			// configured cloning but without config loading yet, done below
			leaf = (HierarchConfNamed *) root->ConfiguredClone(GetCategory(), leafName, false);
		}
		if (leaf && (rootName != leafName)) {
			Trace("registering [" << leafName << "] in registry");
			r->RegisterRegisterableObject(leafName, leaf);
		}
	}
	if ( leaf ) {
		// loading of objects configuration
		leaf->Initialize(GetCategory());
	}
	SysLog::WriteToStderr(".", 1);

	return leaf;
}
