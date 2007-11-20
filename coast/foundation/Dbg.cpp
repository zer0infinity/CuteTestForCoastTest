/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "Dbg.h"

//--- standard modules used ----------------------------------------------------
#include "System.h"
#include "SysLog.h"
#include "StringStream.h"

//--- c-library modules used ---------------------------------------------------
#include <cstring>

#ifdef COAST_TRACE

//---- Tracer -------------------------------------------------------------------------
int Tracer::fgLevel = 0;
Anything Tracer::fgWDDebugContext(Storage::Global());
ROAnything Tracer::fgROWDDebugContext;
long Tracer::fgLowerBound = 0;
long Tracer::fgUpperBound = 0;
bool Tracer::fgDumpAnythings = false;
bool Tracer::fgTerminated = true;
static bool fgIsInitialised = false;

//! <b>Utility class to keep track of trace indent</b>
class TracerHelper
{
public:
	TracerHelper(int nLevel, Allocator *pAlloc)
		: fStrBuf(128L, pAlloc)
		, fStream(fStrBuf) {
		Tab(nLevel);
	}
	~TracerHelper() {
		fStream.flush();
		SysLog::WriteToStderr(fStrBuf);
	}
	void Tab(int n) {
		for (int i = 0; i < n; ++i) {
			fStream << "  ";
		}
	}
	ostream &GetStream() {
		return fStream;
	}

private:
	String fStrBuf;
	OStringStream fStream;
};

Tracer::Tracer(const char *trigger)
	: fTrigger(trigger)
	, fTriggered(false)
	, fpMsg(NULL)
	, fpAlloc(Storage::Current())
{
	fTriggered = CheckWDDebug(fTrigger, fpAlloc);
	if (fTriggered) {
		TracerHelper hlp(fgLevel, fpAlloc);
		hlp.GetStream() << fTrigger << ": --- entering ---\n";
		++fgLevel;
	}
}

Tracer::Tracer(const char *trigger, const char *msg)
	: fTrigger(trigger)
	, fTriggered(false)
	, fpMsg(msg)
	, fpAlloc(Storage::Current())
{
	fTriggered = CheckWDDebug(fTrigger, fpAlloc);
	if (fTriggered) {
		TracerHelper hlp(fgLevel, fpAlloc);
		hlp.GetStream() << fTrigger << ": " << fpMsg << " --- entering ---\n";
		++fgLevel;
	}
}

Tracer::~Tracer()
{
	if (fTriggered) {
		--fgLevel;
		TracerHelper hlp(fgLevel, fpAlloc);
		hlp.GetStream() << fTrigger << ":";
		if (fpMsg) {
			hlp.GetStream() << " " << fpMsg;
		}
		hlp.GetStream() << " --- leaving ---\n";
	}
}

void Tracer::WDDebug(const char *msg)
{
	if (fTriggered) {
		TracerHelper hlp(fgLevel, fpAlloc);
		hlp.GetStream() << fTrigger << ": " << msg << "\n";
	}
}

void Tracer::IntAnyWDDebug(const ROAnything &any, TracerHelper &hlp)
{
	hlp.Tab(fgLevel);
	if (fgDumpAnythings) {
		hlp.GetStream() << "--- Start of Anything Dump ------------\n";
		hlp.Tab(fgLevel);
		any.Export(hlp.GetStream(), fgLevel);
		hlp.GetStream() << "\n";
		hlp.Tab(fgLevel);
		hlp.GetStream() << "--- End of Anything Dump --------------";
	} else {
		hlp.GetStream() << "{ ... anything ommitted ... }";
	}
	hlp.GetStream() << "\n";
}

void Tracer::AnyWDDebug(const ROAnything &any, const char *msg)
{
	if (fTriggered) {
		TracerHelper hlp(fgLevel, fpAlloc);
		hlp.GetStream() << fTrigger << ": "  << msg << "\n";
		IntAnyWDDebug(any, hlp);
	}
}

void Tracer::SubWDDebug(const char *subtrigger, const char *msg)
{
	String trigger(fTrigger, -1, fpAlloc);
	trigger.Append('.').Append(subtrigger);

	if (fTriggered && Tracer::CheckWDDebug(trigger, fpAlloc)) {
		TracerHelper hlp(fgLevel, fpAlloc);
		hlp.GetStream() << trigger << ": " << msg << "\n";
	}
}

void Tracer::SubAnyWDDebug(const char *subtrigger, const ROAnything &any, const char *msg)
{
	String trigger(fTrigger, -1, fpAlloc);
	trigger.Append('.').Append(subtrigger);

	if (fTriggered && Tracer::CheckWDDebug(trigger, fpAlloc)) {
		TracerHelper hlp(fgLevel, fpAlloc);
		hlp.GetStream() << trigger << ": " << msg << "\n";
		IntAnyWDDebug(any, hlp);
	}
}

void Tracer::StatWDDebug(const char *trigger, const char *msg, Allocator *pAlloc)
{
	if (CheckWDDebug(trigger, pAlloc)) {
		TracerHelper hlp(fgLevel, pAlloc);
		hlp.GetStream() << trigger << ": " << msg << "\n";
	}
}

void Tracer::AnythingWDDebug(const char *trigger, const ROAnything &any, const char *msg, Allocator *pAlloc)
{
	if (CheckWDDebug(trigger, pAlloc)) {
		TracerHelper hlp(fgLevel, pAlloc);
		hlp.GetStream() << trigger << ": " << msg << "\n";
		IntAnyWDDebug(any, hlp);
	}
}

bool Tracer::CheckWDDebug(const char *trigger, Allocator *pAlloc)
{
	if (!fgIsInitialised) {
		fgIsInitialised = true;
		Tracer::fgTerminated = (System::EnvGet("NO_DEBUG") == "true") ? true : false;
	}

	if ( fgTerminated ) {
		return false;
	}
	static bool trying( false );	// FIXME: hack until recursive mutex are implemented

	if (fgWDDebugContext.GetType() == AnyNullType) {
		if ( trying ) {
			return false;
		}
		trying = true;
		Reset();
		// try to read Dbg.any only once <<- doesn't work as expected
		trying = false;
	}

	if (fgLowerBound > 0) {
		return CheckTrigger(trigger, pAlloc);
	}
	return false;
}

bool Tracer::CheckMainSwitch(long mainSwitch, long levelSwitch, long levelAll)
{
	if ( mainSwitch == 0L ) {
		return false;	// 0 is always off
	}
	// main switch out of range -> off
	if (mainSwitch < levelSwitch) {
		return false;
	}
	if ( ( levelAll > 0 ) && (mainSwitch > levelAll) ) {
		return false;
	}
	return true;
}

bool Tracer::DoCheckLevel(const char *trigger, const ROAnything &level, long levelSwitch, long levelAll, long enableAll, Allocator *pAlloc)
{
	// check the main switch if it exists
	if ( level.IsDefined("MainSwitch") ) {
		if ( enableAll ) {
			if ( level["MainSwitch"].AsLong(0) < 0 ) {
				return false;
			} else {
				return DoCheckTrigger(trigger, level, levelSwitch, levelAll, enableAll, pAlloc);
			}
		}
		if (!CheckMainSwitch(level["MainSwitch"].AsLong(0), levelSwitch, levelAll)) {
			return false;
		}
	}
	if ( !enableAll && level.IsDefined("LowerBound") ) {
		if (!CheckMainSwitch(level["LowerBound"].AsLong(0), levelSwitch, levelAll)) {
			return false;
		}
	}

	// check the enable all switch if it exists
	if ( levelAll > 0 ) {
		if (level.IsDefined("EnableAll")) {
			enableAll = level["EnableAll"].AsLong(0L);
			// enable all switch in range -> all on
			if ( ( enableAll > 0L ) && ( enableAll >= levelSwitch ) && ( enableAll <= levelAll ) ) {
				return DoCheckTrigger(trigger, level, levelSwitch, levelAll, enableAll, pAlloc);
			}
			enableAll = 0;
		}
	} else {
		levelAll = fgUpperBound;
	}
	return DoCheckTrigger(trigger, level, levelSwitch, levelAll, enableAll, pAlloc);
}

bool Tracer::DoCheckTrigger(const char *trigger, const ROAnything &level, long levelSwitch, long levelAll, long enableAll, Allocator *pAlloc)
{
	const char *cPos( strchr(trigger, '.') );
	if ( cPos != NULL ) {
		long lpos(cPos - trigger);
		char pcPart[512] = { 0 };
		memcpy(pcPart, trigger, std::min( lpos, 511L ) );
		if ( level.IsDefined(pcPart) && ( *++cPos != '\0' ) ) {
			return DoCheckLevel(cPos, level[pcPart], levelSwitch, levelAll, enableAll, pAlloc);
		}
	}

	if ( level.IsDefined(trigger) ) {
		long switchValue;
		if ( level[trigger].IsDefined("MainSwitch") ) {
			switchValue = level[trigger]["MainSwitch"].AsLong(levelSwitch);
		} else {
			switchValue = level[trigger].AsLong(levelSwitch);
		}
		if ( switchValue < 0 ) {
			return false;
		}
		if ( !enableAll ) {
			return DoCheckSwitch(switchValue, levelSwitch, levelAll);
		}
	}
	return (enableAll > 0);
}

bool Tracer::DoCheckSwitch(long switchValue, long levelSwitch, long levelAll)
{
	// check the switch value whether it enables tracing or not
	if ( switchValue >= levelSwitch && switchValue <= levelAll ) {
		return true;
	}
	return false;
}

bool Tracer::CheckTrigger(const char *trigger, Allocator *pAlloc)
{
	return DoCheckLevel(trigger, fgROWDDebugContext, fgLowerBound, 0L, 0L, pAlloc);
}

void Tracer::Reset()
{
	istream *ifp = System::OpenStream("Dbg", "any");
	if (ifp) {
		fgWDDebugContext.Import(*ifp, "Dbg");
		fgROWDDebugContext = fgWDDebugContext;
		delete ifp;
	}
	if (fgWDDebugContext.GetType() != AnyNullType) {
		fgLowerBound = fgWDDebugContext["LowerBound"].AsLong(0);
		fgUpperBound = fgWDDebugContext["UpperBound"].AsLong(0);
		fgDumpAnythings = fgWDDebugContext["DumpAnythings"].AsBool(true);
	}
}

void Tracer::Terminate()
{
	fgTerminated = true;
	fgWDDebugContext = Anything();
	fgROWDDebugContext = fgWDDebugContext;
}

#endif
