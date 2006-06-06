/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _LDAPDAICachePolicyModule_H
#define _LDAPDAICachePolicyModule_H

//---- WDModule include -------------------------------------------------
#include "config_ldapdaicache.h"
#include "CacheHandler.h"

class LDAPDAICacheGetter;

//--- LDAPDAIDataAcccessLoader -----------------------------------------------
class EXPORTDECL_LDAPDAICACHE LDAPDAIDataAcccessLoader : public CacheLoadPolicy
{
public:
	LDAPDAIDataAcccessLoader();
	LDAPDAIDataAcccessLoader(Anything config);
	~LDAPDAIDataAcccessLoader();
//! Method to get a cached ldap result
//! Key must equal the name of a DataAccess definition as follows:
	virtual Anything Load(const char *lapDa);

private:
	// Instance variables
	Anything fConfig;
};

//--- LdapActionLoder --------------------------------------------------
class EXPORTDECL_LDAPDAICACHE LDAPDAIActionLoader : public CacheLoadPolicy
{
public:
	LDAPDAIActionLoader();
	LDAPDAIActionLoader(Anything config);
	~LDAPDAIActionLoader();
//! Method to get a cached ldap result
//! Key must equal the name of a DataAccess definition as follows:
	virtual Anything Load(const char *ldapDaAction);

private:
	Anything fConfig;
	// Instance variables
};

//--- LDAPDAICachePolicyModule ------------------------------------------------------
//! Configure with the name(s) of a (valid) Ldap DataAccesses
//! <PRE> { /LDAPDAIDataAccess { DA1 DA2 }
//! }</PRE>
//! or with an action triggering a Ldap DataAccess.
//! <PRE> { /LDAPDAIDataAccessAction { DA1Action DA2Action }
//! }</PRE>
//! This allows you to define multiple ldap queries to be cached on initialization.
//! Important: The called Action *must* return results in slot ["LDAPResult"][ldapDaAction]
class EXPORTDECL_LDAPDAICACHE LDAPDAICachePolicyModule : public WDModule
{
public:
	enum EDataAccessType { dataaccess, action };
	LDAPDAICachePolicyModule(const char *name);
	~LDAPDAICachePolicyModule();

	virtual bool Init(const ROAnything config);
	virtual bool Finis();
private:
	bool InitialLoad(const ROAnything dataAccesses, LDAPDAICachePolicyModule::EDataAccessType daType, const Anything &config);
//! Design by Contract: Every configured LDAPDataAccessImpl query must return data
	bool CheckContractIsFulfilled(String &failedDataAccesseses, const ROAnything dataAccesses);
	friend class LdapCacheLoaderTest;

	// Instance variables
	LDAPDAIDataAcccessLoader fLDAPDAIDataAcccessLoader;
	static LDAPDAICachePolicyModule *fgLDAPDAICachePolicyModule;
};

//---- LDAPDAICacheGetter -----------------------------------------------------------
//! Invoke with Get(<DataAccessName>,<LookupPath>or<*>);
//! Get("TestDA1",:0.DN");
//! See test cases more examples.

class EXPORTDECL_LDAPDAICACHE LDAPDAICacheGetter : public LookupInterface
{
public:
	LDAPDAICacheGetter(const String &da);
	~LDAPDAICacheGetter();

	static bool Get(ROAnything &res, const String &da, const String &key, char sepS = '.', char sepI = ':');
	static ROAnything GetAll(const String &dataAccess);

protected:
	virtual bool DoLookup(const char *key, ROAnything &result, char delim, char indexdelim) const;

private:
	String fDA;
};

#endif
