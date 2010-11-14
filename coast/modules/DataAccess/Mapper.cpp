/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "Mapper.h"

//--- standard modules used ----------------------------------------------------
#include "StringStream.h"
#include "Registry.h"
#include "Dbg.h"
#include "Timers.h"

//---- MappersModule -----------------------------------------------------------
RegisterModule(MappersModule);

MappersModule::MappersModule(const char *name) : WDModule(name)
{
}

MappersModule::~MappersModule()
{
}

bool MappersModule::Init(const ROAnything config)
{
	StartTrace(MappersModule.Init);
	// installation of different mapping objects for the different backend objects
	ROAnything roaConfig;
	bool installCodeParam(true), installCodeResult(true);
	if ( config.LookupPath(roaConfig, ParameterMapper::gpcConfigPath) ) {
		HierarchyInstaller ai1(ParameterMapper::gpcCategory);
		installCodeParam = RegisterableObject::Install(roaConfig, ParameterMapper::gpcCategory, &ai1);
	}
	if ( config.LookupPath(roaConfig, ResultMapper::gpcConfigPath) ) {
		HierarchyInstaller ai2(ResultMapper::gpcCategory);
		installCodeResult = RegisterableObject::Install(roaConfig, ResultMapper::gpcCategory, &ai2);
	}
	return installCodeParam && installCodeResult;
}

bool MappersModule::ResetFinis(const ROAnything config)
{
	// installation of different mapping objects for the different backend objects
	AliasTerminator at1(ParameterMapper::gpcCategory);
	AliasTerminator at2(ResultMapper::gpcCategory);
	bool ret = RegisterableObject::ResetTerminate(ParameterMapper::gpcCategory, &at1);
	return RegisterableObject::ResetTerminate(ResultMapper::gpcCategory, &at2) && ret;
}

bool MappersModule::Finis()
{
	bool retVal = StdFinis(ParameterMapper::gpcCategory, ParameterMapper::gpcCategory);
	return StdFinis(ResultMapper::gpcCategory, ResultMapper::gpcCategory) && retVal;
}

//---- ParameterMapper ------------------------------------------------------------------------
RegisterParameterMapper(ParameterMapper);
RegisterParameterMapperAlias(Mapper, ParameterMapper);
RegCacheImpl(ParameterMapper);

const char* ParameterMapper::gpcCategory = "ParameterMapper";
const char* ParameterMapper::gpcConfigPath = "Mappers.Input";

ParameterMapper::ParameterMapper(const char *name) :
	HierarchConfNamed(name) {
}

IFAObject *ParameterMapper::Clone(Allocator *a) const
{
	return new (a) ParameterMapper(fName);
}

bool ParameterMapper::DoLoadConfig(const char *category)
{
	StartTrace(ParameterMapper.DoLoadConfig);
	Trace("category: " << category << " fName: " << fName);

	if ( HierarchConfNamed::DoLoadConfig(category) && fConfig.IsDefined(fName) ) {
		Trace("Meta-file for " << category << " found. Extracting config for " << fName);
		// mappers use only a subset of the whole configuration file
		fConfig = fConfig[fName];
		TraceAny(fConfig, "Extracted fConfig: (Returning true)");
		return true;
	}
	fConfig = Anything();
	Trace("No specific " << fName << " config found, still returning true");
	return true;
}

bool ParameterMapper::DoGetConfigName(const char *category, const char *, String &configFileName) const
{
	StartTrace(ParameterMapper.DoGetConfigName);

	if (String(ParameterMapper::gpcCategory).IsEqual(category)) {
		configFileName = "InputMapperMeta"; // keep legacy name to avoid config problems
	} else {
		configFileName = category;
		configFileName << "Meta";
	}
	return true;
}

ROAnything ParameterMapper::DoSelectScript(const char *key, ROAnything script, Context &ctx) const
{
	StartTrace1(ParameterMapper.DoSelectScript, "getting key [" << NotNull(key) << "]");
	TraceAny(script, "script config");
	ROAnything roaReturn;
	if ( script.IsNull() || !script.LookupPath(roaReturn, key) ) {
		Trace("key not found in given script or Null-script, use hierarch-lookup mechanism now");
		roaReturn = Lookup(key);
	}
	TraceAny(roaReturn, "selected script");
	return roaReturn;
}

bool ParameterMapper::Get(const char *key, int &value, Context &ctx)
{
	StartTrace1(ParameterMapper.Get, "( \"" << NotNull(key) << "\" , int &value, Context &ctx)");

	Anything anyValue;
	if ( Get(key, anyValue, ctx) ) {
		if ( !anyValue.IsNull() ) {
			value = (int)anyValue[0L].AsLong(value);
			return true;
		}
	}
	return false;
}

#if !defined(BOOL_NOT_SUPPORTED)
bool ParameterMapper::Get(const char *key, bool &value, Context &ctx)
{
	StartTrace1(ParameterMapper.Get, "( \"" << NotNull(key) << "\" , bool &value, Context &ctx)");

	Anything anyValue;
	if ( Get(key, anyValue, ctx) ) {
		if ( !anyValue.IsNull() ) {
			TraceAny(anyValue, "value-any");
			value = anyValue[0L].AsBool(value);
			Trace("returning: " << value);
			return true;
		}
	}
	return false;
}
#endif

bool ParameterMapper::Get(const char *key, long &value, Context &ctx)
{
	StartTrace1(ParameterMapper.Get, "( \"" << NotNull(key) << "\" , long &value, Context &ctx)");

	Anything anyValue;
	if ( Get(key, anyValue, ctx) ) {
		if ( !anyValue.IsNull() ) {
			TraceAny(anyValue, "value-any");
			value = anyValue[0L].AsLong();
			Trace("returning: " << value);
			return true;
		}
		Trace("value not found, returning false");
	}
	return false;
}

bool ParameterMapper::Get(const char *key, float &value, Context &ctx)
{
	StartTrace1(ParameterMapper.Get, "( \"" << NotNull(key) << "\" , float &value, Context &ctx)");

	Anything anyValue;
	if ( Get(key, anyValue, ctx) ) {
		if ( !anyValue.IsNull() ) {
			value = (float)anyValue[0L].AsDouble(value);
			return true;
		}
	}
	return false;
}

bool ParameterMapper::Get(const char *key, double &value, Context &ctx)
{
	StartTrace1(ParameterMapper.Get, "( \"" << NotNull(key) << "\" , double &value, Context &ctx)");

	Anything anyValue;
	if ( Get(key, anyValue, ctx) ) {
		if ( !anyValue.IsNull() ) {
			value = anyValue[0L].AsDouble(value);
			return true;
		}
	}
	return false;
}

bool ParameterMapper::Get(const char *key, String &value, Context &ctx)
{
	StartTrace1(ParameterMapper.Get, "( \"" << NotNull(key) << "\" , String &value, Context &ctx)");

	Anything anyValue;
	if ( Get(key, anyValue, ctx) ) {
		if ( !anyValue.IsNull() ) {
			value = anyValue[0L].AsString();
			return true;
		}
	}
	return false;
}

bool ParameterMapper::Get(const char *key, Anything &value, Context &ctx)
{
	StartTrace1(ParameterMapper.Get, "( \"" << NotNull(key) << "\" , Anything &value, Context &ctx)");
	DAAccessTimer(ParameterMapper.Get, key << " (Anything)", ctx);
	Anything anyValue;
	if (DoGetAny(key, anyValue, ctx, SelectScript(key, fConfig, ctx))) {
		value = anyValue;
		TraceAny(value, "returned value");
		return true;
	}
	return false;
}

Anything ParameterMapper::Get(const char *key, Context &ctx)
{
	StartTrace1(ParameterMapper.Get, "( \"" << NotNull(key) << "\" , Context &ctx)");
	DAAccessTimer(ParameterMapper.Get, key << " as Anything", ctx);
	Anything anyValue;
	DoGetAny(key, anyValue, ctx, SelectScript(key, fConfig, ctx));
	return anyValue;
}

bool ParameterMapper::Get(const char *key, std::ostream &os, Context &ctx)
{
	StartTrace1(ParameterMapper.Get, "( \"" << NotNull(key) << "\" , ostream &os, Context &ctx)");
	DAAccessTimer(ParameterMapper.Get, key << " (stream)", ctx);
	return DoGetStream(key, os, ctx, SelectScript(key, fConfig, ctx));
}

// convenience method
void ParameterMapper::PlaceIntoAnyOrAppendIfNotEmpty(Anything &var, ROAnything theValue)
{
	StartTrace(ParameterMapper.PlaceIntoAnyOrAppendIfNotEmpty);
	if (var.IsNull()) {
		var = theValue.DeepClone();
		TraceAny(theValue, "Placing value into Anything...");
	} else {
		var.Append(theValue.DeepClone());
		TraceAny(theValue, "Appending value to given Anything");
	}
}

bool ParameterMapper::DoGetAny(const char *key, Anything &value, Context &ctx, ROAnything script)
{
	StartTrace1(ParameterMapper.DoGetAny, "( \"" << NotNull(key) << "\" , Anything &value, Context &ctx, ROAnything script)");

	if (script.IsNull() || script.GetSize() == 0) {
		// no more script to run
		Trace("Script is empty or null...");
		return DoFinalGetAny(key, value, ctx);
	}

	if (script.GetType() != AnyArrayType) {
		// we found a simple value, append it, Append because of scripting
		Trace("Script is a simple value:" << script.AsString());
		PlaceIntoAnyOrAppendIfNotEmpty(value, script);
		return true;
	}

	TraceAny(script, "Got a script. Starting interpretation foreach slot...");
	// now for the scripting case, similar to Renderers
	// interpret as long as you get return values. stop if you don't
	bool retval = true;
	for (long i = 0, sz = script.GetSize(); retval && i < sz; ++i) {
		String slotname(script.SlotName(i));
		ParameterMapper *m;
		if (slotname.Length() <= 0) {
			Trace("Anonymous slot, call myself again with script[" << i << "]");
			retval = DoGetAny(key, value, ctx, script[i]);
		} else if ((m = ParameterMapper::FindParameterMapper(slotname))) {
			Trace("Slotname equals mapper: " << slotname);
			if (script[i].IsNull()) {
				// no more config in script, use original mappers config
				// fallback to orignal mapping with m's config
				Trace("Slotval is null. Calling " << slotname << " with it's default config...");
				retval = m->DoGetAny(key, value, ctx, m->SelectScript(key, m->fConfig, ctx));
			} else {
				Trace("Calling " << slotname << " with script[" << i << "][\"" << NotNull(key) << "\"]...");
				retval = m->DoGetAny(key, value, ctx, m->SelectScript(key, script[i], ctx));
			}
		} else {
			Trace("Slotname " << slotname << " is not a mapper (not found).");
			Trace("Using slot as new key and call myself again with script[" << i << "]");
			retval = DoGetAny(slotname, value, ctx, script[i]);
		}
	}
	return retval;
}
bool ParameterMapper::DoGetStream(const char *key, std::ostream &os, Context &ctx, ROAnything script)
{
	StartTrace1(ParameterMapper.DoGetStream, "( \"" << NotNull(key) << "\" , ostream &os, Context &ctx, ROAnything script)");

	if (script.IsNull() || script.GetSize() == 0) {
		// no more script to run
		Trace("Script is empty or null...");
		return DoFinalGetStream(key, os, ctx);
	}

	if (script.GetType() != AnyArrayType) {
		// we found a simple value, append it, Append because of scripting
		Trace("Script is a simple value, write it on stream...");
		os << script.AsString();
		return true;
	}

	// now for the scripting case, similar to Renderers
	TraceAny(script, "Got a script. Starting interpretation foreach slot...");
	bool retval = true;
	for (long i = 0, sz = script.GetSize(); retval && i < sz; ++i) {
		String slotname(script.SlotName(i));
		ParameterMapper *m;
		if (slotname.Length() <= 0) {
			Trace("Anonymous slot, call myself again with script[" << i << "]");
			retval = DoGetStream(key, os, ctx, script[i]);
		} else if ((m = ParameterMapper::FindParameterMapper(slotname))) {
			Trace("Slotname equals mapper: " << slotname);
			if (script[i].IsNull()) {
				// no more config in script, use original mappers config
				// fallback to orignal mapping with m's config
				Trace("Slotval is null. Calling " << slotname << " with it's default config...");
				retval = m->DoGetStream(key, os, ctx, m->SelectScript(key, m->fConfig, ctx));
			} else {
				Trace("Calling " << slotname << " with script[" << i << "][\"" << NotNull(key) << "\"]...");
				retval = m->DoGetStream(key, os, ctx, m->SelectScript(key, script[i], ctx));
			}
		} else {
			Trace("Slotname " << slotname << " is not a mapper (not found).");
			Trace("Using slot as new key and call myself again with script[" << i << "]");
			retval = DoGetStream(slotname, os, ctx, script[i]);
		}
	}
	return retval;
}

bool ParameterMapper::DoFinalGetAny(const char *key, Anything &value, Context &ctx)
{
	StartTrace1(ParameterMapper.DoFinalGetAny, NotNull(key));
	String sourceSlot = GetSourceSlot(ctx);
	if ( sourceSlot.Length() > 0L ) {
		sourceSlot.Append(getDelim());
	}
	sourceSlot.Append(key);
	ROAnything ctxValue;
	if ( ctx.Lookup(sourceSlot, ctxValue, getDelim(), getIndexDelim()) ) {
		Trace("Found key [" << sourceSlot << "] in context...");
		PlaceIntoAnyOrAppendIfNotEmpty(value, ctxValue);
		return true;
	}
	Trace("Key [" << sourceSlot << "] not found in context. Failing.");
	return false;
}

namespace {
	static void PlaceAnyOnStream(std::ostream &os, ROAnything value) {
		if (value.GetType() == AnyArrayType) {
			os << value;
		} else {
			os << value.AsString();
		}
	}
}

bool ParameterMapper::DoFinalGetStream(const char *key, std::ostream &os, Context &ctx)
{
	StartTrace1(ParameterMapper.DoFinalGetStream, NotNull(key));
	String sourceSlot = GetSourceSlot(ctx);
	if ( sourceSlot.Length() > 0L ) {
		sourceSlot.Append(getDelim());
	}
	sourceSlot.Append(key);
	ROAnything ctxValue;
	if ( ctx.Lookup(sourceSlot, ctxValue, getDelim(), getIndexDelim()) ) {
		Trace("Found key [" << sourceSlot << "] in context with value [" << ctxValue.AsString() << "]");
		Trace("Putting value onto return stream...");
		PlaceAnyOnStream(os, ctxValue);
		return true;
	}
	Trace("Key [" << sourceSlot << "] not found in context. Failing.");
	return false;
}

String ParameterMapper::DoGetSourceSlot(Context &ctx) const
{
	ROAnything slotname;
	String slotnamename(fName, Storage::Current());
	slotnamename.Append(".SourceSlot");
	return (Lookup("SourceSlot", slotname) || ctx.Lookup(slotnamename, slotname)) ? slotname.AsString() : String();
}

//---- EagerParameterMapper ------------------------------------------------
RegisterParameterMapper(EagerParameterMapper);

EagerParameterMapper::EagerParameterMapper(const char *name, ROAnything config) :
	ParameterMapper(name) {
	fConfig = config;
}

ROAnything EagerParameterMapper::DoSelectScript(const char *key, ROAnything script, Context &ctx) const
{
	ROAnything roaReturn(ParameterMapper::DoSelectScript(key, script, ctx));
	if ( roaReturn.IsNull() ) {
		roaReturn = script;
	}
	return roaReturn;
}

//---- ResultMapper ----------------------------------------------------------------
RegisterResultMapper(ResultMapper);
RegisterResultMapperAlias(Mapper, ResultMapper);
RegCacheImpl(ResultMapper);	// FindResultMapper()

const char* ResultMapper::gpcCategory = "ResultMapper";
const char* ResultMapper::gpcConfigPath = "Mappers.Output";

ResultMapper::ResultMapper(const char *name) :
	HierarchConfNamed(name) {
}

IFAObject *ResultMapper::Clone(Allocator *a) const
{
	return new (a) ResultMapper(fName);
}

ROAnything ResultMapper::DoSelectScript(const char *key, ROAnything script, Context &ctx) const
{
	StartTrace1(ResultMapper.DoSelectScript, "getting key [" << NotNull(key) << "]");
	TraceAny(script, "script config");
	ROAnything roaReturn;
	if ( script.IsNull() || !script.LookupPath(roaReturn, key) ) {
		Trace("key not found in given script or Null-script, use hierarch-lookup mechanism now");
		roaReturn = Lookup(key);
	}
	TraceAny(roaReturn, "selected script");
	return roaReturn;
}

bool ResultMapper::DoLoadConfig(const char *category)
{
	StartTrace(ResultMapper.DoLoadConfig);
	Trace("category: " << category << " fName: " << fName);

	if ( HierarchConfNamed::DoLoadConfig(category) && fConfig.IsDefined(fName) ) {
		TraceAny(fConfig, "fConfig before: ");
		// mappers use only a subset of the whole configuration file
		fConfig = fConfig[fName];
		TraceAny(fConfig, "new fConfig:");
		return true;
	}
	fConfig = Anything();
	Trace("No specific " << fName << " config found, still returning true");
	return true;
}

bool ResultMapper::DoGetConfigName(const char *category, const char *objName, String &configFileName) const
{
	StartTrace(ResultMapper.DoGetConfigName);

	if (String(ResultMapper::gpcCategory).IsEqual(category)) {
		configFileName = "OutputMapperMeta";    // keep legacy name to avoid config problems
	} else {
		configFileName = (String(category) << "Meta");
	}
	return true;
}

#if !defined(BOOL_NOT_SUPPORTED)
bool ResultMapper::Put(const char *key, bool value, Context &ctx)
{
	StartTrace(ResultMapper.Put);
	return Put(key, Anything(value), ctx);
}
#endif

bool ResultMapper::Put(const char *key, int value, Context &ctx)
{
	StartTrace(ResultMapper.Put);
	return Put(key, Anything(value), ctx);
}
bool ResultMapper::Put(const char *key, long value, Context &ctx)
{
	StartTrace(ResultMapper.Put);
	return Put(key, Anything(value), ctx);
}

bool ResultMapper::Put(const char *key, float value, Context &ctx)
{
	StartTrace(ResultMapper.Put);
	return Put(key, Anything(value), ctx);
}

bool ResultMapper::Put(const char *key, double value, Context &ctx)
{
	StartTrace(ResultMapper.Put);
	return Put(key, Anything(value), ctx);
}

bool ResultMapper::Put(const char *key, String value, Context &ctx)
{
	StartTrace(ResultMapper.Put);
	return Put(key, Anything(value), ctx);
}

bool ResultMapper::Put(const char *key, Anything value, Context &ctx)
{
	StartTrace(ResultMapper.Put);
	DAAccessTimer(ResultMapper.Put, key, ctx);
	return DoPutAny(key, value, ctx, SelectScript(key, fConfig, ctx));
}

bool ResultMapper::Put(const char *key, std::istream &is, Context &ctx)
{
	StartTrace1(ResultMapper.Put, NotNull(key));
	DAAccessTimer(ResultMapper.Put, key, ctx);
	return DoPutStream(key, is, ctx, SelectScript(key, fConfig, ctx));
}

bool ResultMapper::DoPutAny(const char *key, Anything &value, Context &ctx, ROAnything script)
{
	StartTrace1(ResultMapper.DoPutAny, NotNull(key));
	bool retval = true;

	// force setting a possibly given DestinationSlot from config into the context
	// -> this allows sub-mappers, eg executed in script mode, to put into the same slot if not overridden again
	Anything anyPath;
	anyPath["ResultMapper"]["DestinationSlot"] = GetDestinationSlot(ctx);
	Context::PushPopEntry<Anything> aEntry(ctx, "DestSlotMarker", anyPath);

	if (script.IsNull() || script.GetSize() == 0) {
		// no more script to run
		Trace("Script is empty or null...");
		retval = DoFinalPutAny(key, value, ctx);
	} else if (script.GetType() != AnyArrayType) {
		// we found a simple value in script use it as a new key in final put
		Trace("Script is a simple value:" << script.AsString());
		retval = DoFinalPutAny(script.AsCharPtr(key), value, ctx);
	} else {
		// now for the scripting case, similar to Renderers
		TraceAny(script, "Got a script. Starting interpretation foreach slot...");

		for (long i = 0, sz = script.GetSize(); retval && i < sz; ++i) {
			String slotname(script.SlotName(i));
			ResultMapper *m;
			ROAnything roaScript(script[i]);
			if (slotname.Length() <= 0) {
				Trace("Anonymous slot, call myself again with script[" << i << "]");
				retval = DoPutAny(key, value, ctx, roaScript);
			} else if ((m = ResultMapper::FindResultMapper(slotname))) {
				Trace("Slotname equals mapper: " << slotname);
				if ( roaScript.IsNull() ) {
					// fallback to mappers original config
					Trace("Calling " << slotname << " with it's default config...");
					retval = m->Put(key, value, ctx);
				} else {
					Trace("Calling " << slotname << " with script[" << i << "][\"" << NotNull(key) << "\"]...");
					retval = m->DoPutAny(key, value, ctx, m->SelectScript(key, roaScript, ctx));
				}
			} else {
				Trace("Using slotname [" << slotname << "] as new key (not a mapper)");
				retval = DoPutAnyWithSlotname(key, value, ctx, roaScript, slotname);
			}
		}
	}
	// store the base destination slot in temp store now
	ctx.GetTmpStore()["ResultMapper"]["DestinationSlot"] = anyPath["ResultMapper"]["DestinationSlot"].AsString();
	return retval;
}

bool ResultMapper::DoPutAnyWithSlotname(const char *key, Anything &value, Context &ctx, ROAnything roaScript, const char *slotname)
{
	StartTrace1(ResultMapper.DoPutAnyWithSlotname, "key [" << NotNull(key) << "] slotname [" << NotNull(slotname) << "]");
	Trace("Using slotname [" << slotname << "] as new key (not a mapper)");
	return DoPutAny(slotname, value, ctx, roaScript);
}

bool ResultMapper::DoPutStream(const char *key, std::istream &is, Context &ctx, ROAnything script)
{
	StartTrace1(ResultMapper.DoPutStream, NotNull(key));
	bool retval = true;

	// force setting a possibly given DestinationSlot from config into the context
	// -> this allows sub-mappers, eg executed in script mode, to put into the same slot if not overridden again
	Anything anyPath;
	anyPath["ResultMapper"]["DestinationSlot"] = GetDestinationSlot(ctx);
	Context::PushPopEntry<Anything> aEntry(ctx, "DestSlotMarker", anyPath);

	if (script.IsNull() || script.GetSize() == 0) {
		// no more script to run
		Trace("Script is empty or null...");
		retval = DoFinalPutStream(key, is, ctx);
	} else if (script.GetType() != AnyArrayType) {
		// we found a simple value in script use it as a new key in final put
		Trace("Script is a simple value:" << script.AsString());
		retval = DoFinalPutStream(script.AsCharPtr(key), is, ctx);
	} else {
		// now for the scripting case, similar to Renderers
		TraceAny(script, "Got a script. Starting interpretation foreach slot...");

		for (long i = 0, sz = script.GetSize(); retval && i < sz; ++i) {
			String slotname(script.SlotName(i));
			ResultMapper *m;
			ROAnything roaScript(script[i]);
			if (slotname.Length() <= 0) {
				Trace("Anonymous slot, call myself again with script[" << i << "]");
				retval = DoPutStream(key, is, ctx, roaScript);
			} else if ((m = ResultMapper::FindResultMapper(slotname))) {
				Trace("Slotname equals mapper: " << slotname);
				if ( roaScript.IsNull() ) {
					// fallback to mappers original config
					Trace("Calling " << slotname << " with it's default config...");
					retval = m->Put(key, is, ctx);
				} else {
					Trace("Calling " << slotname << " with script[" << i << "][\"" << NotNull(key) << "\"]");
					retval = m->DoPutStream(key, is, ctx, m->SelectScript(key, roaScript, ctx));
				}
			} else {
				Trace("Using slotname [" << slotname << "] as new key (not a mapper)");
				retval = DoPutStream(slotname, is, ctx, roaScript);
			}
		}
	}
	// store the base destination slot in temp store now
	ctx.GetTmpStore()["ResultMapper"]["DestinationSlot"] = anyPath["ResultMapper"]["DestinationSlot"].AsString();
	return retval;
}

void ResultMapper::DoGetDestinationAny(const char *key, Anything &targetAny, Context &ctx)
{
	StartTrace1(ResultMapper.DoGetDestinationAny, NotNull(key));
	String path = GetDestinationSlot(ctx), kPrefix(key);
	if (path.Length() > 0 && kPrefix.Length()) {
		path.Append(getDelim());
	}
	path.Append(kPrefix);
	Trace("Path for slotfinder: [" << path << "]");
	if (path.Length() > 0) {
		SlotFinder::Operate(ctx.GetTmpStore(), targetAny, path, getDelim(), getIndexDelim());
	} else {
		targetAny = ctx.GetTmpStore();
	}
}

bool ResultMapper::DoFinalPutAny(const char *key, Anything &value, Context &ctx)
{
	StartTrace1(ResultMapper.DoFinalPutAny, "fName [" << fName << "] key: <" << NotNull(key) << ">");

	String kStr(key);
	if (kStr.Length() <= 0) {
		return false;    // empty key not allowed
	}
	// check if key is a path, if so, split it into prefix and key
	long idx = kStr.StrRChr(getDelim());
	String kPrefix, kKey(key);
	if ( idx > -1 ) {
		Trace("Key is a path: " << kKey);
		kPrefix << kStr.SubString(0, idx);
		kKey = kStr.SubString(idx + 1);
		Trace("Prefix: " << kPrefix);
		Trace("Key: " << kKey);
	}
	Trace("Key for put: " << kKey);
	Trace("Initial key: " << key);

	Anything anyTarget;
	DoGetDestinationAny(kPrefix, anyTarget, ctx);
	TraceAny(anyTarget, "store before");
	String strPutPolicy = Lookup("PutPolicy", ( anyTarget.IsDefined(kKey) ? "Append" : "Put" ) );
	if ( strPutPolicy == "Append" ) {
		Trace("appending value");
		anyTarget[kKey].Append(value);
	} else {
		TraceAny(value, "replacing value at [" << kKey << "]");
		anyTarget[kKey] = value;
	}
	TraceAny(anyTarget, "store after");
	return true;
}

bool ResultMapper::DoFinalPutStream(const char *key, std::istream &is, Context &ctx)
{
	StartTrace1(ResultMapper.DoFinalPutStream, NotNull(key));
	String strBuf;
	{
		OStringStream input(strBuf);
		input << is.rdbuf();
	}
	Trace(strBuf);
	Anything anyVal(strBuf);
	return DoFinalPutAny(key, anyVal, ctx);
}

String ResultMapper::GetDestinationSlot(Context &ctx)
{
	StartTrace1(ResultMapper.GetDestinationSlot, "fName [" << fName << "]");
	const char *pcDefault = "Mapper";
	// get current Destination slot if any
	const char *pcCurrent = ctx.Lookup("ResultMapper.DestinationSlot", pcDefault);
	// keep the current destination slot if it is not overridden
	String strDestSlot = DoGetDestinationSlot(ctx, pcCurrent);
	Trace("destination slot is [" << strDestSlot << "]");
	return strDestSlot;
}

String ResultMapper::DoGetDestinationSlot(Context &ctx, const char *pcDefault)
{
	StartTrace1(ResultMapper.DoGetDestinationSlot, "fName [" << fName << "] default value [" << pcDefault << "]");
	ROAnything roaDest;
	String slotnamename(fName, Storage::Current());
	slotnamename.Append(".DestinationSlot");
	TraceAny(fConfig, "my configuration for Lookup");
	if ( !Lookup("DestinationSlot", roaDest) ) {
		Trace("doing ctx.Lookup on [" << slotnamename << "]");
		ctx.Lookup(slotnamename, roaDest);
	}
	String strDestSlot = (!roaDest.IsNull() ? roaDest.AsCharPtr() : pcDefault);
	Trace("destination slot is [" << strDestSlot << "]");
	return strDestSlot;
}

// -------------------------- EagerResultMapper -------------------------
RegisterResultMapper(EagerResultMapper);

EagerResultMapper::EagerResultMapper(const char *name, ROAnything config)
	: ResultMapper(name)
{
	fConfig = config;
}

ROAnything EagerResultMapper::DoSelectScript(const char *key, ROAnything script, Context &ctx) const
{
	ROAnything roaReturn(ResultMapper::DoSelectScript(key, script, ctx));
	if ( roaReturn.IsNull() ) {
		roaReturn = script;
	}
	return roaReturn;
}

// ========================== Other Mappers =================================

// -------------------------- RootMapper -------------------------------
RegisterResultMapper(RootMapper);

// -------------------------- ConfigMapper -----------------------------
RegisterParameterMapper(ConfigMapper);

bool ConfigMapper::DoGetAny(const char *key, Anything &value, Context &ctx, ROAnything config)
{
	StartTrace(ConfigMapper.DoGetAny);
	// step recursively through config and check, if we need to restart scripting
	if ( ctx.Lookup("DoDefaultDoGetAny", 0L) ) {
		ParameterMapper::DoGetAny(key, value, ctx, config);
	} else {
		EvaluateConfig(config, value, ctx);
	}
	return true;
}

void ConfigMapper::EvaluateConfig(ROAnything config, Anything &value, Context &ctx)
{
	StartTrace(ConfigMapper.EvaluateConfig);
	TraceAny(config, "current config");
	if (config.GetType() == AnyArrayType) {
		const String strMS("MapperScript");
		const Anything anyBreak(1L);
		for (int i = 0, sz = config.GetSize(); i < sz; ++i) {
			if ( strMS.IsEqual(config.SlotName(i)) ) {
				// must start scripting again (no key given)
				Context::PushPopEntry<Anything> aDefaultEntry(ctx, "DoDefaultDoGetAny", anyBreak, "DoDefaultDoGetAny");
				DoGetAny("", value, ctx, config[i]);
			} else {
				EvaluateConfig(config[i], value[config.SlotName(i)], ctx);
			}
		}
	} else {
		value = config.DeepClone();
	}
}
