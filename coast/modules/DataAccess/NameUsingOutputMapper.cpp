/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "NameUsingOutputMapper.h"
//--- standard modules used ----------------------------------------------------
#include "StringStream.h"
#include "AnythingUtils.h"
#include "Dbg.h"

//---- NameUsingOutputMapper ------------------------------------------------------------------
RegisterResultMapper(NameUsingOutputMapper);

NameUsingOutputMapper::NameUsingOutputMapper(const char *name) : EagerResultMapper(name)
{
}

IFAObject *NameUsingOutputMapper::Clone(Allocator *a) const
{
	return new (a) NameUsingOutputMapper(fName);
}

bool NameUsingOutputMapper::DoPutAny(const char *key, Anything value, Context &ctx, ROAnything config)
{
	StartTrace(NameUsingOutputMapper.DoPutAny);
	Anything dest = GetDestination(ctx, config);
	TraceAny(dest, "destination, key [" << key << "]");
	ParameterMapper::PlaceIntoAnyOrAppendIfNotEmpty(dest[key], value);
	return true;
}

bool NameUsingOutputMapper::DoPutStream(const char *key, std::istream &is, Context &ctx, ROAnything config)
{
	String strBuf;
	{
		OStringStream input(strBuf);
		input << is.rdbuf();
	}
	return DoPutAny(key, strBuf, ctx, config);
}

Anything NameUsingOutputMapper::GetDestination(Context &ctx, ROAnything config)
{
	StartTrace(NameUsingOutputMapper.GetDestination);

	ROAnything destConfig = GetDestinationConfig(ctx, config);
	TraceAny(destConfig, "destination config");
	Anything dest;
	StoreFinder::Operate(ctx, dest, destConfig);

	return dest;
}
//	where dest config is e.g.
//		/Destination {
//				/Store 		Session
//				/Slot  		AnyWithSlotnames1
//				/Delim   	CharacterDelimitingSlots
//				/IndexDelim	CharacterDelimitingIndexSlots
//		}
ROAnything NameUsingOutputMapper::GetDestinationConfig(Context &ctx, ROAnything config)
{
	StartTrace(NameUsingOutputMapper.GetDestinationConfig);
	TraceAny(fConfig, "fConfig");
	TraceAny(config, "config");
	ROAnything result;
	if (!config.LookupPath(result, "Destination")) {
		ctx.GetTmpStore()["NameUsingMapperTemp"]["Slot"] = fName;
		result = ctx.GetTmpStore()["NameUsingMapperTemp"];
	}
	TraceAny(result, "result");
	return result;
}
