/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "BPLDAImpl.h"

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

//--- BPLDAImpl -----------------------------------------------------
RegisterDataAccessImpl(BPLDAImpl);

BPLDAImpl::BPLDAImpl(const char *name) : HTTPDAImpl(name)
{
}

BPLDAImpl::~BPLDAImpl()
{
}

IFAObject *BPLDAImpl::Clone() const
{
	return new BPLDAImpl(fName);
}

////////String BPLDAImpl::GenerateErrorMessage(const char *msg, Context &context)
////////{
////////	ROAnything appPref(context.Lookup("URIPrefix2ServiceMap"));
////////	Anything anyPrefix= appPref.SlotName(appPref.FindValue(fName));
////////
////////	Anything tmp(context.GetTmpStore());
////////	String otherInfo= "";
////////	otherInfo << "Server Name:" << tmp["Backend"]["ServerName"].AsCharPtr(fName);
////////	otherInfo << " IP:" << tmp["Backend"]["Server"].AsCharPtr("no IP");
////////	otherInfo << " Port:" << tmp["Backend"]["Port"].AsCharPtr("no Port");
////////	String errorMsg(msg); errorMsg << anyPrefix.AsCharPtr(fName) << " [" << otherInfo << "] failed";
////////	return errorMsg;
////////}

bool BPLDAImpl::Exec( Context &context, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(BPLDAImpl.Exec);

	String strServer, strPort, strSSL;
	in->Get("Server", strServer, context);
	Trace("Server [" << strServer << "]");
	in->Get("Port", strPort, context);
	Trace("Port [" << strPort << "]");
	in->Get("UseSSL", strSSL, context);
	Trace("UseSSL [" << strSSL << "]");
	Anything anyAddrConfig;
	anyAddrConfig["Server"] = strServer;
	anyAddrConfig["Port"] = strPort;
	anyAddrConfig["UseSSL"] = strSSL;
	TraceAny(anyAddrConfig, "addrConfig");
	Context::PushPopEntry<Anything> roaEntry(context, "RenderedServerParams", anyAddrConfig, "CurrentServer");
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

bool BPLDAImpl::DoExec(Connector *csc, ConnectorParams *cps, Context &context, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(BPLDAImpl.DoExec);

	Trace("name: " << fName);
	Socket *s = 0;
	iostream *Ios = 0;

	String procedureName, procedureParams, bplQuery;
	ROAnything roaServer;

	// in->Get("Server", server, context);
	roaServer = context.Lookup("BPLServer");

	in->Get("ProcedureName", procedureName, context);
	in->Get("ProcedureParams", procedureParams, context);

	Trace("#" << procedureName << "# Proc Name");
	Trace("#" << procedureParams << "# Proc Params");

	bplQuery = procedureName << " " << procedureParams;

	while (procedureName.Replace(" ", ""));

	out->Put("Query", bplQuery, context);
	out->Put("QuerySource", roaServer.AsString(), context);

	if (procedureName.Length() == 0 && procedureParams.Length() == 0) {
		Trace("Return true because proc name and params are empty");
		return true;
	}

	{
		DAAccessTimer(BPLDAImpl.DoExec, "Connecting <" << fName << ">", context);
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
			Trace("ERROR ON Iso");
			return false;
		}
	}

	{
		context.Push("ParameterMapper", in);
		String strDummy;
		DAAccessTimer(BPLDAImpl.DoExec, " writing", context);

		if ( !SendInput(Ios, s, cps->Timeout(), context, in, out) || !(*Ios)) {
			Trace("ERROR ON SendInput");
			context.Pop(strDummy);
			return false;
		}
		context.Pop(strDummy);
	}
	{
		DAAccessTimer(BPLDAImpl.DoExec, " reading", context);

		String waste;

		if (Ios && !!(*Ios)) {
			char ch;

			while (Ios->get(ch)) {
				waste.Append(ch);
				if (ch == '{') {
					break;
				}
			}
		}

		Trace("WASTE: " << waste);

		Anything responseAny;
		*Ios >> responseAny;

//		TraceAny(responseAny, "Response Anything");

		if (responseAny["Status"]["Code"] != "200") {
			Trace("ERROR Status Code");
			TraceAny(responseAny, "ERROR ANY RESP");
			out->Put("Error", GenerateErrorMessage(responseAny["Status"]["Message"].AsString(), context), context);
			return false;
		}

		responseAny.Remove("Status");

//		TraceAny(responseAny, "Response Anything");

		long responseSize = responseAny.GetSize();
		out->Put("QueryCount", responseSize, context);

		for (long i = 0; i < responseSize; i++) {
//			TraceAny(responseAny[i], "anypart " << i);
			out->Put("QueryResult", responseAny[i], context);
		}

//		if (! out->Put("QueryResult", responseAny, context) )
//		{
//			out->Put("Error",GenerateErrorMessage("Receiving reply of ", context),context);
//			return false;
//		}
//
//		ROAnything roAnyTmp;
//
//		context.Lookup ("DBselectExtListings", roAnyTmp);
//
//		TraceAny(roAnyTmp,"RESULT roAnyTmp");

		return true;
	}
}

////////bool BPLDAImpl::SendInput(iostream *Ios, Socket *s, long timeout, Context &context, ParameterMapper *in, ResultMapper *out)
////////{
////////	StartTrace(BPLDAImpl.SendInput);
////////
////////#ifdef DEBUG
////////		String request;
////////		{
////////			OStringStream os(&request);
////////			if ( ! in->Get("Input", os, context) )
////////			{
////////				out->Put("Error",GenerateErrorMessage("Input Collection of ", context), context);
////////				return false;
////////			}
////////		}
////////
////////		Trace("Request: <" << request << ">");
////////
////////#endif
////////	{
////////		Trace("NOT Debug Version");
////////		return DoSendInput(Ios, s, timeout, context, in,out);
////////	}
////////	return false;
////////}
////////
////////bool BPLDAImpl::DoSendInput(iostream *Ios, Socket *s, long timeout, Context &context, ParameterMapper *in, ResultMapper *out)
////////{
////////	StartTrace(BPLDAImpl.DoSendInput);
////////
////////	if ( Ios )
////////	{
////////		if ( s->IsReadyForWriting() )
////////		{
////////			s->SetNoDelay();
////////			bool retCode= in->Get("Input", *Ios, context);
////////			Ios->flush();
////////			if (retCode) return true;
////////
////////
////////		}
////////	}
////////	out->Put("Error",GenerateErrorMessage("Sending request ", context), context);
////////	return false;
////////}

