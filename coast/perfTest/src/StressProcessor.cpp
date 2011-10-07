/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "StressProcessor.h"
#include "Stresser.h"

RegisterRequestProcessor(StressProcessor);

StressProcessor::StressProcessor(const char *name) :
		RequestProcessor(name) {
}

StressProcessor::~StressProcessor() {
}

bool StressProcessor::DoProcessRequest(std::ostream &reply, Context &ctx) {
	StartTrace(StressProcessor.DoProcessRequest);
	TraceAny(ctx.GetRequest(), "Request");

	String stresserName(ctx.Lookup("StresserName", "Default"));
	Trace("Stresser Name : " << stresserName);

	Anything result = Stresser::RunStresser(stresserName);
	TraceAny(result, "Results");
	result.PrintOn(reply, false);
	return true;
}
