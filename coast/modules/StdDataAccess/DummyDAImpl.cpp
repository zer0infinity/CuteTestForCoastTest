/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */
#include "DummyDAImpl.h"
#include "StringStream.h"
#include "Timers.h"
#include "AnyUtils.h"
RegisterDataAccessImpl( DummyDAImpl);

bool DummyDAImpl::HandleError(const char *msg, Context &context) {
	// does very little right now...
	String errorMsg(msg);
	return false;
}

bool DummyDAImpl::Exec(Context &context, ParameterMapper *in, ResultMapper *out) {
	StartTrace(DummyDAImpl.Exec);
	return DoExec(context, in, out);
}

bool DummyDAImpl::RenderReply(String &theReply, Context &context, ResultMapper *out) {
	IStringStream is(theReply);

	// read from input stream via renderers....
	if (!out->Put("Output", is, context)) {
		return HandleError("receiving reply of ", context);
	}

	return true;
}

bool DummyDAImpl::BuildRequest(String &request, Context &context, ParameterMapper *in) {
	Anything tmpStore(context.GetTmpStore());
	StartTrace(DummyDAImpl.BuildRequest);
	{
		OStringStream os(&request);
		if (!in->Get("Input", os, context)) {
			return HandleError("Input Collection of ", context);
		}
	}
	if (TriggerEnabled(DummyDAImpl.BuildRequest)) {
		tmpStore["ParameterMapper"]["RequestMade"] = request;
	}
	return true; // request built successfully
}

String DummyDAImpl::GetReplyMatchingRequest(Anything &recording, Context &context, String &request) {
	long testStepNo = 0;
	Anything tmpStore(context.GetTmpStore());

	if (tmpStore.IsDefined("TestStepNo")) {
		testStepNo = tmpStore["TestStepNo"].AsLong(-1L);
		++testStepNo;
	}
	tmpStore["TestStepNo"] = testStepNo;

	Anything conv = Anything(testStepNo);
	String index = conv.AsString();

	if (recording[index].IsDefined("Request")) {
		String recordedRequest = recording[index]["Request"].AsString("");

		if (!recordedRequest.IsEqual(request)) {
			String eMsg = "Error: at index ";
			eMsg << index << " request did not match recorded request:" << request;
			SystemLog::Warning("Error: request did not match recorded request");
		}

		String eMsg = "Reply not found, index:";
		eMsg << index << " Request:" << request;
		// done this way because you don't write to any if you check first... which might be important in future...not now..
		if (!recording[index].IsDefined("Reply")) {
			return eMsg;
		}
		return recording[index]["Reply"].AsString(eMsg);
	} else {
		String eMsg = "Request not found, index:";
		eMsg << index << " Request:" << request;
		return eMsg;
	}
}

bool DummyDAImpl::DoExec(Context &context, ParameterMapper *in, ResultMapper *out) {
	StartTrace(DummyDAImpl.DoExecRecordOrTest);
	DAAccessTimer("DummyDAImpl.Exec", "DummyDAImpl.Exec writing", context);

	Anything tmpStore = context.GetTmpStore();

	// open test config file
	Anything recording;
	String str("Recording");
	Recording::ReadinRecording(str, recording);

	Trace("name: " << fName);

	String request;
	if (!BuildRequest(request, context, in)) {
		return false;
	}

	String theReply = GetReplyMatchingRequest(recording, context, request);
	if (TriggerEnabled(DummyDAImpl.DoExecRecordOrTest)) {
		String infoMsg = "\r\nReply from server ";
		infoMsg << theReply;
		SystemLog::Info(infoMsg); // perhaps enable this line with an entry in RequestLineRenderer.any.... future
	}
	if (!RenderReply(theReply, context, out)) {
		return false;
	}
	return true;
}
