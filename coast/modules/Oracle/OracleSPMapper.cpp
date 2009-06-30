/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "OracleSPMapper.h"

//--- project modules used -----------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

//---- OracleSPMapper ------------------------------------------------------------------
RegisterParameterMapper(OracleSPMapper);

OracleSPMapper::OracleSPMapper(const char *name)
	: ParameterMapper(name)
{
	StartTrace(OracleSPMapper.Ctor);
}

IFAObject *OracleSPMapper::Clone() const
{
	return new OracleSPMapper(fName);
}

bool OracleSPMapper::DoGetAny(const char *key, Anything &value, Context &ctx, ROAnything script)
{
	StartTrace1(OracleSPMapper.DoGetAny, NotNull(key) );

	String strKey(key);
	if ( strKey.IsEqual("SP") ) {
		// convenience to return name when we get asked if we are a SP Mapper
		return Get("Name", value, ctx);
	}
	if ( strKey.StartsWith("Params.") ) {
		strKey.Replace("Params.", "");
		return ParameterMapper::DoGetAny(strKey, value, ctx, script);
	}
	return ParameterMapper::DoGetAny(key, value, ctx, script);
}
//bool OracleSPMapper::DoGetStream(const char *key, ostream &os, Context &ctx, ROAnything script)
//{
//	StartTrace1(OracleSPMapper.DoGetStream, NotNull(key) );
//	return false;
//}
//bool OracleSPMapper::DoFinalGetAny(const char *key, Anything &value, Context &ctx)
//{
//	StartTrace1(OracleSPMapper.DoFinalGetAny,NotNull(key));
//	return false;
//}
//bool OracleSPMapper::DoFinalGetStream(const char *key, ostream &os, Context &ctx)
//{
//	StartTrace1(OracleSPMapper.DoFinalGetStream,NotNull(key));
//	return false;
//}
