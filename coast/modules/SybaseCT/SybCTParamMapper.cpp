/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "SybCTParamMapper.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//---- SybCTParamMapper ------------------------------------------------------------------
RegisterParameterMapper(SybCTParamMapper);

SybCTParamMapper::SybCTParamMapper(const char *name) : ParameterMapper(name)
{
	StartTrace(SybCTParamMapper.Ctor);
}

IFAObject *SybCTParamMapper::Clone() const
{
	return new SybCTParamMapper(fName);
}

bool SybCTParamMapper::Get(const char *key, String &value, Context &ctx)
{
	StartTrace1(SybCTParamMapper.Get, NotNull(key));

	// special mapper to allow pseudo-hierarchic configuration of a mapper
	// if the key is found in context, the value of the key is taken as
	// configuration of the second Get call and used like a mapper-script
	// this can be used to specify default values in DataAccessImplMeta.any which
	// can be overridden in the configuration of the ParameterMapper therefor the
	// specification in DataAccessImplMeta MUST be in mapper-script-style

	Anything anyValue;
	if ( ParameterMapper::Get(key, anyValue, ctx) && !anyValue.IsNull() ) {
		TraceAny(anyValue, "result of first Get");
		Anything anyFinal;
		if ( ParameterMapper::DoGetAny(key, anyFinal, ctx, anyValue) && !anyFinal.IsNull() ) {
			TraceAny(anyFinal, "result of second Get");
			if (anyFinal.GetType() == Anything::eArray) {
				value = anyFinal[0L].AsCharPtr();
			} else {
				value = anyFinal.AsCharPtr();
			}
			Trace("retured value [" << value << "]");
			return true;
		}
	}
	return false;
}
