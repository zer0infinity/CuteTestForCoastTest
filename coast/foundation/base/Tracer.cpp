/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "Tracer.h"
#include "SystemBase.h"
#include "SystemFile.h"
#include "SystemLog.h"
#include "StringStream.h"
#include "InitFinisManagerFoundation.h"

using namespace Coast;

#include <cstring>
#ifdef COAST_TRACE

//---- Tracer -------------------------------------------------------------------------
namespace {
	char const fgPathDelim = '.';
	char const *fgTracerAnyName = "Tracer";
	char const *fgLowerBoundName = "LowerBound";
	char const *fgUpperBoundName = "UpperBound";
	char const *fgDumpAnythingsName = "DumpAnythings";
	char const *fgMainSwitchName = "MainSwitch";
	char const *fgEnableAllName = "EnableAll";
	int fgLevel = 0;
	Anything fgTriggerMap(Coast::Storage::Global());
	ROAnything fgROTriggerMap;
	long fgLowerBound = 0;
	long fgUpperBound = 0;
	bool fgDumpAnythings = false;
	bool fgTerminated = true;
	bool fgIsInitialised = false;

	enum EnablingMode {
		eUndecided = -1,
		eDisableAll = 0,
		eEnableAll = 1,
	};
	bool TracingActive() {
		return ( fgLowerBound > 0 && fgUpperBound >= fgLowerBound );
	}
	bool CheckEntryGreaterEqual(long lEntryValue, long lMainSwitch) {
		return ( lEntryValue <= fgUpperBound && lEntryValue >= fgLowerBound && lEntryValue >= lMainSwitch );
	}
	bool EntryEnabled(long lMainSwitch, long lEnableAll, long lEntryValue) {
		return (lMainSwitch >= fgLowerBound) && ( CheckEntryGreaterEqual(lEnableAll, lMainSwitch) || CheckEntryGreaterEqual(lEntryValue, lMainSwitch) );
	}
	void ProcessEntry(ROAnything anySection, String entryKey, EnablingMode parentMode, long lParentMainSwitch) {
		long lMainSwitch = anySection[fgMainSwitchName].AsLong(lParentMainSwitch);
		long lEnableAll = anySection[fgEnableAllName].AsLong(-1L);
		EnablingMode myMode = parentMode;
		if ( lMainSwitch <= 0L || parentMode == eDisableAll ) { // disable this level and all levels below
			myMode = eDisableAll;
		} else if (lMainSwitch >= fgLowerBound && lEnableAll >= lMainSwitch && lEnableAll <= fgUpperBound) {
			myMode = eEnableAll;
		}
		if ( entryKey.Length() && ( myMode == eEnableAll || myMode == eDisableAll ) ) {
			fgTriggerMap[entryKey] = static_cast<long>(myMode);
		}
		String strSlotname;
		for (long lIdx = 0, lSize = anySection.GetSize(); lIdx < lSize; ++lIdx) {
			strSlotname = anySection.SlotName(lIdx);
			if (!strSlotname.Length() || strSlotname.IsEqual(fgMainSwitchName) || strSlotname.IsEqual(fgEnableAllName) || strSlotname.IsEqual(fgLowerBoundName) || strSlotname.IsEqual(fgUpperBoundName) || strSlotname.IsEqual(fgDumpAnythingsName) )
				continue;
			String currentKey = entryKey;
			if ( currentKey.Length() ) currentKey.Append(fgPathDelim);
			currentKey.Append(strSlotname);
			if ( anySection[lIdx].GetType() == AnyArrayType ) {
				ProcessEntry(anySection[lIdx], currentKey, myMode, lMainSwitch);
			} else {
				long lEntryValue = anySection[lIdx].AsLong(0L);
				if ( myMode == eEnableAll || myMode == eDisableAll ) {
					fgTriggerMap[currentKey] = ( lEntryValue < 0 ) ? 0L : static_cast<long>(myMode);
				} else {
					fgTriggerMap[currentKey] = EntryEnabled(lMainSwitch, lEnableAll, lEntryValue) ? 1L : 0L;
				}
			}
		}
	}

	bool IsTriggerEnabled(const char *trigger) {
		ROAnything dummy;
		if (fgROTriggerMap.LookupPath(dummy, trigger, '\000')) {
			return dummy.AsLong(0L) > 0L;
		}
		const char *cPos(strrchr(trigger, '.'));
		if (cPos != NULL) {
			int lpos(cPos - trigger);
			int const iBufSize = 1024;
			char pcPart[iBufSize] = { 0 };
			memcpy(pcPart, trigger, std::min(lpos, iBufSize - 1));
			return IsTriggerEnabled(pcPart);
		}
		return false;
	}

	void InitTracing(String const& strFilename = fgTracerAnyName) {
		std::istream *ifp = System::OpenStream(strFilename, "any");
		if (ifp) {
			Anything anyDebugContext;
			if ( anyDebugContext.Import(*ifp, strFilename) ) {
				if (anyDebugContext.GetType() != AnyNullType) {
					fgLowerBound = anyDebugContext[fgLowerBoundName].AsLong(0);
					fgUpperBound = anyDebugContext[fgUpperBoundName].AsLong(0);
					fgDumpAnythings = anyDebugContext[fgDumpAnythingsName].AsBool(true);
					ProcessEntry(anyDebugContext, "", TracingActive()?eUndecided:eDisableAll, fgLowerBound);
					fgROTriggerMap = fgTriggerMap;
					String strbuf(4096L);
					{ StringStream stream(strbuf); fgTriggerMap.PrintOn(stream) << "\n"; }
					SystemLog::Debug(strbuf);
				}
			}
			delete ifp;
		}
	}
	void TerminateTracing() {
		fgTriggerMap = Anything(fgTriggerMap.GetAllocator());
		fgROTriggerMap = fgTriggerMap;
		fgLowerBound = 0;
		fgUpperBound = 0;
		fgDumpAnythings = false;
		fgTerminated = true;
		fgIsInitialised = false;
	}

	class TracingInitializer : public InitFinisManagerFoundation
	{
	public:
		TracingInitializer(unsigned int uiPriority)
		: InitFinisManagerFoundation(uiPriority) {
			IFMTrace("TracingInitializer created\n");
		}

		~TracingInitializer() {
			IFMTrace("TracingInitializer deleted\n");
		}

		virtual void DoInit() {
			IFMTrace("TracingInitializer::DoInit\n");
			InitTracing();
		}

		virtual void DoFinis() {
			IFMTrace("TracingInitializer::DoFinis\n");
			TerminateTracing();
		}
	};

	TracingInitializer *psgTracingInitializer = new TracingInitializer(2);

	//! Utility class to keep track of trace indent
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
			SystemLog::WriteToStderr(fStrBuf);
		}
		void Tab(int n) {
			for (int i = 0; i < n; ++i) {
				fStream << "  ";
			}
		}
		std::ostream &GetStream() {
			return fStream;
		}

	private:
		String fStrBuf;
		OStringStream fStream;
	};

	void IntAnyWDDebug(const ROAnything &any, TracerHelper &hlp)
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
}

void Tracer::ExchangeConfigFile(const char *filename) {
	TerminateTracing();
	InitTracing( ( filename == 0 ) ? fgTracerAnyName : filename );
}

bool Tracer::CheckWDDebug(const char *trigger)
{
	if (!fgIsInitialised) {
		fgIsInitialised = true;
		fgTerminated = (System::EnvGet("COAST_NO_TRACE") == "true") ? true : false;
	}
	if ( fgTerminated ) {
		return false;
	}
	if ( TracingActive() ) {
		return IsTriggerEnabled(trigger);
	}
	return false;
}

Tracer::Tracer(const char *trigger)
	: fTrigger(trigger)
	, fTriggered(false)
	, fpMsg(NULL)
	, fpAlloc(Coast::Storage::Current())
{
	fTriggered = CheckWDDebug(fTrigger);
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
	, fpAlloc(Coast::Storage::Current())
{
	fTriggered = CheckWDDebug(fTrigger);
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

	if (fTriggered && CheckWDDebug(trigger)) {
		TracerHelper hlp(fgLevel, fpAlloc);
		hlp.GetStream() << trigger << ": " << msg << "\n";
	}
}

void Tracer::SubAnyWDDebug(const char *subtrigger, const ROAnything &any, const char *msg)
{
	String trigger(fTrigger, -1, fpAlloc);
	trigger.Append('.').Append(subtrigger);

	if (fTriggered && CheckWDDebug(trigger)) {
		TracerHelper hlp(fgLevel, fpAlloc);
		hlp.GetStream() << trigger << ": " << msg << "\n";
		IntAnyWDDebug(any, hlp);
	}
}

void Tracer::StatWDDebug(const char *trigger, const char *msg, Allocator *pAlloc)
{
	if (CheckWDDebug(trigger)) {
		TracerHelper hlp(fgLevel, pAlloc);
		hlp.GetStream() << trigger << ": " << msg << "\n";
	}
}

void Tracer::AnythingWDDebug(const char *trigger, const ROAnything &any, const char *msg, Allocator *pAlloc)
{
	if (CheckWDDebug(trigger)) {
		TracerHelper hlp(fgLevel, pAlloc);
		hlp.GetStream() << trigger << ": " << msg << "\n";
		IntAnyWDDebug(any, hlp);
	}
}

#endif
