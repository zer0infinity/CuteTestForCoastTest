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
#include "SystemLog.h"
#include "StringStream.h"
#include "InitFinisManagerFoundation.h"

//--- c-library modules used ---------------------------------------------------
#include <cstring>

#ifdef COAST_TRACE

//---- Tracer -------------------------------------------------------------------------
namespace {
	int fgLevel = 0;
	Anything fgWDDebugContext(Storage::Global());
	ROAnything fgROWDDebugContext;
	long fgLowerBound = 0;
	long fgUpperBound = 0;
	bool fgDumpAnythings = false;
	bool fgTerminated = true;
	bool fgIsInitialised = false;
	char const *fgDbgAnyName = "Dbg";

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
			istream *ifp = System::OpenStream(fgDbgAnyName, "any");
			if (ifp) {
				fgWDDebugContext.Import(*ifp, fgDbgAnyName);
				fgROWDDebugContext = fgWDDebugContext;
				delete ifp;
			}
			if (fgROWDDebugContext.GetType() != AnyNullType) {
				fgLowerBound = fgWDDebugContext["LowerBound"].AsLong(0);
				fgUpperBound = fgWDDebugContext["UpperBound"].AsLong(0);
				fgDumpAnythings = fgWDDebugContext["DumpAnythings"].AsBool(true);
			}
		}

		virtual void DoFinis() {
			IFMTrace("TracingInitializer::DoFinis\n");
			fgTerminated = true;
			fgWDDebugContext = Anything();
			fgROWDDebugContext = fgWDDebugContext;
			fgLowerBound = 0;
			fgUpperBound = 0;
			fgDumpAnythings = false;
			fgTerminated = true;
			fgIsInitialised = false;
		}
	};

	TracingInitializer *psgTracingInitializer = new TracingInitializer(0);

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
		ostream &GetStream() {
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

	//! Checks if level of trigger entry is to be enabled for tracing based on coresponding MainSwitch value
	/*! \param mainSwitch value to test for
		\param levelSwitch lower bound value
		\param levelAll upper bound value */
	bool CheckMainSwitch(long mainSwitch, long levelSwitch, long levelAll)
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

	//! Checks if level of trigger entry is to be enabled for tracing
	/*! \param switchValue value to test for
		\param levelSwitch lower bound value
		\param levelAll upper bound value */
	bool DoCheckSwitch(long switchValue, long levelSwitch, long levelAll)
	{
		// check the switch value whether it enables tracing or not
		if ( switchValue >= levelSwitch && switchValue <= levelAll ) {
			return true;
		}
		return false;
	}

	// forward declare
	bool DoCheckTrigger(const char *trigger, const ROAnything &level, long levelSwitch, long levelAll, long enableAll);

	//! Checks if current scope is enabled or not and delegates to DoCheckTrigger() if a trigger entry needs to be checked
	/*! \param trigger scope of trigger to check for
		\param level ROAnything referencing specific trace switch config entry used by looking up \em trigger in fgWDDebugContext
		\param levelSwitch lower bound at current scope to test for
		\param levelAll log level of outer scope
		\param enableAll enable all level of outer scope */
	bool DoCheckLevel(const char *trigger, const ROAnything &level, long levelSwitch, long levelAll, long enableAll)
	{
		// check the main switch if it exists
		if ( level.IsDefined("MainSwitch") ) {
			if ( enableAll ) {
				if ( level["MainSwitch"].AsLong(0) < 0 ) {
					return false;
				} else {
					return DoCheckTrigger(trigger, level, levelSwitch, levelAll, enableAll);
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
					return DoCheckTrigger(trigger, level, levelSwitch, levelAll, enableAll);
				}
				enableAll = 0;
			}
		} else {
			levelAll = fgUpperBound;
		}
		return DoCheckTrigger(trigger, level, levelSwitch, levelAll, enableAll);
	}

	//! forwarding method to DoCheckLevel()
	/*! \param trigger scope of trigger to check for */
	bool CheckTrigger(const char *trigger)
	{
		return DoCheckLevel(trigger, fgROWDDebugContext, fgLowerBound, 0L, 0L);
	}

	//! Checks if trigger entry is enabled for tracing or not
	/*! \param trigger scope of trigger to check for
		\param level ROAnything referencing specific trace switch config entry used by looking up \em trigger in fgWDDebugContext
		\param levelSwitch lower bound at current scope to test for
		\param levelAll log level of outer scope
		\param enableAll enable all level of outer scope */
	bool DoCheckTrigger(const char *trigger, const ROAnything &level, long levelSwitch, long levelAll, long enableAll)
	{
		const char *cPos( strchr(trigger, '.') );
		if ( cPos != NULL ) {
			long lpos(cPos - trigger);
			char pcPart[512] = { 0 };
			memcpy(pcPart, trigger, std::min( lpos, 511L ) );
			if ( level.IsDefined(pcPart) && ( *++cPos != '\0' ) ) {
				return DoCheckLevel(cPos, level[pcPart], levelSwitch, levelAll, enableAll);
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

}

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
		fgTerminated = (System::EnvGet("COAST_NO_TRACE") == "true") ? true : false;
	}
	if ( fgTerminated ) {
		return false;
	}
	if (fgROWDDebugContext.GetType() != AnyNullType && fgLowerBound > 0) {
		return CheckTrigger(trigger);
	}
	return false;
}

#endif
