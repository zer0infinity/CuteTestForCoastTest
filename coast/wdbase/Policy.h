/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _POLICY_H
#define _POLICY_H

#include "IFAConfObject.h"

//---- InstallerPolicy ----------------------------------------------------------------
/*! installer policies that install objects of a category according to a specification and the algorithm implemented in install */
class InstallerPolicy {
	InstallerPolicy();
	InstallerPolicy(const InstallerPolicy &);
	InstallerPolicy &operator=(const InstallerPolicy &);
	bool IntInitialize(Registry *r);
	void TellSuccess(bool success);

	String fCategory;
public:
	InstallerPolicy(const char *category) :
		fCategory(category) {
	}
	virtual ~InstallerPolicy() {
	}

	bool Install(const ROAnything installerSpec, Registry *r);

	const char * const GetCategory() const {
		return fCategory;
	}

protected:
	virtual bool DoInstall(const ROAnything installerSpec, Registry *r) = 0;
};

//---- TerminationPolicy ----------------------------------------------------------------
/*! installer policies that install objects of a category according to a specification and the algorithm implemented in install */
class TerminationPolicy {
	TerminationPolicy();
	TerminationPolicy(const TerminationPolicy &);
	TerminationPolicy &operator=(const TerminationPolicy &);
	bool IntFinalize(Registry *r);

	String fCategory;
public:
	TerminationPolicy(const char *category) :
		fCategory(category) {
	}
	virtual ~TerminationPolicy() {
	}

	bool Terminate(Registry *r);

	const char * const GetCategory() const {
		return fCategory;
	}

protected:
	virtual bool DoTerminate(Registry *r) = 0;
};

//---- AliasInstaller ------------------------------------------------------
/*! alias installer installs the same object with different names in the registry */
class AliasInstaller: public InstallerPolicy {
public:
	AliasInstaller(const char *category) :
		InstallerPolicy(category) {
	}

protected:
	virtual bool DoInstall(const ROAnything installerSpec, Registry *r);
};

//---- AliasTerminator ------------------------------------------------------
class AliasTerminator: public TerminationPolicy {
public:
	AliasTerminator(const char *category) :
		TerminationPolicy(category) {
	}

protected:
	virtual bool DoTerminate(Registry *r);
};

//---- HierarchyInstaller ------------------------------------------------------
/*! hierarchyInstaller installs objects that are connected by a super relation into the registry */
class HierarchyInstaller: public InstallerPolicy {
public:
	HierarchyInstaller(const char *cat) :
		InstallerPolicy(cat) {
	}

protected:
	virtual bool DoInstall(const ROAnything installerSpec, Registry *r);
	virtual void DoInitializeLeaf(const char *leafName, HierarchConfNamed *& leaf);
	HierarchConfNamed *Find(const char *name, Registry *r);
	bool HasSuper(const HierarchConfNamed *super, const char *name) const;

	bool InstallRoot(HierarchConfNamed *root, const char *name);
	bool InstallTree(HierarchConfNamed *root, const char *rootName, const ROAnything tree, Registry *r);

	HierarchConfNamed *GetLeaf(const char *leafName, HierarchConfNamed *root, Registry *r);
};

#endif
