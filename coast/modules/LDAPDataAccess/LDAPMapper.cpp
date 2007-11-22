/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "LDAPMapper.h"

//--- standard modules used ----------------------------------------------------
#include "Registry.h"
#include "StringStream.h"
#include "SysLog.h"
#include "Base64.h"
#include "Dbg.h"
#include "Timers.h"

#if defined(ONLY_STD_IOSTREAM)
using namespace std;
#endif

//--- c-library modules used ---------------------------------------------------
#include <openssl/sha.h>

//---- LDAPFieldMapper ------------------------------------------------------------------------
RegisterParameterMapper(LDAPFieldMapper);
LDAPFieldMapper::LDAPFieldMapper(const char *name): EagerParameterMapper(name)
{
}

IFAObject *LDAPFieldMapper::Clone() const
{
	return new LDAPFieldMapper(fName);
}

bool LDAPFieldMapper::DoGetStream(const char *key, ostream &os, Context &ctx,  ROAnything info)
{
	StartTrace1(LDAPFieldMapper.Get, "Key: " << key);
	if ( key != 0 ) {
		String rawInput;
		String formattedInput;
		String dataFormat;
		if ( ParameterMapper::Get(key, rawInput, ctx) ) {
			if ( info.IsDefined("DataFormat") ) {
				dataFormat = info["DataFormat"].AsCharPtr();
				Encoder *base64 = Encoder::FindEncoder("Base64Regular");
				if  ( dataFormat == "PWD" ) {
					// Passwords must be Secure hashed and base64 encoded
					unsigned char *sha_result;
					const char *dataTmp = rawInput;	// we need this stupid temp var because
					// there isn't an unsigned char* operator in string class
					unsigned char m[SHA_DIGEST_LENGTH];
					sha_result = SHA1((unsigned char *)dataTmp, (unsigned long)rawInput.Length(), m);

					Trace("<" << (char *)sha_result << ">" << "Returnwert: "); // apply cast for legacy compilers (SunC++4.2)
					formattedInput = "{SHA}";
					String in((void *)sha_result, SHA_DIGEST_LENGTH);
					base64->DoEncode(formattedInput, in);
					Trace("tmp: " << formattedInput);
				} else if  (  dataFormat == "Base64" ) {
					base64->DoEncode(formattedInput, rawInput);
				}
				os << formattedInput;
			} else {
				os << rawInput;
			}
			return true;
		}
	}
	return false;
}

//---- LDAPQueryMapper ------------------------------------------------------------------------
RegisterParameterMapper(LDAPQueryMapper);
LDAPQueryMapper::LDAPQueryMapper(const char *name): ParameterMapper(name)
{
}

IFAObject *LDAPQueryMapper::Clone() const
{
	return new LDAPQueryMapper(fName);
}

bool LDAPQueryMapper::DoGet(ostream &os, Context &ctx,  ROAnything config,  ROAnything query)
{
	StartTrace(LDAPQueryMapper.DoGet);

	bool retVal = true;
	long sz = query.GetSize();
	ROAnything queryItem;
	for (long i = 0; i < sz; ++i) {
		queryItem = query[i];

		// build final inner expression
		const char *slotName = query.SlotName(i);
		if ( slotName ) {
			os << "(";
			if ( queryItem.GetType() == AnyCharPtrType ) {
				const char *qvalue = queryItem.AsCharPtr("");

				if (config.IsDefined("Map")) {
					os << config["Map"][qvalue].AsCharPtr("");
				} else {
					os << qvalue;
				}
				os << slotName;

				ROAnything ctxValue(ctx.Lookup(qvalue));
				// the value from context takes precedence
				if ( !ctxValue.IsNull() ) {
					retVal = ParameterMapper::DoGetStream(qvalue, os, ctx, ROAnything()) && retVal;
				} else {
					// We can configure the values of the Map-Items too!
					if (config.IsDefined("Value")) {
						os << config["Value"][qvalue].AsCharPtr("");
					} else {
						retVal = ParameterMapper::DoGetStream(qvalue, os, ctx, ROAnything()) && retVal;
					}
				}
			} else {
				os << slotName;
				retVal = DoGet(os, ctx, config, queryItem) && retVal;
			}
			os << ")";
		} else {
			retVal = DoGet(os, ctx, config, queryItem) && retVal;
		}
	}
	return retVal;
}

bool LDAPQueryMapper::DoGetStream(const char *key, ostream &os, Context &ctx,  ROAnything config)
{
	StartTrace1(LDAPQueryMapper.Get, "Key: " << key);
	ROAnything ctxValue(ctx.Lookup(key));
	if ( !ctxValue.IsNull() ) {
		os << ctxValue.AsString();
		return true;
	}
	if ( config.IsDefined("Query") ) {
		return DoGet(os, ctx, config, config["Query"]);
	}
	return DoGet(os, ctx, config, config);
}

//---- LDAPParamMapper ------------------------------------------------------------------------
RegisterParameterMapper(LDAPParamMapper);
LDAPParamMapper::LDAPParamMapper(const char *name): EagerParameterMapper(name)
{
}

IFAObject *LDAPParamMapper::Clone() const
{
	return new LDAPParamMapper(fName);
}

bool LDAPParamMapper::DoGetAny(const char *key, Anything &value, Context &ctx,  ROAnything config)
{
	StartTrace1(LDAPParamMapper.DoGetAny, "Key: " << key);
	if ( key ) {
		if (!config.IsNull()) {
			TraceAny(config, "config");
			long sz = config.GetSize();
			ROAnything paramItem;
			for (long i = 0; i < sz; ++i) {
				paramItem = config[i];
				// build final inner expression
				const char *slotName = config.SlotName(i);
				Anything any;
				ROAnything ctxValue;
				if (slotName != NULL) {
					if (ctx.Lookup(slotName, ctxValue)) {
						any = ctxValue.DeepClone();
					}
				}
				if (ctxValue.IsNull()) {
					any = paramItem.DeepClone();
				}
				if ( slotName ) {
					Trace("setting named slot [" << slotName << "]");
					value[slotName] = any;
				} else {
					Trace("appending unnamed slot");
					value.Append(any);
				}
			}
			return true;
		}
	}
	return false;
}

//---- LDAPDNameMapper ------------------------------------------------------------------------
RegisterParameterMapper(LDAPDNameMapper);
LDAPDNameMapper::LDAPDNameMapper(const char *name): ParameterMapper(name)
{
}

IFAObject *LDAPDNameMapper::Clone() const
{
	return new LDAPDNameMapper(fName);
}

bool LDAPDNameMapper::DoGet(ostream &os, Context &ctx,  ROAnything config,  ROAnything query)
{
	StartTrace(LDAPDNameMapper.DoGet);

	bool retVal = true;
	long sz = query.GetSize();
	ROAnything queryItem;
	for (long i = 0; i < sz; ++i) {
		queryItem = query[i];

		// build final inner expression
		const char *slotName = query.SlotName(i);
		if ( slotName ) {
			if ( queryItem.GetType() == AnyCharPtrType ) {
				const char *qvalue = queryItem.AsCharPtr("");

				if (config.IsDefined("Map")) {
					os << config["Map"][qvalue].AsCharPtr("");
				} else {
					os << qvalue;
				}
				os << slotName;

				ROAnything ctxValue(ctx.Lookup(qvalue));
				// the value from context takes precedence
				if ( !ctxValue.IsNull() ) {
					retVal = ParameterMapper::DoGetStream(qvalue, os, ctx, ROAnything()) && retVal;
				} else {
					// We can configure the values of the Map-Items too!
					if (config.IsDefined("Value")) {
						os << config["Value"][qvalue].AsCharPtr("");
					} else {
						retVal = ParameterMapper::DoGetStream(qvalue, os, ctx, ROAnything()) && retVal;
					}
				}
			} else {
				os << slotName;
				retVal = DoGet(os, ctx, config, queryItem) && retVal;
			}
			os << ",";
		} else {
			retVal = DoGet(os, ctx, config, queryItem) && retVal;
		}
	}
	return retVal;
}

bool LDAPDNameMapper::DoGetStream(const char *key, ostream &os, Context &ctx,  ROAnything config)
{
	StartTrace1(LDAPDNameMapper.Get, "Key: " << key);
	ROAnything ctxValue(ctx.Lookup(key));
	if ( !ctxValue.IsNull() ) {
		os << ctxValue.AsString();
		return true;
	}
	if ( config.IsDefined("Query") ) {
		return DoGet(os, ctx, config, config["Query"]);
	}
	return DoGet(os, ctx, config, config);
}

//---- LDAPModifyValsMapper ------------------------------------------------------------------------
RegisterParameterMapper(LDAPModifyValsMapper);
LDAPModifyValsMapper::LDAPModifyValsMapper(const char *name): EagerParameterMapper(name)
{
}

IFAObject *LDAPModifyValsMapper::Clone() const
{
	return new LDAPModifyValsMapper(fName);
}

bool LDAPModifyValsMapper::DoGetAny(const char *key, Anything &value, Context &ctx,  ROAnything config)
{
	bool retVal = true;
	if ( key ) {
		if (!config.IsNull()) {
			long modsize = config.GetSize();
			for ( long i = 0; i < modsize; ++i ) {
				Anything tempmod = config[i].DeepClone();
				if ( tempmod.IsDefined("Mode") ) {
					value[i]["Mode"] = tempmod["Mode"];
				} else {
					// Fehler Syslog - abbruch??
					retVal = false;
				}
				if ( tempmod.IsDefined("Attribut") ) {
					value[i]["Attribut"] = tempmod["Attribut"];
				} else {
					// Fehler Syslog - abbruch??
					retVal = false;
				}
				if ( tempmod.IsDefined("Values") ) {
					long valsize = tempmod["Values"].GetSize();
					Anything tempval = tempmod["Values"];
					for ( long m = 0; m < valsize; ++m ) {
						if ( tempval[m].IsDefined("Map") ) {
							String modifyVal;
							{
								OStringStream os(&modifyVal);
								os << flush;
								const char *newkey = tempval[m]["Map"].SlotName(0L);
								ParameterMapper::DoGetStream(newkey, os, ctx, tempval[m]["Map"]);
							}
							value[i]["Values"][m] = modifyVal;
						} else {
							value[i]["Values"][m] = tempval[m]["Value"];
						}
						if ( !value[i]["Values"][m].AsString().Length() ) {
							// There are no data!!
							String msg("LDAPModify BuildQuery: There are no inputdata ");
							msg << "Into Modifyslot " << i << " at value " << m;
							SysLog::Error(msg);
							retVal = false;
						}
					}
				}
			}
		} else {
			retVal = false;
		}
	} else {
		retVal = false;
	}
	return retVal;
}

//---- LDAPListWithPrimaryKeyMapper ------------------------------------------------------------------------
RegisterResultMapper(LDAPListWithPrimaryKeyMapper);
LDAPListWithPrimaryKeyMapper::LDAPListWithPrimaryKeyMapper(const char *name)
	: ResultMapper(name)
{
}

IFAObject *LDAPListWithPrimaryKeyMapper::Clone() const
{
	return new LDAPListWithPrimaryKeyMapper(fName);
}

bool LDAPListWithPrimaryKeyMapper::DoPutAny(const char *key, Anything result, Context &ctx, ROAnything config)
{
	StartTrace1(LDAPListWithPrimaryKeyMapper.DoPut, "Key: " << key);

	TraceAny(config, "config");
	TraceAny(result, "result");
	Trace(fName);

	String primaryKey = config["PrimaryKey"].AsString("");
	if ( key && primaryKey.Length() > 0) {
		if (result.IsDefined(primaryKey)) {
			String primaryKeyValue = result[primaryKey].AsString();
			Anything tmpStore(ctx.GetTmpStore());
			tmpStore["Mapper"][key][primaryKeyValue] = result;
		}
		return true;
	}
	return false;
}
