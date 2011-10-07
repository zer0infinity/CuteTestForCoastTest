/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "StressApp.h"
#include "StringStream.h"
#include "SystemFile.h"
#include "DiffTimer.h"
#include "Stresser.h"
#include <iostream>
#include <iomanip>

RegisterApplication(StressApp);

StressApp::StressApp() :
		Application("StressApp") {
	StartTrace1(StressApp.StressApp, "StressApp");
}

StressApp::StressApp(const char *AppName) :
		Application(AppName) {
	StartTrace1(StressApp.StressApp, NotNull(AppName));
}
StressApp::~StressApp() {
}

int StressApp::DoRun() {
	StartTrace(StressApp.DoRun);
	String stresserName = Lookup("StresserName", "Default");

	Trace("Running " << stresserName);
	DiffTimer Dt;
	fResult = Stresser::RunStresser(stresserName);
	ShowResult(Dt.Diff());

	return 0;
}

void StressApp::ShowResult(long lTime) {
	StartTrace(StressApp.ShowResult);
	using namespace std;

	unsigned long totTr = 0;
	unsigned long totSteps = 0;
	unsigned long totSum = 0;
	long totMax = 0;
	long totMin = 2000000;
	long totErr = 0;
	String buf;

	// get the result from all users
	{
		OStringStream strCout(&buf);
		strCout << "\nResult:" << std::endl;
		Anything results;
		TraceAny(fResult, "fResult");
		if (!fResult.LookupPath(results, "Results")) {
			results = fResult;
		}

		long sz = results.GetSize();
		for (long i = 0; i < sz; i++) {
			Anything result = results[i];

			TraceAny(result, "Result " << i);
			unsigned long anzTr = result["Nr"].AsLong(0);
			unsigned long anzSteps = result["Steps"].AsLong(0);
			unsigned long sum = result["Sum"].AsLong(0);
			long itopia_max = result["Max"].AsLong(0);
			long itopia_min = result["Min"].AsLong(0);
			long err = result["Error"].AsLong(0);
			double trxpsec = sum ? (double(anzTr) / (double(sum) / 1000.0)) : 0;
			double avgtime = anzTr ? (double(sum) / double(anzTr)) : 0;

			// show summary for this run
			strCout << "ID: " << setw(3) << setiosflags(ios::right) << i << " Time: " << setw(4) << setiosflags(ios::right | ios::fixed)
					<< setprecision(1) << (double(sum) / 1000.0) << "s" << " Trx: " << anzTr << " Steps: " << anzSteps << " Trx/sec: "
					<< setiosflags(ios::fixed) << setprecision(2) << trxpsec;
			strCout << " AverageTime: " << setiosflags(ios::fixed) << setprecision(1) << avgtime << "ms" << " MaxTime: " << itopia_max
					<< "ms" << " MinTime: " << itopia_min << "ms";

			if (err > 0) {
				strCout << " Errors: " << err;
			}
			strCout << std::endl;

			// create nice printable anything with useful information
			Anything details, errors, infos;
			bool boHasDetails = result.LookupPath(details, "Details");
			bool boHasErrors = false;
			bool boHasInfo = result.LookupPath(infos, "InfoMessageCtr");
			if (err) {
				boHasErrors = result.LookupPath(errors, "ErrorMessageCtr");
			}
			Anything printDetails;
			for (unsigned long lSteps = 0; lSteps < anzSteps; lSteps++) {
				String strStepNr;
				strStepNr << (long) (lSteps + 1L);
				String strStepLabel = strStepNr;
				Anything anyTmp;
				if (boHasDetails && details.LookupPath(anyTmp, strStepNr)) {
					if (anyTmp.IsDefined("Label") && strStepLabel != anyTmp["Label"].AsCharPtr()) {
						strStepLabel = anyTmp["Label"].AsCharPtr();
					}Trace("current step [" << strStepNr << "]");
					Trace("steplabel [" << strStepLabel << "]");
					if (anyTmp.IsDefined("ExecTime")) {
						String execTime(anyTmp["ExecTime"].AsCharPtr("0"));
						execTime << " ms";
						printDetails[strStepLabel]["Execution Time"] = execTime;
					}
				}
				if (boHasInfo && infos.LookupPath(anyTmp, strStepNr)) {
					printDetails[strStepLabel]["Infos"] = anyTmp;
				}
				if (boHasErrors && errors.LookupPath(anyTmp, strStepNr)) {
					printDetails[strStepLabel]["Errors"] = anyTmp;
				}
			}TraceAny(printDetails, "printDetails");
			if (!printDetails.IsNull()) {
				strCout << printDetails << std::endl;
			}

			// add this result to the totals
			totTr += anzTr;
			totSteps += anzSteps;
			totSum += sum;
			totMax = std::max(itopia_max, totMax);
			totMin = std::min(itopia_min, totMin);
			totErr += err;
		}

		double totTrxpsec = totSum ? (double(totTr) / (double(totSum) / 1000.0)) : 0;
		double totAvgtime = totTr ? (double(totSum) / double(totTr)) : 0;
		strCout << "\nTotal:" << std::endl;
		strCout << " Time: " << setw(5) << setiosflags(ios::right | ios::fixed) << setprecision(1) << (double(totSum) / 1000.0) << "s"
				<< " Trx: " << totTr << " Steps: " << totSteps << " Trx/sec: " << setiosflags(ios::fixed) << setprecision(2) << totTrxpsec;
		strCout << " AverageTime: " << setiosflags(ios::fixed) << setprecision(1) << totAvgtime << "ms" << " MaxTime: " << totMax << "ms"
				<< " MinTime: " << totMin << "ms";
		if (totErr > 0) {
			strCout << " Errors: " << totErr;
		}
		strCout << std::endl << std::endl;
	}

	String fn = fConfig["ResultFile"].AsCharPtr("time.txt");
	ostream *os = Coast::System::OpenOStream(fn, 0, ios::out | ios::app);

	if (os) {
		os->write(buf, buf.Length());
		*os << "-----------------------------------------" << std::endl << std::endl;
		delete os;
		os = 0;
	}
	cout.write(buf, buf.Length());
	cout.flush();
}
