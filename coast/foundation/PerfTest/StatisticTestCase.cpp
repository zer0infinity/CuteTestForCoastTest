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
#include "Dbg.h"
#include "DiffTimer.h"
#include "System.h"

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
	fStatistics[fDatetime][strTestName] = lMilliTime;
}

void StatisticTestCase::ExportCsvStatistics(long lModulus)
{
	StartTrace(StatisticTestCase.ExportCsvStatistics);
	Anything anyCsv;
	long lDestDateIdx = 0L;
	for (long lDateIdx = 0L; lDateIdx < fStatistics.GetSize(); lDateIdx++) {
		if ( lDateIdx % lModulus == 0 ) {
			lDestDateIdx++;
		}
		const char *pDT = fStatistics.SlotName(lDateIdx);
		anyCsv["TestName"][lDestDateIdx] = pDT;
		for (long lCaseIdx = 0L; lCaseIdx < fStatistics[lDateIdx].GetSize(); lCaseIdx++) {
			const char *pCaseName = fStatistics[lDateIdx].SlotName(lCaseIdx);
			anyCsv[pCaseName][lDestDateIdx] = fStatistics[lDateIdx][lCaseIdx].AsCharPtr();
		}
	}
	TraceAny(anyCsv, "collected");
	iostream *pStream = System::OpenOStream(getClassName(), "csv");
	if ( t_assert(pStream != NULL) ) {
		for (long lRows = 0L; lRows < anyCsv.GetSize(); lRows++) {
			(*pStream) << anyCsv.SlotName(lRows) << ',';
			for (long lCol = 1L; lCol < anyCsv[lRows].GetSize(); lCol++) {
				(*pStream) << anyCsv[lRows][lCol].AsString() << ',';
			}
			(*pStream) << "\r\n";
		}
	}
	delete pStream;
}
