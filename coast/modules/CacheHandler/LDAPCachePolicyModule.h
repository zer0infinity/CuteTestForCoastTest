/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _LdapCachePolicyModule_H
#define _LdapCachePolicyModule_H

//---- WDModule include -------------------------------------------------
#include "config_cache.h"
#include "CacheHandler.h"

class LdapCacheGetter;

//--- LdapDataAccessLoader -----------------------------------------------
class EXPORTDECL_CACHE LdapDataAccessLoader : public CacheLoadPolicy
{
public:
	LdapDataAccessLoader();
	~LdapDataAccessLoader();
//! Method to get a cached ldap result
//! Key must equal the name of a DataAccess definition as follows:
	virtual Anything Load(const char *lapDa);

private:
	// Instance variables
};

//--- LdapActionLoder --------------------------------------------------
class EXPORTDECL_CACHE LdapActionLoader : public CacheLoadPolicy
{
public:
	LdapActionLoader();
	~LdapActionLoader();
//! Method to get a cached ldap result
//! Key must equal the name of a DataAccess definition as follows:
	virtual Anything Load(const char *ldapDaAction);

private:
	// Instance variables
};

//--- LdapCachePolicyModule ------------------------------------------------------
//! Configure with the name(s) of a (valid) Ldap DataAccesses
//! <PRE> { /LdapDataAccess { DA1 DA2 }
//! }</PRE>
//! or with an action triggering a Ldap DataAccess.
//! <PRE> { /LdapDataAccessAction { DA1Action DA2Action }
//! }</PRE>
//! This allows you to define multiple ldap queries to be cached on initialization.
class EXPORTDECL_CACHE LdapCachePolicyModule : public WDModule
{
public:
	enum EDataAccessType { dataaccess, action };
	LdapCachePolicyModule(const char *name);
	~LdapCachePolicyModule();

	virtual bool Init(const Anything &config);
	virtual bool Finis();
private:
	bool InitialLoad(const Anything &dataAccesses, LdapCachePolicyModule::EDataAccessType daType);
//! Design by Contract: Every configured LDAPDataAccessImpl query must return data
	bool CheckContractIsFulfilled(String &failedDataAccesseses, const Anything &dataAccesses);
	friend class LdapCacheLoaderTest;

	// Instance variables
	LdapDataAccessLoader fLdapDataAccessLoader;
	static LdapCachePolicyModule *fgLdapCachePolicyModule;
};

//---- LdapCacheGetter -----------------------------------------------------------
//! Invoke with Get(<DataAccessName>,<LookupPath>or<*>);
//! Get("TestDA1",:0.DN");
//! See test cases more examples.

class EXPORTDECL_CACHE LdapCacheGetter : public LookupInterface
{
public:
	LdapCacheGetter(const String &da);
	~LdapCacheGetter();

	static bool Get(ROAnything &res, const String &da, const String &key, char sepS = '.', char sepI = ':');
	static ROAnything GetAll(const String &dataAccess);

protected:
	virtual bool DoLookup(const char *key, ROAnything &result, char delim, char indexdelim) const;

private:
	String fDA;
};

#endif
