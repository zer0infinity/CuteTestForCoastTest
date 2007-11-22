/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "LDAPParams.h"

//--- standard modules used ----------------------------------------------------
#include "SysLog.h"
#include "StringStream.h"
#include "Mapper.h"
#include "LDAPAPI.h"
#include "Dbg.h"

#if defined(ONLY_STD_IOSTREAM)
using namespace std;
#else
#include <stdlib.h>
#endif

//--- c-library modules used ---------------------------------------------------

//--- implementation of LDAPParams -----------------------------------------------------------
LDAPParams::LDAPParams(Context &ctx, ParameterMapper *in) : fContext(ctx), fMapper(in), fLDAPAttr(0)
{
	StartTrace(LDAPParams.LDAPParams);

	// Build basic parameters for ldap call
	Anything params;
	fMapper->Get("InputParams", params, ctx);

	TraceAny(params, "Params: ");

	Anything any;
	// let the Base be a Mapper script
	if (fMapper->DoGetAny("Base", any, ctx, params["Base"])) {
		TraceAny(any, "generated Base from Get");
		fBase = any[0L].AsString();
	}
	Trace("Base [" << fBase << "]");

	// attributes is a list of unnamed slots in an anything
	fAttributes = params["Attributes"];
	TraceAny(fAttributes, "Attributes");

	fTimeout.tv_sec = params["Timeout"].AsLong();
	Trace("Timeout [" << (long)fTimeout.tv_sec << "]");
	fTimeout.tv_usec = 0;
	fSizeLimit = params["SizeLimit"].AsLong();
	Trace("SizeLimit [" << (long)fSizeLimit << "]");

	fScope = params["Scope"].AsString();
	Trace("Scope [" << fScope << "]");

	// Builds queryfilter
	{
		OStringStream os(&fFilter);
		fMapper->Get("Filter", os, ctx);
	}

	Trace("My Search Filter [" << fFilter << "]");
}

LDAPParams::~LDAPParams()
{
	if (fLDAPAttr) {
		free(fLDAPAttr);
	}
}

const char *LDAPParams::Base()
{
	return fBase;
}

ldaptimeout *LDAPParams::Timeout()
{
	return &fTimeout;
}

ldaptimeout *LDAPParams::Timeout4FuncCall()
{
	if (fTimeout.tv_sec == 0 && fTimeout.tv_usec == 0) {
		return NULL;
	} else {
		return &fTimeout;
	}
}

int LDAPParams::Scope()
{
	if (fScope == "one") {
		return LDAP_SCOPE_ONELEVEL;
	}
	if (fScope == "base") {
		return LDAP_SCOPE_BASE;
	}
	return LDAP_SCOPE_SUBTREE;   // we resort to this if nothing is specified
}

long LDAPParams::SizeLimit()
{
	return fSizeLimit;
}

const char *LDAPParams::Filter()
{
	return fFilter;
}

char **LDAPParams::Attributes()
{
	StartTrace(LDAPParams.Attributes);
	if (!fLDAPAttr) {
		// fills up the attributes
		long sz = fAttributes.GetSize();
		fLDAPAttr = (char **) calloc(sz + 1, sizeof(char *));
		if (fLDAPAttr == NULL) {
			static const char crashmsg[] = "FATAL: LDAPParams::Attribute calloc failed. I will crash :-(\n";
			SysLog::WriteToStderr(crashmsg, sizeof(crashmsg));
		}
		for (long i = 0; i < sz; i++) {
			fLDAPAttr[i] = (char *)fAttributes[i].AsCharPtr("");
		}

		Trace("Attributes " << fLDAPAttr[0]);
	}
	return fLDAPAttr;
}

bool LDAPParams::FilterDefined()
{
	return (fFilter.Length() > 0);
}

//--- LDAPModifyCompareParams -----------------------------------

LDAPModifyCompareParams::LDAPModifyCompareParams(Context &ctx, ParameterMapper *in)
	: LDAPParams(ctx, in), fModifies(0), fModifiesSz(0)
{
	StartTrace(LDAPModifyCompareParams.LDAPModifyCompareParams);
	{
		OStringStream os(&fDName);
		in->Get("DName", os, ctx);
	}
	fDName.Append(fBase);
	Trace("Distinguished Name: " << fDName);

	{
		OStringStream os(&fValue2Compare);
		in->Get("Compare", os, ctx);
	}
}

LDAPModifyCompareParams::~LDAPModifyCompareParams()
{
	if (fModifies) {
		for ( long i = 0; i < fModifiesSz; i++ ) {
			delete [] fModifies[i]->mod_values;
			delete fModifies[i];
		}
		delete[] fModifies;
	}
}

const char *LDAPModifyCompareParams::DName()
{
	return fDName;
}

bool LDAPModifyCompareParams::DNameDefined()
{
	return (fDName.Length() > 0);
}

char *LDAPModifyCompareParams::Value2Compare()
{
	return (char *)(const char *)fValue2Compare;
}

long LDAPModifyCompareParams::V2CLength()
{
	return fValue2Compare.Length();
}

const char *LDAPModifyCompareParams::Attribute()
{
	if ( !fLDAPAttr ) {
		Attributes();
	}
	return *fLDAPAttr;
}

void LDAPModifyCompareParams::DebugEntries2Modify(ostream &os)
{
#ifdef DEBUG

	LDAPMod **modifies = Entries2Modify();
	LDAPMod *aMod;
	char **valueslist;

	while (modifies && *modifies) {
		aMod = *modifies;
		os << "op: " << aMod->mod_op;
		os << " type: <" << aMod->mod_type << "> ";
		valueslist = aMod->mod_values;
		while (*valueslist) {
			os << "<" << *valueslist << ">";
			valueslist++;
		}
		os << endl;
		modifies++;
	}
#endif
}

LDAPMod **LDAPModifyCompareParams::Entries2Modify()
{
	if ( !fModifies ) {
		fMapper->Get("ModifyVals", fValueList, fContext);

		fModifiesSz = fValueList.GetSize();

		fModifies = new LDAPMod*[ fModifiesSz + 1 ];

		LDAPMod *mod;
		char **vals = 0;

		for ( long i = 0; i < fModifiesSz; i++ ) {
			mod = new LDAPMod;
			mod->mod_op = fValueList[i]["Mode"].AsLong();
			mod->mod_type = (char *)fValueList[i]["Attribut"].AsCharPtr();
			long valuesize = fValueList[i]["Values"].GetSize();

			vals = 0;
			vals = new char*[ valuesize + 1 ];
			for ( long m = 0; m < valuesize; m++ ) {
				vals[m] = (char *)fValueList[i]["Values"][m].AsCharPtr();
			}

			vals[valuesize] = NULL;
			mod->mod_values = vals;
			fModifies[i] = mod;;
		}
		fModifies[fModifiesSz] = NULL;
	}
	return fModifies;
}
