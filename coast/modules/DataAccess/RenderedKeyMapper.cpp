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
	template<typename MapperType>
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
	template<typename MapperType>
	void storeKey(MapperType &mapper, Context &ctx, String const& strKey) {
		ROAnything roaStoreSpec;
		if (mapper.Lookup("StoreKeyAt", roaStoreSpec)) {
			Anything anyToPut(strKey);
			StorePutter::Operate(anyToPut, ctx, mapper.Lookup("Store", "TmpStore"), Renderer::RenderToString(ctx, roaStoreSpec), false);
		}
	}
}
//---- RenderedKeyResultMapper ------------------------------------------------------------------
RegisterResultMapper(RenderedKeyResultMapper);

bool RenderedKeyResultMapper::DoPutAny(const char *key, Anything &value, Context &ctx, ROAnything script) {
	StartTrace1(RenderedKeyResultMapper.DoPutAny, NotNull(key));
	String strKey = renderKey(*this, ctx, key, value);
	Trace("new key [" << strKey << "]");
	storeKey(*this, ctx, strKey);
	return ResultMapper::DoPutAny(strKey, value, ctx, script);
}

bool RenderedKeyResultMapper::DoPutStream(const char *key, std::istream & is, Context & ctx, ROAnything script) {
	StartTrace1(RenderedKeyResultMapper.DoPutStream, NotNull(key));
	Anything dummy;
	String strKey = renderKey(*this, ctx, key, dummy);
	Trace("new key [" << strKey << "]");
	storeKey(*this, ctx, strKey);
	return ResultMapper::DoPutStream(strKey, is, ctx, script);
}

ROAnything RenderedKeyResultMapper::DoSelectScript(const char *key, ROAnything script, Context &ctx) const {
	StartTrace1(RenderedKeyResultMapper.DoSelectScript, "getting key [" << NotNull(key) << "]");
	TraceAny(script, "script config");
	ROAnything roaReturn;
	if (script.IsNull() || not (script.LookupPath(roaReturn, key) || script.LookupPath(roaReturn, "*", '\0', '\0'))) {
		Trace("key not found in given script or Null-script, use hierarch-lookup mechanism now");
		roaReturn = Lookup(key);
	}
	TraceAny(roaReturn, "selected script");
	return roaReturn;
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
