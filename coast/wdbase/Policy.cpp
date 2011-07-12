/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "SystemLog.h"
#include "Registry.h"
#include "Tracer.h"
#include "Policy.h"

//---- InstallerPolicy ------------------------------------------------------
bool InstallerPolicy::Install(const ROAnything installerSpec, Registry *r) {
	StartTrace1(InstallerPolicy.DoInstall, "Category: " << GetCategory());
	SystemLog::WriteToStderr(String("\t") << GetCategory());
	// let specific handler install its configured objects first
	bool bRet = DoInstall(installerSpec, r);
	bRet = IntInitialize(r) && bRet;
	TellSuccess(bRet);
	return bRet;
}

bool InstallerPolicy::IntInitialize(Registry *r) {
	StartTrace(InstallerPolicy.IntInitialize);
	// initialize remaining objects of this category
	bool bRet = true;
	RegistryIterator ri(r);
	String name;
	RegisterableObject *ro;
	while (ri.HasMore()) {
		ro = ri.Next(name);
		if (ro && !ro->IsInitialized()) {
			bool bSucc = ro->Initialize(GetCategory());
			Trace("initializing <" << name << "> was " << (bSucc ? "" : "not ") << "successful");
			if (!bSucc) {
				SystemLog::Warning(String("initializing <") << name << "> was not successful!");
			}
			bRet = bRet && bSucc;
		}
	}
	return bRet;
}

void InstallerPolicy::TellSuccess(bool success) {
	if (success) {
		SystemLog::WriteToStderr(" done\n");
	} else {
		SystemLog::WriteToStderr(" failed\n");
	}
}

//---- TerminationPolicy ------------------------------------------------------
bool TerminationPolicy::Terminate(Registry *r) {
	StartTrace(TerminationPolicy.Terminate);
	bool bRet = DoTerminate(r);
	bRet = IntFinalize(r) && bRet;
	return bRet;
}

bool TerminationPolicy::IntFinalize(Registry *r) {
	StartTrace(TerminationPolicy.IntFinalize);
	// finalize remaining objects of this category
	bool bRet = true;
	RegistryIterator ri(r);
	String name;
	RegisterableObject *ro;
	while (ri.HasMore() && ((ro = ri.Next(name)) != NULL)) {
		Trace("checking initialized state of <" << name << ">");
		if (ro->IsInitialized()) {
			bool bSucc = ro->Finalize();
			Trace("finalizing <" << name << "> was " << (bSucc ? "" : "not ") << "successful");
			if (!bSucc) {
				SystemLog::Warning(String("finalizing <") << name << "> was not successful!");
			}
			bRet = bRet && bSucc;
		}
	}
	return bRet;
}

//---- AliasInstaller ------------------------------------------------------
bool AliasInstaller::DoInstall(const ROAnything installerSpec, Registry *r) {
	StartTrace1(AliasInstaller.DoInstall, "Category: " << GetCategory());
	bool installationSuccess = true;
	long isSz = 0L;
	TraceAny(installerSpec, "alias config");
	// allow empty category definitions
	if ((installerSpec.GetType() == AnyArrayType) && ((isSz = installerSpec.GetSize()) >= 0L)) {
		// loop over RegisterableObjects to install aliases
		for (long l = 0; l < isSz; ++l) {
			const char *pcRegObjectName = installerSpec.SlotName(l);
			Trace("Installing aliases for RegisterableObject <" << NotNull(pcRegObjectName) << ">");

			ROAnything a = installerSpec[l];
			long sz = 0L;
			// we allow pre-definitions of /RegisterableObject slots without having aliases defined already
			if ((sz = a.GetSize()) >= 0L) {
				ROAnything k;
				for (long j = 0; j < sz && installationSuccess; ++j) {
					k = a[j];
					// only process pure value entries, no sub-anythings
					// if there is a slotname given, or the type is an anything, we dont process it
					if ((a.SlotName(j) == NULL) && (k.GetType() != AnyArrayType)) {
						RegisterableObject *t = (RegisterableObject *) r->Find(pcRegObjectName);
						if (t) {
							String alias = k.AsString();
							if (alias.Length()) {
								// clone the object with the correct name
								String origName;
								t->GetName(origName);
								t->SetName(alias);

								// installs a clone or a simple alias
								// depending on subclass of Registerable
								RegisterableObject *reg = (RegisterableObject *) t->Clone(Coast::Storage::Global());
								Trace("\talias <" << alias << ">");
								reg->Register(alias, GetCategory());
								reg->Initialize(GetCategory());
								t->SetName(origName);
								SystemLog::WriteToStderr(".", 1);
							} else {
								SYSERROR("Alias configuration error in category: " << GetCategory() << " for RegisterableObject <" << NotNull(pcRegObjectName) << ">, empty alias name!");
								installationSuccess = false;
							}
						} else {
							SYSERROR("Alias configuration error in category: " << GetCategory() << ", can't find base RegisterableObject " << pcRegObjectName);
							installationSuccess = false;
						}
					} else {
						SYSERROR("Alias configuration error in category: " << GetCategory() << " for RegisterableObject <" << NotNull(pcRegObjectName) << ">, alias name must be a string!");
						installationSuccess = false;
					}
				}
			} else {
				SYSERROR("Alias configuration error in category: " << GetCategory() << " for RegisterableObject <" << NotNull(pcRegObjectName) << ">, expected Anything!");
				installationSuccess = false;
			}
		}
	} else {
		SYSERROR("Alias configuration error for category: " << GetCategory());
		installationSuccess = false;
	}
	return installationSuccess;
}

//---- AliasTerminator ------------------------------------------------------
bool AliasTerminator::DoTerminate(Registry *r) {
	StartTrace1(AliasTerminator.DoTerminate, "Category: " << GetCategory());
	// iterate backwards over registry, since we're removing the objects
	RegistryIterator ri(r, false);

	String name;
	while (ri.HasMore()) {
		RegisterableObject *ro = ri.Next(name);
		if (ro) {
			Trace("Terminating <" << name << ">");
			ro->Finalize();
			if (not ro->IsStatic()) {
				r->UnregisterRegisterableObject(name);
				delete ro;
			}
		}
	}
	return true;
}

//---- HierarchyInstaller ------------------------------------------------------
bool HierarchyInstaller::DoInstall(const ROAnything installerSpec, Registry *r) {
	StartTrace(HierarchyInstaller.DoInstall);
	TraceAny(installerSpec, "");
	long isSz = installerSpec.GetSize();
	bool installSuccess = true;
	for (long l = 0; l < isSz; ++l) {
		const char *pcRegObjectName = installerSpec.SlotName(l);
		installSuccess = InstallTree(GetLeaf(pcRegObjectName, 0, r), pcRegObjectName, installerSpec[l], r);
	}
	return installSuccess;
}

bool HierarchyInstaller::HasSuper(const HierarchConfNamed *super, const char *name) const {
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
		if (superName == name) {
			Trace("super class found with same name");
			return true;
		}
		super = super->GetSuper();
		Trace("next super-class is [" << (super ? super->GetName() : "<null>") << "] addr:" << (long)super);
	}
	return false;
}

HierarchConfNamed *HierarchyInstaller::Find(const char *name, Registry *r) {
	return (HierarchConfNamed *) r->Find(name);
}

bool HierarchyInstaller::InstallRoot(HierarchConfNamed *root, const char *name) {
	StartTrace1(HierarchyInstaller.InstallRoot, "root [" << NotNull(name) << "]" << (root ? "" : " is null!"));
	if (root) {
		// AB: don't create cycles in inheritance tree
		if (HasSuper(root->GetSuper(), name)) {
			Trace("");
			return false;
		}
		return true;
	}
	return false;
}

bool HierarchyInstaller::InstallTree(HierarchConfNamed *root, const char *rootName, const ROAnything tree, Registry *r) {
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
		if (leafName) { // there is something to install
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

HierarchConfNamed *HierarchyInstaller::GetLeaf(const char *leafName, HierarchConfNamed *root, Registry *r) {
	StartTrace1(HierarchyInstaller.GetLeaf, (root ? "" : "root is null!"));
	HierarchConfNamed *leaf = Find(leafName, r);
	Trace("leaf [" << leafName << "] " << (leaf ? "" : "not ") << "found in registry");
	String rootName("//");

	if (!leaf) {
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
	if (leaf) {
		DoInitializeLeaf(leafName, leaf);
	}
	SystemLog::WriteToStderr(".", 1);

	return leaf;
}

void HierarchyInstaller::DoInitializeLeaf(const char *leafName, HierarchConfNamed *& leaf) {
	leaf->Initialize(GetCategory());
}
