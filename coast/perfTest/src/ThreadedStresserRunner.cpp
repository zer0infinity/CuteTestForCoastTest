/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ThreadedStresserRunner.h"

//--- standard modules used ----------------------------------------------------
#include "DiffTimer.h"
#include "Application.h"
#include "Dbg.h"
#include "SysLog.h"

//--- standard modules used ----------------------------------------------------
#include "StresserThread.h"

RegisterStresser(ThreadedStresserRunner);
//---- ThreadedStresserRunner ----------------------------------------------------------------
ThreadedStresserRunner::ThreadedStresserRunner(const char *StresserName)
	: Stresser(StresserName)
{
	StartTrace(ThreadedStresserRunner.ThreadedStresserRunner);
}

ThreadedStresserRunner::~ThreadedStresserRunner()
{
	StartTrace(ThreadedStresserRunner.~ThreadedStresserRunner);
}

long ThreadedStresserRunner::ConfigStressers(long numOfStressers, long &sz, ROAnything &stresser)
{
	//FIXME: hack assume one sort of stressers if numOfStressers != 0
	// Create and Initialize the StresserThreads
	StartTrace1(ThreadedStresserRunner.ConfigStressers, "NumOfStressers: " << numOfStressers);
	long numStressers = numOfStressers;
	if ( Lookup("Stressers", stresser) ) {
		TraceAny(stresser, "Stressers");
		if ( numStressers <= 0 ) {
			sz = stresser.GetSize();
			for (long i = 0; i < sz; i++) {
				numStressers += stresser[i].AsLong(0);
			}
		} else {
			sz = numStressers;
		}
	}
	return numStressers;
}

Anything ThreadedStresserRunner::Run(long /* id */)
{
	//FIXME: this method is a bunch of crap and needs a major refactoring!!
	//FIXME: for british people: this code is a major disaster
	StartTrace(ThreadedStresserRunner.Run);
	String stresserName;
	StresserThread *vastArrayOfStressers = 0;
	long numStressers(0);
	long stressersPending(0);
	long sz(0);
	Mutex::ConditionType 	stresserCond;
	Mutex		stresserMutex(fName);
	bool hasScriptConfig = false;

	//FIXME: hack to configure number of threads on the fly from scripting arguments
	//FIXME: this solution only supports one kind of stresser flow controllers
	ROAnything stresser;

	String appName;
	Application *application = Application::GetGlobalApplication(appName);
	if ( application ) {
		Trace(appName << " application found" );
		hasScriptConfig = (application->Lookup("NumberOfThreads", 0L) != 0);
		numStressers = ConfigStressers(application->Lookup("NumberOfThreads", 0L), sz, stresser);
		stresserName = application->Lookup("Stresser", "");
	} else {
		numStressers = ConfigStressers(0, sz, stresser);
	}

	Trace("Number of Stressers: " << numStressers);
	if ( !Lookup("Stressers", stresser) ) {
		Trace("Stressers misconfigured");
	} else {
		TraceAny(stresser, "Stressers: ");
	}

	vastArrayOfStressers = new StresserThread[numStressers];
	stressersPending = 0;
	for (long ii = 0; ii < sz; ii++) {
		Trace("Initialising " << ii);
		long num = stresser[ii].AsLong(0);
		if (!hasScriptConfig) {
			stresserName = stresser.SlotName(ii);
			for (long j = 0; j < num; j++) {
				vastArrayOfStressers[stressersPending].Init(stresserName, stressersPending + 1, &stresserCond, &stresserMutex, &stressersPending);
				stressersPending++;
			}
		} else {
			if (stresserName.Length() == 0) {
				stresserName = stresser.SlotName(0);
			}
			vastArrayOfStressers[stressersPending].Init(stresserName, stressersPending + 1, &stresserCond, &stresserMutex, &stressersPending);
			stressersPending++;
		}
	}
	// Let the Stressers run
	Trace(fName << " is running ...");
	DiffTimer Dt;
	bool usePool = Lookup("UsePoolStorage", 0L) == 1L;
	long poolStorageSize = Lookup("PoolStorageSize", 1000L);
	for (long i1 = 0; i1 < numStressers; i1++ ) {
		Trace("Starting " << i1);
		if ( usePool ) {
			vastArrayOfStressers[i1].Start(MT_Storage::MakePoolAllocator(poolStorageSize));
		} else {
			vastArrayOfStressers[i1].Start();
		}
	}

	// wait for all threads to finish
	{
		LockUnlockEntry 	me(stresserMutex);
		Trace("all threads have been started ... " << stressersPending << " pending");

		while ( stressersPending > 0 ) {		   	// modified by terminating threads
			(void) stresserCond.Wait( stresserMutex );	// release mutex and wait for condition
			Trace("Got up by signal, still " << stressersPending << " pending") ;
		}
	}

	// Collect results
	Anything results;
	results["Total"]["Time"] = (long)Dt.Diff();

	long totTr = 0, totSteps = 0;
	long totSum = 0;
	long totMax = 0;
	long totMin = 2000000;
	long totErr = 0;

	for (long i = 0; i < numStressers; i++ ) {
		Anything fullResult = vastArrayOfStressers[i].GetResult();

		TraceAny(fullResult, "Result " << i);

		Anything result, anyResults;
		if ( !fullResult.LookupPath(result, "Total") ) {
			Trace("no Total slot found, trying Results");
			// not a composit stresser
			if ( !fullResult.LookupPath(anyResults, "Results") ) {
				Trace("no Results slot found, assuming array of testresults");
				// assume only one resultset at index 0
				if ( fullResult.GetSize() > 1L ) {
					SYSWARNING("expected only one resultset!");
					anyResults = fullResult;
					TraceAny(anyResults, "array of results found, size:" << anyResults.GetSize());
				} else if ( fullResult.GetSize() == 1L ) {
					Trace("sinlge result found");
					result = fullResult[0L];
				} else {
					SYSWARNING("no results!");
				}
			}
		}
		if ( anyResults.GetSize() ) {
			result = anyResults[0L];
			anyResults.Remove(0L);
		}
		while ( !result.IsNull() ) {
			TraceAny(result, "current result");
			ROAnything roaResult(result);
			// add this result to the totals
			totTr += roaResult["Nr"].AsLong(0);
			totSteps += roaResult["Steps"].AsLong(0);
			totSum += roaResult["Sum"].AsLong(0);
			long itopia_max = roaResult["Max"].AsLong(0);
			totMax < itopia_max ? totMax = itopia_max : totMax = totMax;
			long itopia_min = roaResult["Min"].AsLong(0);
			totMin > itopia_min ? totMin = itopia_min : totMin = totMin;
			totErr += roaResult["Error"].AsLong(0);
			results["Results"].Append(result);
			result = Anything();
			if ( anyResults.GetSize() ) {
				result = anyResults[0L];
				anyResults.Remove(0L);
			}
		}
	}
	results["Total"]["Nr"] = totTr;
	results["Total"]["Steps"] = totSteps;
	results["Total"]["Sum"] = totSum;
	results["Total"]["Max"] = totMax;
	results["Total"]["Min"] = totMin;
	results["Total"]["Error"] = totErr;

	delete [] vastArrayOfStressers;

	return results;
}
