/*
 * Copyright (c) 2006, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "RenderedKeyMapper.h"
#include "Renderer.h"
#include "Dbg.h"

namespace {
	String renderKey(Context &ctx, char const *key, Anything &value) {
		String strKey(key);
		ROAnything roaKeySpec;
		if (Lookup("KeySpec", roaKeySpec)) {
			Anything anyCurrentKey(key);
			Context::PushPopEntry<Anything> aEntry(ctx, "ValuesToLookupFirst", value);
			Context::PushPopEntry<Anything> aKeyEntry(ctx, "CurrentPutKey", anyCurrentKey, "MappedKey");
			strKey = Renderer::RenderToStringWithDefault(ctx, roaKeySpec, key);
			Trace("new key [" << strKey << "]");
		}
		return strKey;
	}
	void storeKey(Context &ctx, String const& strKey) {
		ROAnything roaStoreSpec;
		if (Lookup("StoreKeyAt", roaStoreSpec)) {
			Anything anyToPut(strKey);
			StorePutter::Operate(anyToPut, ctx, Lookup("Store", "TmpStore"), Renderer::RenderToString(ctx, roaStoreSpec), false);
		}
	}
}
//---- RenderedKeyMapper ------------------------------------------------------------------
RegisterResultMapper(RenderedKeyMapper);

bool RenderedKeyMapper::DoPutAny(const char *key, Anything &value, Context &ctx, ROAnything script) {
	StartTrace1(RenderedKeyMapper.DoPutAny, NotNull(key));
	String strKey = renderKey(ctx, key, value);
	Trace("new key [" << strKey << "]");
	storeKey(ctx, strKey, key);
	return ResultMapper::DoPutAny(strKey, value, ctx, script);
}
//---- RenderedKeyParameterMapper ------------------------------------------------------------------
RegisterParameterMapper(RenderedKeyParameterMapper);

bool RenderedKeyParameterMapper::DoGetAny(const char *key, Anything &value, Context &ctx, ROAnything script) {
	StartTrace1(RenderedKeyParameterMapper.DoGetAny, NotNull(key));
	String strKey = renderKey(ctx, key, value);
	Trace("new key [" << strKey << "]");
	storeKey(ctx, strKey, key);
	return ParameterMapper::DoGetAny(strKey, value, ctx, script);
}
