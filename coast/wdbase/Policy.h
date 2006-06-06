/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _POLICY_H
#define _POLICY_H

#include "config_wdbase.h"
#include "IFAConfObject.h"

//---- InstallerPolicy ----------------------------------------------------------------
// installer policies that install objects of a category according
// to a specification and the algorithm implemented in install
class EXPORTDECL_WDBASE InstallerPolicy
{
public:
	InstallerPolicy(const char *category) : fCategory(category) { }
	virtual ~InstallerPolicy() { }

	virtual bool Install(const ROAnything installerSpec, Registry *r) = 0;

protected:
	virtual void TellSuccess(bool success);
	String fCategory;	// short lived object... String may be allocated using the default allocator

private:
	InstallerPolicy();
	InstallerPolicy(const InstallerPolicy &);

	InstallerPolicy &operator=(const InstallerPolicy &);
};

//---- TerminationPolicy ----------------------------------------------------------------
// installer policies that install objects of a category according
// to a specification and the algorithm implemented in install
class EXPORTDECL_WDBASE TerminationPolicy
{
public:
	TerminationPolicy(const char *category) : fCategory(category) { }
	virtual ~TerminationPolicy() { }

	virtual bool Terminate(Registry *r) = 0;

protected:
	String fCategory;

private:
	TerminationPolicy();
	TerminationPolicy(const TerminationPolicy &);

	TerminationPolicy &operator=(const TerminationPolicy &);
};

//---- AliasInstaller ------------------------------------------------------
// alias installer installs the same object with different
// names in the registry
class EXPORTDECL_WDBASE AliasInstaller: public InstallerPolicy
{
public:
	AliasInstaller(const char *cat);
	virtual ~AliasInstaller();

	virtual bool Install(const ROAnything installerSpec, Registry *r);
};

//---- AliasTerminator ------------------------------------------------------
class EXPORTDECL_WDBASE AliasTerminator: public TerminationPolicy
{
public:
	AliasTerminator(const char *cat);
	virtual ~AliasTerminator();

	virtual bool Terminate(Registry *r);
};

//---- HierarchyInstaller ------------------------------------------------------
// hierarchyInstaller installs objects that are connected by a super relation
// into the registry
class EXPORTDECL_WDBASE HierarchyInstaller: public InstallerPolicy
{

public:
	HierarchyInstaller(const char *cat);
	virtual ~HierarchyInstaller();

	virtual bool Install(const ROAnything installerSpec, Registry *r);

protected:

	HierarchConfNamed *Find(const char *name, Registry *r);
	bool HasSuper(HierarchConfNamed *super, const char *name);

	bool InstallRoot(HierarchConfNamed *root, const char *name);
	bool InstallTree(HierarchConfNamed *root, const char *rootName, const ROAnything tree, Registry *r);

	HierarchConfNamed *GetLeaf(const char *leafName, HierarchConfNamed *root, Registry *r);
};

#endif
