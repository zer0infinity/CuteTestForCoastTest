/*
 * Copyright (c) 2006, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "RenderedKeyMapper.h"

//--- standard modules used ----------------------------------------------------
#include "Renderer.h"
#include "Dbg.h"

//---- RenderedKeyMapper ------------------------------------------------------------------
RegisterResultMapper(RenderedKeyMapper);

RenderedKeyMapper::RenderedKeyMapper(const char *name)
	: ResultMapper(name)
{
	StartTrace(RenderedKeyMapper.Ctor);
}

IFAObject *RenderedKeyMapper::Clone(Allocator *a) const
{
	return new (a) RenderedKeyMapper(fName);
}

bool RenderedKeyMapper::DoPutAny(const char *key, Anything value, Context &ctx, ROAnything script)
{
	StartTrace1(RenderedKeyMapper.DoPutAny, NotNull(key));
	String strKey(key);
	ROAnything roaKeySpec;
	if ( Lookup("KeySpec", roaKeySpec) ) {
		Context::PushPopEntry<Anything> aEntry(ctx, "ValuesToLookupFirst", value);
		strKey = Renderer::RenderToStringWithDefault(ctx, roaKeySpec, key);
		Trace("new key [" << strKey << "]");
		ROAnything roaStoreSpec;
		if ( Lookup("StoreKeyAt", roaStoreSpec) ) {
			Anything anyToPut(strKey);
			Trace("storing key in TmpStore at [" << roaStoreSpec.AsString() << "]");
			SlotPutter::Operate(anyToPut, ctx.GetTmpStore(), roaStoreSpec.AsString(), false);
		}
	}
	return ResultMapper::DoPutAny(strKey, value, ctx, script);
}

//---- RenderedKeyParameterMapper ------------------------------------------------------------------
RegisterParameterMapper(RenderedKeyParameterMapper);

RenderedKeyParameterMapper::RenderedKeyParameterMapper(const char *name)
: ParameterMapper(name)
{
	StartTrace(RenderedKeyParameterMapper.Ctor);
}

IFAObject *RenderedKeyParameterMapper::Clone(Allocator *a) const
{
	return new (a) RenderedKeyParameterMapper(fName);
}

bool RenderedKeyParameterMapper::DoGetAny(const char *key, Anything &value, Context &ctx, ROAnything script)
{
	StartTrace1(RenderedKeyMapper.DoPutAny, NotNull(key));
	String strKey(key);
	ROAnything roaKeySpec;
	if ( Lookup("KeySpec", roaKeySpec) )
	{
		Context::PushPopEntry<Anything> aEntry(ctx, "ValuesToLookupFirst", value);
		strKey = Renderer::RenderToStringWithDefault(ctx, roaKeySpec, key);
		Trace("new key [" << strKey << "]");
		ROAnything roaStoreSpec;
		if ( Lookup("StoreKeyAt", roaStoreSpec) )
		{
			Anything anyToPut(strKey);
			Trace("storing key in TmpStore at [" << roaStoreSpec.AsString() << "]");
			SlotPutter::Operate(anyToPut, ctx.GetTmpStore(), roaStoreSpec.AsString(), false);
		}
	}
	return ParameterMapper::DoGetAny(strKey, value, ctx, script);
}
