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
    const char *_AllSelector = "*";
    const char _lookupPathBlocker = '\0';
    const char *_KeySpec = "KeySpec";
    const char *_StoreKeyAt = "StoreKeyAt";
    const char *_Store = "Store";
    const char *_TmpStore = "TmpStore";
    const char *_MappedKey = "MappedKey";

    template<typename MapperType>
	String renderKey(MapperType &mapper, Context &ctx, char const *key, Anything &value) {
		String strKey(key);
		ROAnything roaKeySpec;
		if (mapper.Lookup(_KeySpec, roaKeySpec)) {
			Anything anyCurrentKey(key);
			Context::PushPopEntry<Anything> aEntry(ctx, "ValuesToLookupFirst", value);
			Context::PushPopEntry<Anything> aKeyEntry(ctx, "CurrentPutKey", anyCurrentKey, _MappedKey);
			strKey = Renderer::RenderToStringWithDefault(ctx, roaKeySpec, key);
		}
		return strKey;
	}
	template<typename MapperType>
	void storeKey(MapperType &mapper, Context &ctx, String const& strKey) {
		ROAnything roaStoreSpec;
		if (mapper.Lookup(_StoreKeyAt, roaStoreSpec)) {
			Anything anyToPut(strKey);
			StorePutter::Operate(anyToPut, ctx, mapper.Lookup(_Store, _TmpStore), Renderer::RenderToString(ctx, roaStoreSpec), false);
		}
	}
	template<typename MapperType>
	ROAnything mySelectScript(MapperType &mapper, const char *key, ROAnything script, Context &ctx) {
		StartTrace1(RenderedKeyResultMapper.mySelectScript, "getting key [" << NotNull(key) << "]");
		TraceAny(script, "script config");
		ROAnything roaReturn;
		if (script.IsNull() || not (script.LookupPath(roaReturn, key, mapper.getDelim(), mapper.getIndexDelim()) || script.LookupPath(roaReturn, _AllSelector, _lookupPathBlocker, _lookupPathBlocker))) {
			Trace("key not found in given script or Null-script, use hierarch-lookup mechanism now");
			mapper.Lookup(key, roaReturn, mapper.getDelim(), mapper.getIndexDelim()) || mapper.Lookup(_AllSelector, roaReturn, _lookupPathBlocker, _lookupPathBlocker);
		}
		TraceAny(roaReturn, "selected script for new key");
		return roaReturn;
	}
}
RegisterResultMapper(RenderedKeyResultMapper);

bool RenderedKeyResultMapper::DoPutAny(const char *key, Anything &value, Context &ctx, ROAnything script) {
	StartTrace1(RenderedKeyResultMapper.DoPutAny, NotNull(key));
	String strKey = renderKey(*this, ctx, key, value);
	Trace("new key [" << strKey << "]");
	storeKey(*this, ctx, strKey);
	return DoPutAnyWithSlotname(key, value, ctx, mySelectScript(*this, strKey, script, ctx), strKey);
}

bool RenderedKeyResultMapper::DoPutStream(const char *key, std::istream & is, Context & ctx, ROAnything script) {
	StartTrace1(RenderedKeyResultMapper.DoPutStream, NotNull(key));
	Anything dummy;
	String strKey = renderKey(*this, ctx, key, dummy);
	Trace("new key [" << strKey << "]");
	storeKey(*this, ctx, strKey);
	return DoPutStreamWithSlotname(key, is, ctx, mySelectScript(*this, strKey, script, ctx), strKey);
}

bool RenderedKeyResultMapper::DoPutAnyWithSlotname(const char *key, Anything &value, Context &ctx, ROAnything roaScript, const char *slotname)
{
	StartTrace1(RenderedKeyResultMapper.DoPutAnyWithSlotname, "key [" << NotNull(key) << "] slotname [" << NotNull(slotname) << "]");
	Trace("Using slotname [" << slotname << "] as new key (not a mapper)");
	return ResultMapper::DoPutAny(slotname, value, ctx, roaScript);
}

bool RenderedKeyResultMapper::DoPutStreamWithSlotname(const char *key, std::istream &is, Context &ctx, ROAnything roaScript, const char *slotname)
{
	StartTrace1(RenderedKeyResultMapper.DoPutStreamWithSlotname, "key [" << NotNull(key) << "] slotname [" << NotNull(slotname) << "]");
	Trace("Using slotname [" << slotname << "] as new key (not a mapper)");
	return ResultMapper::DoPutStream(slotname, is, ctx, roaScript);
}

RegisterParameterMapper(RenderedKeyParameterMapper);

bool RenderedKeyParameterMapper::DoGetAny(const char *key, Anything &value, Context &ctx, ROAnything script) {
	StartTrace1(RenderedKeyParameterMapper.DoGetAny, NotNull(key));
	String strKey = renderKey(*this, ctx, key, value);
	Trace("new key [" << strKey << "]");
	storeKey(*this, ctx, strKey);
	return ParameterMapper::DoGetAny(strKey, value, ctx, script);
}
