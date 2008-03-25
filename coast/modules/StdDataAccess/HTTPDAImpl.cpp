/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "HTTPDAImpl.h"

//--- standard modules used ----------------------------------------------------
#include "StringStream.h"
#include "Timers.h"
#include "ConnectorParams.h"
#include "SSLSocket.h"
#include "Dbg.h"

#ifdef RECORD
#include "AnyUtils.h"
#include "SysLog.h"
#endif

#if defined(ONLY_STD_IOSTREAM)
using namespace std;
#endif

//--- HTTPDAImpl -----------------------------------------------------
RegisterDataAccessImpl(HTTPDAImpl);

HTTPDAImpl::HTTPDAImpl(const char *name) : DataAccessImpl(name)
{
}

HTTPDAImpl::~HTTPDAImpl()
{
}

IFAObject *HTTPDAImpl::Clone() const
{
	return new HTTPDAImpl(fName);
}

String HTTPDAImpl::GenerateErrorMessage(const char *msg, Context &context)
{
	ROAnything appPref(context.Lookup("URIPrefix2ServiceMap"));
	Anything anyPrefix = appPref.SlotName(appPref.FindValue(fName));

	Anything tmp(context.GetTmpStore());
	String otherInfo = "";
	otherInfo << "Server Name:" << tmp["Backend"]["ServerName"].AsCharPtr(fName);
	otherInfo << " IP:" << tmp["Backend"]["Server"].AsCharPtr("no IP");
	otherInfo << " Port:" << tmp["Backend"]["Port"].AsCharPtr("no Port");
	String errorMsg(msg);
	errorMsg << anyPrefix.AsCharPtr(fName) << " [" << otherInfo << "] failed";
	return errorMsg;
}

bool HTTPDAImpl::Exec( Context &context, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(HTTPDAImpl.Exec);

	ConnectorParams cps(this, context);
	Trace( "Address<" << cps.IPAddress() << "> Port[" << cps.Port() << "] SSL(" << ((cps.UseSSL()) ? "yes" : "no") << ")" );

	if ( cps.UseSSL() ) {
		TraceAny(context.Lookup("SSLModuleCfg"), "SSLModuleCfg");
		ConnectorArgs ca(cps.IPAddress(), cps.Port(), cps.Timeout());
		SSLSocketArgs sa(context.Lookup("VerifyCertifiedEntity").AsBool(0),
						 context.Lookup("CertVerifyString").AsString(),
						 context.Lookup("CertVerifyStringIsFilter").AsBool(0),
						 context.Lookup("SessionResumption").AsBool(0));

		Trace(sa.ShowState());

		SSLConnector sslcsc (ca, sa, context.Lookup("SSLModuleCfg"),
							 (SSL_CTX *)context.Lookup("SSLContext").AsIFAObject(0),
							 NULL, 0L, cps.UseThreadLocal());
		return DoExec(&sslcsc, &cps, context, in, out);
	} else {
		Connector csc(cps.IPAddress(), cps.Port(), cps.Timeout(), cps.UseThreadLocal());
		return DoExec(&csc, &cps, context, in, out);
	}
	return false;
}

bool HTTPDAImpl::DoExec(Connector *csc, ConnectorParams *cps, Context &context, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(HTTPDAImpl.DoExec);

#ifdef RECORD
	return DoExecRecord( csc, cps, context, in, out);
#endif

	Trace("name: " << fName);
	Socket *s = 0;
	iostream *Ios = 0;

	{
		DAAccessTimer(HTTPDAImpl.DoExec, "Connecting <" << fName << ">", context);
		s = csc->Use();
		// Store client info
		context.GetTmpStore()["ClientInfoBackends"] = csc->ClientInfo();
		if ( s ) {
			Ios = csc->GetStream();
			if ( cps->UseSSL() ) {
				if ( s->IsCertCheckPassed(context.Lookup("SSLModuleCfg")) == false ) {
					return false;
				}
			}
		}
		if (! Ios ) {
			out->Put("Error", GenerateErrorMessage("Connection to ", context), context);
			return false;
		}
	}
	{
		DAAccessTimer(HTTPDAImpl.DoExec, " writing", context);

		if ( !SendInput(Ios, s, cps->Timeout(), context, in, out) || !(*Ios)) {
			return false;
		}
	}
	{
		DAAccessTimer(HTTPDAImpl.DoExec, " reading", context);

		if (! out->Put("Output", *Ios, context) ) {
			out->Put("Error", GenerateErrorMessage("Receiving reply of ", context), context);
			return false;
		}
		return true;
	}
}
#if defined(RECORD)
bool HTTPDAImpl::ReadReply( String &theReply, Context &context, iostream *Ios )
{
	if (Ios && !!(*Ios)) {
		char ch;

		while (Ios->get(ch)) {
			theReply.Append(ch);
		}
		return true;
	} else {
		return false ;
	}
}

bool HTTPDAImpl::RenderReply( String &theReply, Context &context, ResultMapper *out  )
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

bool HTTPDAImpl::SendRequest(String &request, iostream *Ios, Socket *s, ConnectorParams *cps )
{
	if (Ios) {
		long sockRetCode;
		if ( s->IsReadyForWriting(cps->Timeout(), sockRetCode) ) {
			s->SetNoDelay();
			(*Ios) << request;
			(*Ios) << flush;
			// don't use ShutDownWriting, since not all HTTP-Agents understand it (e.g. CICS-WebInterface)
		}
	}
	return true;
}

bool HTTPDAImpl::DoExecRecord(Connector *csc, ConnectorParams *cps, Context &context, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(HTTPDAImpl.DoExecRecordOrTest);

	Anything tmpStore = context.GetTmpStore();

	// open test config file
	Anything recording;

	Recording::ReadinRecording( String("Recording"), recording );

	Trace("name: " << fName);
	Socket *s = 0;
	iostream *Ios = 0;
	{
		DAAccessTimer(HTTPDAImpl.DoExecRecord, "connecting <" << fName << ">", context);
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

#ifdef DEBUG
	String infoMsg = "\r\nReply from server ";
	infoMsg << theReply;
	SysLog::Info( infoMsg ); // perhaps enable this line with an entry in RequestLineRenderer.any.... future
#endif

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

bool HTTPDAImpl::SendInput(iostream *Ios, Socket *s, long timeout, Context &context, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(HTTPDAImpl.SendInput);

	//***************
	Anything myany;
	in->Get("Input", myany, context);
	String body = myany.AsString();

	String contentLength = "";
	contentLength.Append(body.Length());

	context.GetTmpStore()["Request"]["BodyLength"] = contentLength;
	//***************

#ifdef COAST_TRACE
	Trace("Debug Version");

	if ( Tracer::CheckWDDebug("HTTPDAImpl.SendInput", Storage::Current()) ) {
		String request;
		{
			OStringStream os(&request);
			if ( ! in->Get("Input", os, context) ) {
				out->Put("Error", GenerateErrorMessage("Input Collection of ", context), context);
				return false;
			}
		}
		Trace("Request: <" << request << ">");

		Anything tmpStore(context.GetTmpStore());
		tmpStore["Mapper"]["RequestMade"] = request;
//		TraceAny( tmpStore["Mapper"], "tmpStore.Mapper" );

		if (Ios) {
			if ( s->IsReadyForWriting() ) {
				s->SetNoDelay();
				(*Ios) << request;
				(*Ios) << flush;
				// don't use ShutDownWriting, since not all HTTP-Agents understand it (e.g. CICS-WebInterface)
				return true;
			} else {
				out->Put("Error", GenerateErrorMessage("Sending request ", context), context);
				return false;
			}
		}
	} else
#endif
	{
		Trace("NOT Debug Version");
		return DoSendInput(Ios, s, timeout, context, in, out);
	}
	return false;
}

bool HTTPDAImpl::DoSendInput(iostream *Ios, Socket *s, long timeout, Context &context, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(HTTPDAImpl.DoSendInput);

	if ( Ios ) {
		if ( s->IsReadyForWriting() ) {
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
