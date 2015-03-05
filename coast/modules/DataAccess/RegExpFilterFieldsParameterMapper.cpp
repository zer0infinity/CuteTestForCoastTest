/*
 * Copyright (c) 2010, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "RegExpFilterFieldsParameterMapper.h"
#include "RE.h"
#include "AnyIterators.h"
#include "Context.h"

RegisterParameterMapper(RegExpFilterFieldsParameterMapper);

namespace {
	const char *_CombineKeyName = "CombineKey";
	const char *_MatchFlagsName = "MatchFlags";
    const char *_MappedKey = "MappedKey";
    const char *_CurrentSlotname = "Slotname";
    const char *_CombinedKeyName = "CombinedKey";
    const char *_ShortCutKey = "_WhAtEvErNaMe_";
}

template<typename ValueType>
bool RegExpFilterFieldsParameterMapper::getValueWithSlotname(const char *key, String const &strSlotname, Context & ctx, ValueType & value,
		ROAnything script) {
	StartTrace1(RegExpFilterFieldsParameterMapper.getValueWithSlotname, "key [" << NotNull(key) << "] slotname [" << strSlotname << "]");
	String strNewKey = key;
	if (Lookup(_CombineKeyName, 1L) != 0) {
		strNewKey.Append(getDelim()).Append(strSlotname);
	}
	Anything slotnameBuffer = strSlotname, combinedKeyBuffer = strNewKey;
	Context::PushPopEntry<Anything> aSlotnameEntry(ctx, "CurrentSlotname", slotnameBuffer, _CurrentSlotname);
	Context::PushPopEntry<Anything> aCombinedEntry(ctx, "CombinedKey", combinedKeyBuffer, _CombinedKeyName);
	return doGetValue(strNewKey, value, ctx, script);
}

template<typename ValueType>
bool RegExpFilterFieldsParameterMapper::getValueWithSlotnameCommon(const char *key, String const &slotname, Context & ctx, ValueType & value,
		ROAnything script, Anything &availableContent) {
	StartTrace1(RegExpFilterFieldsParameterMapper.getValueWithSlotnameCommon, "key [" << NotNull(key) << "] slotname [" << slotname << "]");
	RE aRE(slotname, static_cast<RE::eMatchFlags>(Lookup(_MatchFlagsName, 0L)));
	AnyExtensions::Iterator<Anything> slotnameIterator(availableContent);
	Anything anyValue, anyProcessed, anyCurrentKey = key;
	Context::PushPopEntry<Anything> aKeyEntry(ctx, "CurrentPutKey", anyCurrentKey, _MappedKey);
	String strSlotname;
	bool bMappingSuccess = true, bMappedValues=false;
	while ( slotnameIterator.Next(anyValue) && bMappingSuccess ) {
		if ( slotnameIterator.SlotName(strSlotname) ) {
			Trace("slotname [" << strSlotname << "]");
			if (!aRE.ContainedIn(strSlotname)){
				continue;
			}
			anyProcessed.Append(slotnameIterator.Index());
			if ( script.AsString().IsEqual("-") ) {
				continue;
			}
			TraceAny(anyValue, "pattern [" << slotname << "] matched with value:");
			bMappedValues = true;
			bMappingSuccess = getValueWithSlotname(key, strSlotname, ctx, value, script);
		}
	}
	for ( long idx = anyProcessed.GetSize()-1l; idx >= 0L; --idx ) {
		Trace("removing processed entry at index " << anyProcessed[idx].AsLong(-1L));
		availableContent.Remove(anyProcessed[idx].AsLong(-1L));
	}
	return (bMappedValues ? bMappingSuccess : Lookup("NoMatchReturnValue", 1L) > 0L );
}

template<typename ValueType>
bool RegExpFilterFieldsParameterMapper::intInterpretMapperScript(const char *key, ValueType &value, Context &ctx, ROAnything script) {
	StartTrace1(RegExpFilterFieldsParameterMapper.intInterpretMapperScript, "( \"" << NotNull(key) << "\" , ValueType &value, Context &ctx, ROAnything script)");
	Anything availableContent;
	if ( not DoFinalGetAny(key, availableContent, ctx) ) {
		return false;
	}
	Context::PushPopEntry<Anything> aSlotnameEntry(ctx, "ShortCutEntry", availableContent, _ShortCutKey);
	bool retval = true;
	// iterate over list of regular expressions specified in the slotname
	for (long i = 0, sz = script.GetSize(); retval && i < sz; ++i) {
		retval = getValueWithSlotnameCommon(key, script.SlotName(i), ctx, value, script[i], availableContent);
	}
	return retval;
}

bool RegExpFilterFieldsParameterMapper::interpretMapperScript(const char *key, Anything &value, Context &ctx, ROAnything script) {
	StartTrace1(RegExpFilterFieldsParameterMapper.interpretMapperScript, "( \"" << NotNull(key) << "\" , Anything &value, Context &ctx, ROAnything script)");
	ROAnything dummyResult;
	if ( ctx.Lookup(_ShortCutKey, dummyResult) ) {
		return ParameterMapper::interpretMapperScript(key, value, ctx, script);
	}
	return intInterpretMapperScript(key, value, ctx, script);
}

bool RegExpFilterFieldsParameterMapper::interpretMapperScript(const char *key, std::ostream &os, Context &ctx, ROAnything script) {
	StartTrace1(RegExpFilterFieldsParameterMapper.interpretMapperScript, "( \"" << NotNull(key) << "\" , std::ostream &os, Context &ctx, ROAnything script)");
	ROAnything dummyResult;
	if ( ctx.Lookup(_ShortCutKey, dummyResult) ) {
		return ParameterMapper::interpretMapperScript(key, os, ctx, script);
	}
	return intInterpretMapperScript(key, os, ctx, script);
}
