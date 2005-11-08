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
	fHostName << '_' << WD_BUILDFLAGS;
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
	fStatistics[fHostName][fDatetime][strTestName] = lMilliTime;
}

void StatisticTestCase::ExportCsvStatistics(long lModulus)
{
	StartTrace(StatisticTestCase.ExportCsvStatistics);
	Anything anyCsv;
	long lDestDateIdx = 0L;
	TraceAny(fStatistics, "Collected Statistics");
	ROAnything roaStatistics(fStatistics), roaHostEntry;
	AnyExtensions::Iterator<ROAnything> aHostIterator(roaStatistics);
	while ( aHostIterator.Next(roaHostEntry) ) {
		const char *pHostName = roaStatistics.SlotName(aHostIterator.Index());
		TraceAny(roaHostEntry, "current entry for host [" << pHostName << "]");
		ROAnything roaDateEntry;
		AnyExtensions::Iterator<ROAnything> aDateIterator(roaHostEntry);
		while ( aDateIterator.Next(roaDateEntry) ) {
			if ( aDateIterator.Index() % lModulus == 0 ) {
				lDestDateIdx++;
			}
			const char *pDT = roaHostEntry.SlotName(aDateIterator.Index());
			anyCsv["TestName"][lDestDateIdx] = pDT;
			anyCsv["HostName"][lDestDateIdx] = pHostName;
			TraceAny(roaDateEntry, "date entry per [" << pDT << "]");
			ROAnything roaCaseEntry;
			AnyExtensions::Iterator<ROAnything> aCaseIterator(roaDateEntry);
			while ( aCaseIterator.Next(roaCaseEntry) ) {
				const char *pCaseName = roaDateEntry.SlotName(aCaseIterator.Index());
				anyCsv[pCaseName][lDestDateIdx] = roaCaseEntry.AsCharPtr();
			}
		}
	}
	TraceAny(anyCsv, "collected");
	iostream *pStream = System::OpenOStream(getClassName(), "csv");
	if ( t_assert(pStream != NULL) ) {
		ROAnything roaRowEntry, roaCvs(anyCsv);
		AnyExtensions::Iterator<ROAnything> aRowIterator(roaCvs);
		while ( aRowIterator.Next(roaRowEntry) ) {
			(*pStream) << roaCvs.SlotName(aRowIterator.Index()) << ',';
			ROAnything roaCol;
			AnyExtensions::Iterator<ROAnything> aColIterator(roaRowEntry);
			while ( aColIterator.Next(roaCol) ) {
				(*pStream) << roaCol.AsString() << ',';
			}
			(*pStream) << "\r\n";
		}
	}
	delete pStream;
}
