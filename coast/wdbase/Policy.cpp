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
void InstallerPolicy::TellSuccess(bool success)
{
	if ( success ) {
		SysLog::WriteToStderr(" done\n");
	} else {
		SysLog::WriteToStderr(" failed\n");
	}

}
//---- AliasInstaller ------------------------------------------------------
AliasInstaller::AliasInstaller(const char *category): InstallerPolicy(category) { }
AliasInstaller::~AliasInstaller() { }

bool AliasInstaller::Install(const Anything &installerSpec, Registry *r)
{
	StartTrace1(AliasInstaller.Install, "Category: " << fCategory);
	long isSz = installerSpec.GetSize();
	bool installationSuccess = true;
	SysLog::WriteToStderr(String("\t") << fCategory);
	for (long l = 0; l < isSz; ++l) {
		const char *s = installerSpec.SlotName(l);

		Trace("Installing <" << NotNull(s) << ">");

		Anything a = installerSpec[l];
		for (long j = 0, sz = a.GetSize(); j < sz; ++j) {
			Anything k = a[j];
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
				reg->Register(alias, fCategory);
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
	TellSuccess(installationSuccess);
	return installationSuccess;
}

//---- HierarchyInstaller ------------------------------------------------------
HierarchyInstaller::HierarchyInstaller(const char *cat) : InstallerPolicy(cat) { }
HierarchyInstaller::~HierarchyInstaller() { }

bool HierarchyInstaller::Install(const Anything &installerSpec, Registry *r)
{
	StartTrace(HierarchyInstaller.Install);
	TraceAny(installerSpec, "");

	long isSz = installerSpec.GetSize();
	bool installSuccess = true;
	SysLog::WriteToStderr(String("\t") << fCategory);
	for (long l = 0; l < isSz; ++l) {
		const char *s = installerSpec.SlotName(l);
		installSuccess = InstallTree(GetLeaf(s, 0, r), s, installerSpec[l], r);
	}
	TellSuccess(installSuccess);
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
		if ( superName == name ) {
			Trace("found super class [" << superName << "]");
			return true;
		}
		super = super->GetSuper();
		Trace("super-class of [" << superName << "] addr:" << (long)super);
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
		Trace("force to load configuration if not already loaded");
		root->CheckConfig(fCategory, true);
		return true;
	}
	return false;
}

bool HierarchyInstaller::InstallTree(HierarchConfNamed *root, const char *rootName, const Anything &tree, Registry *r)
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

	if (!leaf) {
		if (root) {
			root->GetName(rootName);
			Trace("clone Page, trying ConfiguredClone for [" << rootName << "]");
			leaf = (HierarchConfNamed *) root->ConfiguredClone(fCategory, leafName, true);
		}
		if (leaf && (rootName != leafName)) {
			Trace("registering leaf [" << leafName << "]");
			r->RegisterRegisterableObject(leafName, leaf);
		}
	}
	if ( leaf ) {
		SysLog::WriteToStderr(".", 1);
		leaf->CheckConfig(fCategory, true);
	}

	return leaf;
}

//---- AliasTerminator ------------------------------------------------------
AliasTerminator::AliasTerminator(const char *category): TerminationPolicy(category) { }
AliasTerminator::~AliasTerminator() { }

bool AliasTerminator::Terminate(Registry *r)
{
	StartTrace(AliasTerminator.Terminate);
	RegistryIterator ri(r, false); // iterate backwards over registry, since we're removing the objects

	String name;
	RegisterableObject *ro;

	Anything removeSet; // the set of objects that has to be removed
	while ( ri.HasMore() ) {
		ro = ri.Next(name);
		if (ro && !ro->IsStatic() ) {
			SysLog::Info(String("AliasTerminator::Terminate [") << name << "]");
			r->UnregisterRegisterableObject(name);
			String key(12L); // provide reasonable default capacity
			key << (long)ro;

			removeSet[key] = ro; // overwrites the same place if it's the same object
		}
	}

	long sz = removeSet.GetSize();

	for (long i = sz - 1; i >= 0; --i) {
		ro = (RegisterableObject *)removeSet[i].AsIFAObject(0);
		delete ro;
	}
	return true;
}
