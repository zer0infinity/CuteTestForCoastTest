/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _LdapCachePolicyModule_H
#define _LdapCachePolicyModule_H

#include "CacheHandler.h"

//--- LdapDataAccessLoader -----------------------------------------------
class LdapDataAccessLoader: public CacheLoadPolicy {
public:
	//! Method to get a cached ldap result
	//! Key must equal the name of a DataAccess definition as follows:
	virtual Anything Load(const char *lapDa);
};

//--- LdapActionLoder --------------------------------------------------
class LdapActionLoader: public CacheLoadPolicy {
public:
	//! Method to get a cached ldap result
	//! Key must equal the name of a DataAccess definition as follows:
	virtual Anything Load(const char *ldapDaAction);
};

//--- LdapCachePolicyModule ------------------------------------------------------
//! Configure with the name(s) of a (valid) Ldap DataAccesses
//! <PRE> { /LdapDataAccess { DA1 DA2 }
//! }</PRE>
//! or with an action triggering a Ldap DataAccess.
//! <PRE> { /LdapDataAccessAction { DA1Action DA2Action }
//! }</PRE>
//! This allows you to define multiple ldap queries to be cached on initialization.
class LdapCachePolicyModule: public WDModule {
	bool CheckContractIsFulfilled(String &failedDataAccesseses, const ROAnything dataAccesses);
	friend class LdapCacheLoaderTest;
public:
	enum EDataAccessType {
		dataaccess, action
	};
	LdapCachePolicyModule(const char *name) :
		WDModule(name) {
	}
	virtual bool Init(const ROAnything config);
	virtual bool Finis();
private:
	//! Design by Contract: Every configured LDAPDataAccessImpl query must return data
	bool InitialLoad(const ROAnything dataAccesses, LdapCachePolicyModule::EDataAccessType daType);
};

//---- LdapCacheGetter -----------------------------------------------------------
//! Invoke with Get(<DataAccessName>,<LookupPath>or<*>);
//! Get("TestDA1",:0.DN");
//! See test cases more examples.
class LdapCacheGetter: public LookupInterface {
	String fDA;
public:
	LdapCacheGetter(const String &da) :
		fDA(da) {
	}
	static bool Get(ROAnything &res, const String &da, const String &key, char sepS = '.', char sepI = ':');
	static ROAnything GetAll(const String &dataAccess);

protected:
	virtual bool DoLookup(const char *key, ROAnything &result, char delim, char indexdelim) const;
};

#endif
