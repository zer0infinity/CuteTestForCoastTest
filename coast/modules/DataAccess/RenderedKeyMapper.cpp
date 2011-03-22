/*
 * Copyright (c) 2006, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "RenderedKeyMapper.h"
#include "Renderer.h"
#include "AnythingUtils.h"

namespace {
	template <typename MapperType>
	String renderKey(MapperType &mapper, Context &ctx, char const *key, Anything &value) {
		String strKey(key);
		ROAnything roaKeySpec;
		if (mapper.Lookup("KeySpec", roaKeySpec)) {
			Anything anyCurrentKey(key);
			Context::PushPopEntry<Anything> aEntry(ctx, "ValuesToLookupFirst", value);
			Context::PushPopEntry<Anything> aKeyEntry(ctx, "CurrentPutKey", anyCurrentKey, "MappedKey");
			strKey = Renderer::RenderToStringWithDefault(ctx, roaKeySpec, key);
		}
		return strKey;
	}
	template <typename MapperType>
	void storeKey(MapperType &mapper, Context &ctx, String const& strKey) {
		ROAnything roaStoreSpec;
		if (mapper.Lookup("StoreKeyAt", roaStoreSpec)) {
			Anything anyToPut(strKey);
			StorePutter::Operate(anyToPut, ctx, mapper.Lookup("Store", "TmpStore"), Renderer::RenderToString(ctx, roaStoreSpec), false);
		}
	}
}
//---- RenderedKeyMapper ------------------------------------------------------------------
RegisterResultMapper(RenderedKeyMapper);

bool RenderedKeyMapper::DoPutAny(const char *key, Anything &value, Context &ctx, ROAnything script) {
	StartTrace1(RenderedKeyMapper.DoPutAny, NotNull(key));
	String strKey = renderKey(*this, ctx, key, value);
	Trace("new key [" << strKey << "]");
	storeKey(*this, ctx, strKey);
	return ResultMapper::DoPutAny(strKey, value, ctx, script);
}
//---- RenderedKeyParameterMapper ------------------------------------------------------------------
RegisterParameterMapper(RenderedKeyParameterMapper);

bool RenderedKeyParameterMapper::DoGetAny(const char *key, Anything &value, Context &ctx, ROAnything script) {
	StartTrace1(RenderedKeyParameterMapper.DoGetAny, NotNull(key));
	String strKey = renderKey(*this, ctx, key, value);
	Trace("new key [" << strKey << "]");
	storeKey(*this, ctx, strKey);
	return ParameterMapper::DoGetAny(strKey, value, ctx, script);
}
