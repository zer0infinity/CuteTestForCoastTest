/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _LDAPParams_H
#define _LDAPParams_H

#include "config_ldap.h"

#if defined(__sun) || defined(WIN32) || defined(__linux__)

//---- forward declaration -----------------------------------------------
class Anything;
class Mapper;
class Context;
typedef struct ldapmod LDAPMod;

//---- LDAPParams ----------------------------------------------------------
typedef struct timeval ldaptimeout;

//! factor out construction and use of parameters for ldap calls use only as local variable in a single thread
class EXPORTDECL_LDAP LDAPParams
{
public:
	LDAPParams(Context &ctx, ParameterMapper *in);
	virtual ~LDAPParams();

	virtual const char *Base();
	virtual ldaptimeout *Timeout();
	//!Return a NULL *  in case sec/usec == 0
	virtual ldaptimeout *Timeout4FuncCall();
	virtual int Scope();
	virtual long SizeLimit();
	virtual const char *Filter();
	virtual char **Attributes();
	virtual bool FilterDefined();

protected:
	Context &fContext;
	ParameterMapper *fMapper;

	String fBase;
	ldaptimeout fTimeout;
	long fSizeLimit;
	String fScope;
	String fFilter;
	Anything fAttributes;
	char **fLDAPAttr;

private:
	LDAPParams();
	LDAPParams(const LDAPParams &);
	LDAPParams &operator=(const LDAPParams &);
};

class EXPORTDECL_LDAP LDAPModifyCompareParams : public LDAPParams
{
public:
	LDAPModifyCompareParams(Context &ctx, ParameterMapper *in);
	virtual ~LDAPModifyCompareParams();

	virtual const char *DName();
	virtual bool DNameDefined();

	virtual char *Value2Compare();
	virtual long V2CLength();

	virtual const char *Attribute();

	virtual LDAPMod **Entries2Modify();
	virtual void DebugEntries2Modify(ostream &os);

protected:
	String fDName;
	String fValue2Compare;

	LDAPMod **fModifies;
	long fModifiesSz;
	Anything fValueList;

private:
	LDAPModifyCompareParams();
	LDAPModifyCompareParams(const LDAPModifyCompareParams &);
	LDAPModifyCompareParams &operator=(const LDAPModifyCompareParams &);
};

#endif // specific systems
#endif // _LDAPParams_H
