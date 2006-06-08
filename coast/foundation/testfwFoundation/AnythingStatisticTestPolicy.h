/*
 * Copyright (c) 2006, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AnythingStatisticTestPolicy_H
#define _AnythingStatisticTestPolicy_H

//---- baseclass include -------------------------------------------------
#include "AnyIterators.h"
#include "StringStream.h"
#include "System.h"

//---- forward declaration -----------------------------------------------

//---- AnythingStatisticTestPolicy ----------------------------------------------------------
//! <B>really brief class description</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
namespace TestFramework
{

	template
	<
	class dummy
	>
	class AnythingStatisticTestPolicy
	{
		Anything fStatistics;
		String fStatClassName, fTestName, fFilename, fDatetime, fHostName;

	public:
		typedef AnythingStatisticTestPolicy<dummy> StatisticPolicyType;

		AnythingStatisticTestPolicy() {};
		virtual ~AnythingStatisticTestPolicy() {};

		friend class CatchTime;
		class CatchTime
		{
		public:
			CatchTime(TString strTestName, StatisticPolicyType *theTest)
				: fTestName(strTestName)
				, fTest(theTest) {
			}
			~CatchTime() {
				if ( fTest ) {
					fTest->AddStatisticOutput(fTestName, fTimer.Diff());
				}
			}
		private:
			TString fTestName;
			StatisticPolicyType *fTest;
			TestTimer fTimer;
		};
		typedef typename StatisticPolicyType::CatchTime CatchTimeType;

	protected:
		//! export timing values in csv style
		void LoadData(TString strClassName, TString strTestName) {
			StartTrace(AnythingStatisticTestPolicy.LoadData);
			fStatClassName = strClassName;
			fTestName = strTestName;
			System::LoadConfigFile(fStatistics, strClassName, "stat.any", fFilename);
			if ( !fFilename.Length() ) {
				fFilename = strClassName;
				fFilename << ".stat.any";
			}
			fDatetime = GenTimeStamp();
			System::HostName(fHostName);
			TraceAny(fStatistics, "filename of statistics file is [" << fFilename << "], timestamp [" << fDatetime << "]");
		}

		void StoreData() {
			StartTrace(AnythingStatisticTestPolicy.StoreData);
			iostream *pStream = System::OpenOStream(fFilename, "");
			if ( pStream != NULL ) {
				fStatistics.PrintOn(*pStream, true);
			}
			delete pStream;
		}

		void ExportCsvStatistics() {
			StartTrace(AnythingStatisticTestPolicy.ExportCsvStatistics);
			Anything anyCsv;
			TraceAny(fStatistics, "Collected Statistics");
			long lLevel = 0;
			RecurseExportCsvStatistics(anyCsv, fStatistics, lLevel);

			TraceAny(anyCsv, "collected");
			iostream *pStream = System::OpenOStream(fStatClassName, "csv");
			if ( pStream != NULL ) {
				long lRowIdx = anyCsv.GetSize();
				if ( lRowIdx > 0 ) {
					// add entry header
					(*pStream) << "measurement description,name of host tests were run on,compilation flags,compiler version,timestamp of testrun,duration/average in ms" << "\r\n";
				}
				for ( ; --lRowIdx >= 0; ) {
					Trace("printing line [" << anyCsv[lRowIdx].AsCharPtr() << "]");
					(*pStream) << anyCsv[lRowIdx].AsCharPtr() << "\r\n";
				}
			}
			delete pStream;
		}

		void AddStatisticOutput(TString strSummaryName, long lMilliTime) {
			StartTrace(AnythingStatisticTestPolicy.AddStatisticOutput);
			fStatistics[strSummaryName][fHostName][WD_BUILDFLAGS][WD_COMPILER][fDatetime] = lMilliTime;
		}

	private:
		double RecurseExportCsvStatistics(Anything &anyCsv, ROAnything roaLoopOn, long &lLevel) {
			StartTrace(AnythingStatisticTestPolicy.RecurseExportCsvStatistics);
			long lEntryLevel = lLevel;
			Trace("total levels:" << lLevel);
			double dAverage = 0.0;
			AnyExtensions::Iterator<ROAnything, ROAnything, String> aEntryIterator(roaLoopOn);
			ROAnything roaEntry;
			long lThisLevelEntries = 0L;
			while ( aEntryIterator.Next(roaEntry) ) {
				String strEntryName;
				aEntryIterator.SlotName(strEntryName);
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
				strOut << strEntryName;
				for ( lIdx = lLevel; lIdx <= lEntryLevel; ++lIdx) {
					strOut << ',';
				}
				if ( lLevel > 0 ) {
					strOut << dValue;
				}
				Trace("appending [" << strOut << "]");
				anyCsv.Append(strOut);

				dAverage += dValue;
				++lThisLevelEntries;
			}
			if ( lThisLevelEntries > 1L ) {
				dAverage /= (double)lThisLevelEntries;
			}
			lLevel = lEntryLevel;
			return dAverage;
		}

		String GenTimeStamp() {
			StartTrace(AnythingStatisticTestPolicy.GenTimeStamp);
			const int dateSz = 40;
			time_t now = time(0);
			struct tm res, *tt;
			tt = System::LocalTime(&now, &res);

			char date[dateSz];
			strftime(date, dateSz, "%Y%m%d%H%M%S", tt);
			return date;
		}
	};

}	// end namespace TestFramework

#endif
