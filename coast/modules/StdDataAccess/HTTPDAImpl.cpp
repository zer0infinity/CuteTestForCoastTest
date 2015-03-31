/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */
#include "HTTPDAImpl.h"
#include "StringStream.h"
#include "Timers.h"
#include "ConnectorParams.h"
#include "SSLSocket.h"
#ifdef RECORD
#include "AnyUtils.h"
#include "SystemLog.h"
#endif
RegisterDataAccessImpl( HTTPDAImpl);

String HTTPDAImpl::GenerateErrorMessage(const char *msg, Context &context) {
	ROAnything appPref(context.Lookup("URIPrefix2ServiceMap"));
	Anything anyPrefix = appPref.SlotName(appPref.FindValue(GetName()));
	String errorMsg(msg);
	errorMsg << anyPrefix.AsString(GetName()) << "[";
	errorMsg << "Server:" << context.Lookup("Backend.Server").AsString("no IP");
	errorMsg << " Port:" << context.Lookup("Backend.Port").AsString("no Port");
	errorMsg << "] failed";
	return errorMsg;
}

bool HTTPDAImpl::Exec(Context &context, ParameterMapper *in, ResultMapper *out) {
	StartTrace1(HTTPDAImpl.Exec, GetName());

	ConnectorParams cps(context, in);
	Trace( "Address<" << cps.IPAddress() << "> Port[" << cps.Port() << "] SSL(" << ((cps.UseSSL()) ? "yes" : "no") << ")" );

	if (cps.UseSSL()) {
		TraceAny(context.Lookup("SSLModuleCfg"), "SSLModuleCfg");
		ConnectorArgs ca(cps.IPAddress(), cps.Port(), cps.Timeout());
		SSLSocketArgs sa(context.Lookup("VerifyCertifiedEntity").AsBool(0), context.Lookup("CertVerifyString").AsString(),
				context.Lookup("CertVerifyStringIsFilter").AsBool(0), context.Lookup("SessionResumption").AsBool(0));

		Trace(sa.ShowState());
		SSLConnector sslcsc(ca, sa, context.Lookup("SSLModuleCfg"), (SSL_CTX *) context.Lookup("SSLContext").AsIFAObject(0), NULL, 0L,
				cps.UseThreadLocal());
		return DoExec(&sslcsc, &cps, context, in, out);
	} else {
		Connector csc(cps.IPAddress(), cps.Port(), cps.Timeout(), "", 0, cps.UseThreadLocal());
		return DoExec(&csc, &cps, context, in, out);
	}
	return false;
}

bool HTTPDAImpl::DoExec(Connector *csc, ConnectorParams *cps, Context &context, ParameterMapper *in, ResultMapper *out) {
	StartTrace1(HTTPDAImpl.DoExec, GetName());
#ifdef RECORD
	return DoExecRecord( csc, cps, context, in, out);
#endif
	Socket *s = 0;
	std::iostream *Ios = 0;
	{
		DAAccessTimer(HTTPDAImpl.DoExec, "Connecting <" << GetName() << ">", context);
		s = csc->Use();
		// Store client info
		context.GetTmpStore()["ClientInfoBackends"] = csc->ClientInfo();
		if (s) {
			Ios = csc->GetStream();
			if (cps->UseSSL()) {
				if (s->IsCertCheckPassed(context.Lookup("SSLModuleCfg")) == false) {
					return false;
				}
			}
		}
		if (!Ios) {
			out->Put("Error", GenerateErrorMessage("Connection to ", context), context);
			return false;
		}
	}
	{
		DAAccessTimer(HTTPDAImpl.DoExec, " writing", context);
		if (!SendInput(Ios, s, cps->Timeout(), context, in, out) || !(*Ios)) {
			return false;
		}
	}
	{
		DAAccessTimer(HTTPDAImpl.DoExec, " reading", context);
		bool bPutSuccess = out->Put("Output", *Ios, context);
		if (TriggerEnabled(HTTPDAImpl.DoExec.Stores)) {
			OStringStream ostr;
			context.DebugStores(0, ostr, true);
			SubTrace(Stores, ostr.str());
		}
		if (!bPutSuccess) {
			out->Put("Error", GenerateErrorMessage("Receiving reply of ", context), context);
		}
		return bPutSuccess;
	}
}
#if defined(RECORD)
bool HTTPDAImpl::ReadReply( String &theReply, Context &context, std::iostream *Ios )
{
	if (Ios && !!(*Ios)) {
		char ch;

		while (Ios->get(ch)) {
			theReply.Append(ch);
		}
		return true;
	} else {
		return false;
	}
}

bool HTTPDAImpl::RenderReply( String &theReply, Context &context, ResultMapper *out )
{
	IStringStream is(theReply);

	// read from input stream via renderers....
	if (! out->Put("Output", is, context) ) {
		out->Put("Error", GenerateErrorMessage("Receiving reply of ", context), context);
		return false;
	}
	return true;
}

bool HTTPDAImpl::BuildRequest( String &request, Context &context, ParameterMapper *in, ResultMapper *out )
{
	StartTrace(HTTPDAImpl.BuildRequest);
	{
		OStringStream os(&request);
		if ( ! in->Get("Input", os, context) ) {
			out->Put("Error", GenerateErrorMessage("Input Collection of ", context), context);
			return false;
		}
	}

	Anything tmpStore(context.GetTmpStore());
	tmpStore["ParameterMapper"]["RequestMade"] = request;
	TraceAny( tmpStore["Mapper"], "tmpStore.Mapper" );

	return true; // request built successfully
}

bool HTTPDAImpl::SendRequest(String &request, std::iostream *Ios, Socket *s, ConnectorParams *cps )
{
	if (Ios) {
		long sockRetCode;
		if ( s->IsReadyForWriting(cps->Timeout(), sockRetCode) ) {
			s->SetNoDelay();
			(*Ios) << request;
			(*Ios) << std::flush;
			// don't use ShutDownWriting, since not all HTTP-Agents understand it (e.g. CICS-WebInterface)
		}
	}
	return true;
}

bool HTTPDAImpl::DoExecRecord(Connector *csc, ConnectorParams *cps, Context &context, ParameterMapper *in, ResultMapper *out)
{
	StartTrace1(HTTPDAImpl.DoExecRecord, GetName());

	Anything tmpStore = context.GetTmpStore();

	// open test config file
	Anything recording;

	Recording::ReadinRecording( String("Recording"), recording );
	Socket *s = 0;
	std::iostream *Ios = 0;
	{
		DAAccessTimer(HTTPDAImpl.DoExecRecord, "connecting <" << GetName() << ">", context);
		s = csc->Use();
		if ( s ) {
			Ios = csc->GetStream();
		}
		if (! Ios || !(*Ios)) {
			out->Put("Error", GenerateErrorMessage("Connection to ", context), context);
			return false;
		}
	}

	DAAccessTimer(HTTPDAImpl.DoExecRecord, " writing", context);

	String request;

	if ( ! BuildRequest( request, context, in, out ) ) {
		return false;
	};

	String theReply;

	bool result = SendRequest( request, Ios, s, cps ); // really send request to server...
	if (result && !ReadReply( theReply, context, Ios )) {
		out->Put("Error", GenerateErrorMessage("Connection to ", context), context);
		result = false;
	}

	if ( TriggerEnabled(HTTPDAImpl.DoExecRecordOrTest) ) {
		String infoMsg = "\r\nReply from server ";
		infoMsg << theReply;
		SystemLog::Info( infoMsg ); // perhaps enable this line with an entry in RequestLineRenderer.any.... future
	}

	if (! RenderReply( theReply, context, out ) ) {
		return false;
	};

	// write out recording
	Anything conv = Anything( recording.GetSize() );
	String index = conv.AsString();
	//	TestConfigRecordingSoFar[index][slotName]= tmpStore;

	recording[index]["Request"] = request;
	recording[index]["Reply"] = theReply;
	Recording::WriteoutRecording( String("Recording"), recording );

	return result;
}
#endif

#include "utf8.h"

namespace {
	long getStringLength(String const &str) {
		long len = 0L;
		try {
			len = utf8::distance(str.begin(), str.end());
		} catch (utf8::invalid_utf8& e) {
			len = str.Length();
		}
		StatTrace(HTTPDAImpl.getStringLength, "len: " << len << " str [" << str << "]", coast::storage::Current());
		return len;
	}
}

bool HTTPDAImpl::SendInput(std::iostream *Ios, Socket *s, long timeout, Context &context, ParameterMapper *in, ResultMapper *out) {
	StartTrace(HTTPDAImpl.SendInput);
	if (TriggerEnabled(HTTPDAImpl.SendInput)) {
		String request(16384L);
		{
			OStringStream os(request);
			if (!in->Get("Input", os, context)) {
				Trace("getting Input failed, content so far:" << request);
				out->Put("Error", GenerateErrorMessage("Input Collection of ", context), context);
				return false;
			}
		}
		Trace("Request:" << request);
		Anything tmpStore(context.GetTmpStore());
		tmpStore["Mapper"]["RequestMade"] = request;
		if (Ios) {
			if (s->IsReadyForWriting()) {
				Trace("sending input");
				s->SetNoDelay();
				(*Ios) << request;
				(*Ios) << std::flush;
				// don't use ShutDownWriting, since not all HTTP-Agents understand it (e.g. CICS-WebInterface)
				return true;
			} else {
				Trace("socket not ready for writing");
				out->Put("Error", GenerateErrorMessage("Sending request ", context), context);
				return false;
			}
		}
	} else {
		return DoSendInput(Ios, s, timeout, context, in, out);
	}
	return false;
}

bool HTTPDAImpl::DoSendInput(std::iostream *Ios, Socket *s, long timeout, Context &context, ParameterMapper *in, ResultMapper *out) {
	StartTrace(HTTPDAImpl.DoSendInput);
	if (Ios) {
		if (s->IsReadyForWriting()) {
			s->SetNoDelay();
			bool retCode = in->Get("Input", *Ios, context);
			Ios->flush();
			if (retCode) {
				return true;
			}
		}
	}
	out->Put("Error", GenerateErrorMessage("Sending request ", context), context);
	return false;
}
