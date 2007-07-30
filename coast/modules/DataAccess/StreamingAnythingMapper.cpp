/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "StreamingAnythingMapper.h"

//--- standard modules used ----------------------------------------------------
#include "StringStream.h"
#include "Dbg.h"
#include "Timers.h"
#include "SysLog.h"

#if defined(ONLY_STD_IOSTREAM)
using namespace std;
#endif

//--- c-library modules used ---------------------------------------------------

//---- AnythingToStreamMapper ----------------------------------------------------------------
RegisterParameterMapper(AnythingToStreamMapper);

bool AnythingToStreamMapper::DoFinalGetStream(const char *key, ostream &os, Context &ctx)
{
	StartTrace1(AnythingToStreamMapper.DoFinalGetStream, NotNull(key));
	if ( key ) {
		// use the superclass mapper to get the anything
		Anything anyValue;
		DoFinalGetAny(key, anyValue, ctx);

		TraceAny(anyValue, "value fetched from context");

		if ( !anyValue.IsNull() ) {
			DAAccessTimer(AnythingToStreamMapper.DoFinalGetStream, "exporting to stream", ctx);
			os << anyValue << flush;
			TraceAny(anyValue, "written to stream:");
			return true;
		}
		Trace("Nothing written to stream, value was null.");
	}
	return false;
}

//---- StreamToAnythingMapper ----------------------------------------------------------------
RegisterResultMapper(StreamToAnythingMapper);

bool StreamToAnythingMapper::DoPutStream(const char *key, istream &is, Context &ctx, ROAnything script)
{
	StartTrace1(StreamToAnythingMapper.DoPutStream, NotNull(key));
	Anything anyResult;
	String strBuf(4096);
	bool importok = false;
	long lToCopy = 4096L, lCopied = 0L, lTotal = 0L;
	bool bSuccess = true;
	{
		OStringStream ostr(strBuf);
		DAAccessTimer(StreamToAnythingMapper.DoPutStream, "copying from stream", ctx);
		while ( ( bSuccess = NSStringStream::PlainCopyStream2Stream(&is, ostr, lCopied, lToCopy) ) && ( lCopied >= lToCopy ) ) {
			lTotal += lCopied;
			Trace("bytes read so far: " << lTotal);
		}
		lTotal += lCopied;
	}
	if ( bSuccess ) {
		Trace("Length of received content:" << lTotal);
		SubTrace(TraceBuf, "content [" << strBuf << "]");
		{
			DAAccessTimer(StreamToAnythingMapper.DoPutStream, "importing Anything", ctx);
#if 0
			String strDump = strBuf.DumpAsHex(32);
			SysLog::WriteToStderr(strDump);
#endif
			IStringStream istr(strBuf);
			importok = anyResult.Import(istr, "ImportingFromStream");
		}
		if ( importok ) {
			SubTraceAny(TraceResult, anyResult, "anything imported from stream:");
			importok = DoPutAny(key, anyResult, ctx, script);
		} else {
			SYSWARNING("importing Anything from stream failed: total input-length:" << lTotal);
		}
	} else {
		SYSWARNING("receiving stream content failed: bytes received so far:" << lTotal);
	}
	return importok;
}

bool StreamToAnythingMapper::DoPutAnyWithSlotname(const char *key, Anything value, Context &ctx, ROAnything roaScript, const char *slotname)
{
	StartTrace1(StreamToAnythingMapper.DoPutAnyWithSlotname, "key [" << NotNull(key) << "] slotname [" << NotNull(slotname) << "]");
	Trace("Using slotname [" << slotname << "] as Lookup path in value");
	bool bRet = true;	// do not fail when lookup fails!
	Anything anyValue;
	if ( value.LookupPath(anyValue, slotname) ) {
		TraceAny(anyValue, "Calling myself again with Anything looked up at [" << slotname << "]");
		bRet = DoPutAny(key, anyValue, ctx, roaScript);
		Trace("RetCode of DoPutAny:" << (bRet ? "true" : "false"));
	}
	return bRet;
}
