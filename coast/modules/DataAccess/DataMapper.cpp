/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "DataMapper.h"

//--- standard modules used ----------------------------------------------------
#include "SystemLog.h"
#include "Renderer.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

//---- FixedSizeMapper ------------------------------------------------------------------
RegisterParameterMapper(FixedSizeMapper);

FixedSizeMapper::FixedSizeMapper(const char *name) : EagerParameterMapper(name)
{
}

IFAObject *FixedSizeMapper::Clone() const
{
	return new FixedSizeMapper(fName);
}

void FixedSizeMapper::Pad(String &value, long length, char pad)
{
	if (value.Length() > length) {
		value.Trim(length);
	} else {
		long oldlength = value.Length();
		for (long i = oldlength; i < length; ++i) {
			value.Append(pad);
		}
	}
}

bool FixedSizeMapper::DoGetStream(const char *key, ostream &os, Context &ctx, ROAnything info)
{
	String value;
	OStringStream osStr(&value);
	long length = info["Length"].AsLong(20L);
	char pad = info["PadChar"].AsCharPtr(" ")[0];

	if ( info.IsDefined(key) ) {
		info = info[key];
	} else {
		info = ROAnything();
	}

	ParameterMapper::DoGetStream(key, osStr, ctx, info);
	osStr.flush();

	Pad(value, length, pad);
	os << value;
	return true;
}

//---- UpperCaseMapper ------------------------------------------------------------------
RegisterParameterMapper(UpperCaseMapper);

UpperCaseMapper::UpperCaseMapper(const char *name) : EagerParameterMapper(name)
{
}

IFAObject *UpperCaseMapper::Clone() const
{
	return new UpperCaseMapper(fName);
}

bool UpperCaseMapper::DoGetStream(const char *key, ostream &os, Context &ctx, ROAnything info)
{
	String strBuf;
	{
		OStringStream osStr(strBuf);
		ParameterMapper::DoGetStream(key, osStr, ctx, info);
	}
	os << strBuf.ToUpper();
	return true;
}

//---- RendererMapper ------------------------------------------------------------------
RegisterParameterMapper(RendererMapper);

RendererMapper::RendererMapper(const char *name) : EagerParameterMapper(name)
{
}

IFAObject *RendererMapper::Clone() const
{
	return new RendererMapper(fName);
}

bool RendererMapper::DoGetStream(const char *key, ostream &os, Context &ctx, ROAnything info)
{
	StartTrace1(RendererMapper.DoGetStream, NotNull(key));
	bool bInfoIsNull(true);
	if (!(bInfoIsNull=info.IsNull())) {
		Renderer::Render( os, ctx, info );
	}
	TraceAny(info, "Renderer spec content, returning " << (bInfoIsNull?"false":"true"));
	return !bInfoIsNull;
}

bool RendererMapper::DoGetAny(const char *key, Anything &value, Context &ctx, ROAnything info)
{
	StartTrace1(RendererMapper.DoGetAny, NotNull(key));
	bool bGetSuccess(true);
	String strBuf(16384L);
	OStringStream stream(strBuf);
	if ((bGetSuccess=DoGetStream(key, stream, ctx, info))) {
		stream.flush();
		value = strBuf;
	}
	Trace("returnging " << (bGetSuccess?"true":"false"));
	return bGetSuccess;
}

//special case, because we are non-eager in the anything case.
bool RendererMapper::Get(const char *key, Anything &value, Context &ctx)
{
	StartTrace1(RendererMapper.Get, "( \"" << NotNull(key) << "\" , Anything &value, Context &ctx)");
	Anything anyValue;
	if (DoGetAny(key, anyValue, ctx, ParameterMapper::DoSelectScript(key, fConfig, ctx))) {
		value = anyValue;
		TraceAny(value, "returned value");
		return true;
	}
	return false;
}

//---- LookupMapper ------------------------------------------------------------------
RegisterParameterMapper(LookupMapper);

LookupMapper::LookupMapper(const char *name) : EagerParameterMapper(name)
{
}

IFAObject *LookupMapper::Clone() const
{
	return new LookupMapper(fName);
}

static const char *gcSlotName =		"LookupName";

bool LookupMapper::DoGetStream(const char *key, ostream &os, Context &ctx, ROAnything info)
{
	StartTrace(LookupMapper.DoGet);
	TraceAny(info, "info");

	bool isSimpleArray = false;

	ROAnything lookupName;
	if (!info.LookupPath(lookupName, gcSlotName, '\000')) {		// use new slotname
		isSimpleArray = true;
		lookupName = info[0L];
	}

	// lookup data and use it as a renderer specification
	if (! lookupName.IsNull()) {		// check if lookupName is defined somehow
		if ( isSimpleArray ) {
			ParameterMapper *m = ParameterMapper::FindParameterMapper(lookupName.AsCharPtr(""));
			if (m) {
				return m->Get(key, os, ctx);
			}
		}

		return DoGetStream(key, os, ctx, lookupName);

	} else {
		// handle error: lookup name is not a string
		String error("LookupMapper::Get: invalid lookup name: ");
		{
			OStringStream ostr(&error);
			lookupName.PrintOn(ostr, false);	// append Anything to ease debugging
		}
		SystemLog::Error(error);
		return false;
	}
}
