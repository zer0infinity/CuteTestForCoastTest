/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _LDAPMapper_H
#define _LDAPMapper_H

#include "config_ldap.h"
#include "Mapper.h"

#if defined(__sun) || defined(WIN32) || defined(__linux__)

class Registry;
class Context;
class ROAnything;

class EXPORTDECL_LDAP LDAPFieldMapper : public EagerParameterMapper
{
public:
	LDAPFieldMapper(const char *name);

	IFAObject *Clone() const;

public:
	//--- Conversion interface called by TRX to get data
	//! pulls out an ostream according to key
	//! Clients use this method to pull out an ostream value with name key from the Mapper
	//! \param key the name defines kind of value to get
	//! \param os the stream to be mapped
	//! \param ctx the thread context of the invocation
	//! \param info current mapper configuration
	//! \return returns true if the mapping was successful otherwise false
	virtual bool DoGetStream(const char *key, ostream &os, Context &ctx,  ROAnything info);

private:
	LDAPFieldMapper();
	LDAPFieldMapper(const LDAPFieldMapper &);
	LDAPFieldMapper &operator=(const LDAPFieldMapper &);
};

class EXPORTDECL_LDAP LDAPQueryMapper: public ParameterMapper
{
public:
	LDAPQueryMapper(const char *name);
//	virtual ~LDAPQueryMapper();

	IFAObject *Clone() const;

public:
	//--- Conversion interface called by TRX to get data
	//! pulls out an ostream according to key
	//! Clients use this method to pull out an ostream value with name key from the Mapper
	//! \param key the name defines kind of value to get
	//! \param os the stream to be mapped
	//! \param ctx the thread context of the invocation
	//! \param info current mapper configuration
	//! \return returns true if the mapping was successful otherwise false
	virtual bool DoGetStream(const char *key, ostream &os, Context &ctx,  ROAnything info);

protected:
	//! pulls out an ostream according to key
	//! Clients use this method to pull out an ostream value with name key from the Mapper
	//! \param os the ostream value which will pickup the results
	//! \param ctx the thread context of the invocation
	//! \param config the whole mapperconfig
	//! \param query the query item we use to parse
	//! \return returns true if the mapping was successful otherwise false
	bool DoGet(ostream &os, Context &ctx,  ROAnything config,  ROAnything query);
	// always use full config:
	virtual ROAnything DoSelectScript(const char *key, ROAnything script) {
		return script;
	}

private:
	LDAPQueryMapper();
	LDAPQueryMapper(const LDAPQueryMapper &);
	LDAPQueryMapper &operator=(const LDAPQueryMapper &);
};

class EXPORTDECL_LDAP LDAPDNameMapper: public ParameterMapper
{
public:
	LDAPDNameMapper(const char *name);
//	virtual ~LDAPDNameMapper();

	IFAObject *Clone() const;

public:

	//--- Conversion interface called by TRX to get data
	//! pulls out an ostream according to key
	//! Clients use this method to pull out an ostream value with name key from the Mapper
	//! \param key the name defines kind of value to get
	//! \param os the stream to be mapped
	//! \param ctx the thread context of the invocation
	//! \param info current mapper configuration
	//! \return returns true if the mapping was successful otherwise false
	virtual bool DoGetStream(const char *key, ostream &os, Context &ctx,  ROAnything info);

protected:
	bool DoGet(ostream &os, Context &ctx,  ROAnything config,  ROAnything query);
	// always use full config:
	virtual ROAnything DoSelectScript(const char *key, ROAnything script) {
		return script;
	}

private:
	LDAPDNameMapper();
	LDAPDNameMapper(const LDAPDNameMapper &);
	LDAPDNameMapper &operator=(const LDAPDNameMapper &);
};

class EXPORTDECL_LDAP LDAPParamMapper: public EagerParameterMapper
{
public:
	LDAPParamMapper(const char *name);
//	virtual ~LDAPParamMapper();

	IFAObject *Clone() const;

public:
	//--- Conversion interface called by TRX to get data
	//! pulls out an Anything according to key
	//! Clients use this method to pull out an Anything value with name key from the Mapper
	//! \param key the name defines kind of value to get
	//! \param value the value to be mapped
	//! \param ctx the thread context of the invocation
	//! \param info current mapper configuration
	//! \return returns true if the mapping was successful otherwise false
	virtual bool DoGetAny(const char *key, Anything &value, Context &ctx,  ROAnything config);

private:
	LDAPParamMapper();
	LDAPParamMapper(const LDAPParamMapper &);
	LDAPParamMapper &operator=(const LDAPParamMapper &);
};

class EXPORTDECL_LDAP LDAPModifyValsMapper: public EagerParameterMapper
{
public:
	LDAPModifyValsMapper(const char *name);
//	virtual ~LDAPModifyValsMapper();

	IFAObject *Clone() const;

public:
	//--- Conversion interface called by TRX to get data
	//! pulls out an Anything according to key
	//! Clients use this method to pull out an Anything value with name key from the Mapper
	//! \param key the name defines kind of value to get
	//! \param value the value to be mapped
	//! \param ctx the thread context of the invocation
	//! \param info current mapper configuration
	//! \return returns true if the mapping was successful otherwise false
	virtual bool DoGetAny(const char *key, Anything &value, Context &ctx,  ROAnything config);

private:
	LDAPModifyValsMapper();
	LDAPModifyValsMapper(const LDAPModifyValsMapper &);
	LDAPModifyValsMapper &operator=(const LDAPModifyValsMapper &);
};

//---- LDAPListMapper -------------------------------------------------------------------
//! <b>Store results list without transformation</b>
//!	prepares output into context tmpstore data as list of records
//! {
//!		/MapperName {
//!			{
//!				/key0	value0
//!				/key1	value1
//!				...
//!			}
//!			{
//!				/key0	value0
//!				/key1	value1
//!				...
//!			}
//!			...
//!		}
//!	}
class EXPORTDECL_LDAP LDAPListMapper : public ResultMapper
{
public:
	LDAPListMapper(const char *name);

	IFAObject *Clone() const;			//!support for prototype

protected:
	//--- Conversion interface called by TRX to put data
	//! pulls out an Anything according to key
	//! Clients use this method to put data from Dataaccess source to context
	//! \param key the name defines kind of value to put
	//! \param value the value to be mapped and put to context
	//! \param ctx the thread context of the invocation
	//! \return returns true if the mapping was successful otherwise false
	virtual bool DoFinalPutAny(const char *key, Anything value, Context &ctx);

private:
	LDAPListMapper();
	LDAPListMapper(const LDAPListMapper &);
	LDAPListMapper &operator=(const LDAPListMapper &);
};

//! <b>special mapper tranforming list into map</b>
//! Config: <pre>
//!		/PrimaryKey		Slotname of result entry to use as key for storing of entry
//!						If the same value occurs more than once, the last occurence wins.
//! </pre>
//!	prepares output into context tmpstore data as list of records
//! {
//!		/MapperName {
//!			/value0
//!			{
//!				/key0	value0
//!				/key1	value1
//!				...
//!			}
//!			/value3
//!			{
//!				/key0	value3
//!				/key1	value4
//!				...
//!			}
//!			...
//!		}
//!	}
class EXPORTDECL_LDAP LDAPListWithPrimaryKeyMapper : public ResultMapper
{
public:
	LDAPListWithPrimaryKeyMapper(const char *name);

	IFAObject *Clone() const;			// support for prototype

	virtual bool DoPutAny(const char *key, Anything value, Context &ctx, ROAnything script);
protected:
	//--- Conversion interface called by TRX to put data
	//! pulls out an Anything according to key
	//! Clients use this method to put data from Dataaccess source to context
	//! \param key the name defines kind of value to put
	//! \param value the value to be mapped and put to context
	//! \param ctx the thread context of the invocation
	//! \return returns true if the mapping was successful otherwise false
	virtual ROAnything DoSelectScript(const char *key, ROAnything script) {
		return script;
	}

private:
	LDAPListWithPrimaryKeyMapper();
	LDAPListWithPrimaryKeyMapper(const LDAPListMapper &);
	LDAPListWithPrimaryKeyMapper &operator=(const LDAPListWithPrimaryKeyMapper &);
};

#endif		//not defined _LDAPMapper_H
#endif
