/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- interface include --------------------------------------------------------
#include "StatisticTestCase.h"

//--- standard modules used ----------------------------------------------------
#include "DiffTimer.h"
#include "System.h"
#include "AnyIterators.h"

//--- c-library modules used ---------------------------------------------------

//---- StatisticTestCase ----------------------------------------------------------------
StatisticTestCase::StatisticTestCase(TString tstrName) : TestCase(tstrName)
{
	StartTrace(StatisticTestCase.Ctor);
}

StatisticTestCase::~StatisticTestCase()
{
	StartTrace(StatisticTestCase.Dtor);
}

void StatisticTestCase::LoadData()
{
	StartTrace(StatisticTestCase.LoadData);
	if ( !t_assertm(System::LoadConfigFile(fStatistics, getClassName(), "any", fFilename), "could not load statistics file!") ) {
		fFilename = getClassName();
		fFilename << ".any";
	}
	fDatetime = GenTimeStamp();
	System::HostName(fHostName);
	fFlags << WD_BUILDFLAGS << '_' << WD_COMPILER;
	TraceAny(fStatistics, "filename of statistics file is [" << fFilename << "], timestamp [" << fDatetime << "]");
}

void StatisticTestCase::StoreData()
{
	StartTrace(StatisticTestCase.StoreData);
	iostream *pStream = System::OpenOStream(fFilename, "");
	if ( t_assert(pStream != NULL) ) {
		fStatistics.PrintOn(*pStream, true);
	}
	delete pStream;
}

void StatisticTestCase::setUp()
{
	StartTrace(StatisticTestCase.setUp);
	LoadData();
}

void StatisticTestCase::tearDown()
{
	StartTrace(StatisticTestCase.tearDown);
	StoreData();
}

String StatisticTestCase::GenTimeStamp()
{
	StartTrace(StatisticTestCase.GenTimeStamp);
	const int dateSz = 40;
	time_t now = time(0);
	struct tm res, *tt;
	tt = System::LocalTime(&now, &res);

	char date[dateSz];
	strftime(date, dateSz, "%Y%m%d%H%M%S", tt);
	return date;
}

void StatisticTestCase::AddStatisticOutput(String strTestName, long lMilliTime)
{
	StartTrace(StatisticTestCase.AddStatisticOutput);
	fStatistics[strTestName][fHostName][WD_BUILDFLAGS][WD_COMPILER][fDatetime] = lMilliTime;
}

double StatisticTestCase::RecurseExportCsvStatistics(Anything &anyCsv, ROAnything roaLoopOn, long &lLevel)
{
	StartTrace(StatisticTestCase.RecurseExportCsvStatistics);
	long lEntryLevel = lLevel;
	Trace("total levels:" << lLevel);
	long lEntries = 0;
	double dAverage = 0.0;
	AnyExtensions::Iterator<ROAnything> aEntryIterator(roaLoopOn);
	ROAnything roaEntry;
	while ( aEntryIterator.Next(roaEntry) ) {
		const char *pEntryName = roaLoopOn.SlotName(aEntryIterator.Index());
		double dValue = 0.0;
		if ( roaEntry.GetType() == AnyArrayType ) {
			lEntryLevel = lLevel;
			dValue = RecurseExportCsvStatistics(anyCsv, roaEntry, ++lEntryLevel);
			Trace("recursion level:" << lEntryLevel);
		} else {
			dValue = (double)roaEntry.AsLong(0L);
		}
		long lIdx = 0;
		String strOut;
		for ( lIdx = 0; lIdx < lLevel; ++lIdx) {
			strOut << ',';
		}
		strOut << pEntryName;
		for ( lIdx = lLevel; lIdx <= lEntryLevel; ++lIdx) {
			strOut << ',';
		}
		strOut << dValue;
		Trace("appending [" << strOut << "]");
		anyCsv.Append(strOut);

		dAverage += dValue;
		++lEntries;
	}
	lLevel = lEntryLevel;
	if ( lEntries > 0 ) {
		dAverage /= (double)lEntries;
	}
	return dAverage;
}

void StatisticTestCase::ExportCsvStatistics(long lModulus)
{
	StartTrace(StatisticTestCase.ExportCsvStatistics);
	Anything anyCsv;
	TraceAny(fStatistics, "Collected Statistics");
	long lLevel = 1;
	double dTotalAvg = RecurseExportCsvStatistics(anyCsv, fStatistics, lLevel);

	TraceAny(anyCsv, "collected");
	iostream *pStream = System::OpenOStream(getClassName(), "csv");
	if ( t_assert(pStream != NULL) ) {
		for ( long lRowIdx = anyCsv.GetSize(); --lRowIdx >= 0; ) {
			Trace("printing line [" << anyCsv[lRowIdx].AsCharPtr() << "]");
			(*pStream) << anyCsv[lRowIdx].AsCharPtr() << "\r\n";
		}
	}
	(*pStream) << "Total Average";
	for ( long lIdx = 0; lIdx <= lLevel; ++lIdx) {
		(*pStream) << ',';
	}
	(*pStream) << dTotalAvg << "\r\n";
	delete pStream;
}
