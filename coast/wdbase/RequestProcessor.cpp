/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "RequestProcessor.h"

//--- standard modules used ----------------------------------------------------
#include "Socket.h"
#include "StringStream.h"
#include "Registry.h"
#include "Server.h"
#include "ServerUtils.h"
#include "SystemLog.h"
#include "Dbg.h"

//--- RequestProcessor ----------------------------------------------------------
RegCacheImpl(RequestProcessor)
; // FindRequestProcessor()
RegisterRequestProcessor(RequestProcessor);

RequestProcessor::RequestProcessor(const char *processorName) :
	RegisterableObject(processorName), fServer(0) {
	StartTrace(RequestProcessor.RequestProcessor);
}

void RequestProcessor::Init(Server *server) {
	StartTrace(RequestProcessor.Init);
	fServer = server;
}

void RequestProcessor::ProcessRequest(Context &ctx) {
	StartTrace(RequestProcessor.ProcessRequest);
	ctx.SetServer(GetServer());
	Socket *socket = ctx.GetSocket();
	std::iostream *Ios = 0;

	if (socket != (Socket *) NULL) {
		ROAnything timeout;
		ctx.Lookup("SocketReadTimeout", timeout);
		socket->SetTimeout(timeout.AsLong(10 * 1000L));
		TraceAny(socket->ClientInfo(), "socket client info");
		Ios = socket->GetStream();
	}

	if (Ios && socket->IsReadyForReading()) {
		// disable nagle algorithm
		socket->SetNoDelay();
		DoReadInput(*Ios, ctx);

		if (!(*Ios)) {
			return;
		}

		KeepConnectionAlive(ctx); //!@FIXME: should not be a feature of a non-HTTP request

		if (socket->IsReadyForWriting()) {
			if (VerifyRequest(ctx)) {
				// process this arguments and
				// write the output to the reply
				DoProcessRequest(*Ios, ctx);
				//!@FIXME BufferReply mechanism possible
			} else {
				HandleVerifyError(*Ios, ctx);
			}
		}
	} else {
		String logMsg("No valid stream from socket");
		SystemLog::eLogLevel level(SystemLog::eWARNING);
		if (socket) {
			logMsg << ", fd:" << ((socket) ? socket->GetFd() : -1L);
			logMsg << ", from " << socket->ClientInfo()["REMOTE_ADDR"].AsString() << ':' << socket->ClientInfo()["REMOTE_PORT"].AsString();
			if (socket->HadTimeout()) {
				logMsg << ", had timeout (" << socket->GetTimeout() << "ms)";
				level = SystemLog::eINFO;
			}
		}
		SystemLog::Log(level, logMsg);
	}
}

namespace {
	RequestProcessor *GetCurrentRequestProcessor(Context &ctx) {
		RequestProcessor *pProc = 0;
		((ctx.GetServer() && (pProc = ctx.GetServer()->GetRequestProcessor())) || (pProc = RequestProcessor::FindRequestProcessor(
				ctx.Lookup("RequestProcessor", "RequestProcessor")))
				|| (pProc = RequestProcessor::FindRequestProcessor("RequestProcessor")));
		return pProc;
	}
}

bool RequestProcessor::VerifyRequest(Context &ctx) {
	StartTrace(RequestProcessor.VerifyRequest);
	return DoVerifyRequest(ctx);
}

bool RequestProcessor::DoVerifyRequest(Context &ctx) {
	StartTrace(RequestProcessor.DoVerifyRequest);
	return true;
}

void RequestProcessor::HandleVerifyError(std::ostream &reply, Context &ctx) {
	StartTrace(RequestProcessor.HandleVerifyError);
	DoHandleVerifyError(reply, ctx);
}

void RequestProcessor::DoHandleVerifyError(std::ostream &reply, Context &ctx) {
	StartTrace(RequestProcessor.DoHandleVerifyError);
}

void RequestProcessor::ForceConnectionClose(Context &ctx) {
	ctx.GetTmpStore()["Keep-Alive"] = 0L;
}

bool RequestProcessor::KeepConnectionAlive(Context &ctx) {
	bool retVal = false;
	StatTrace(RequestProcessor.KeepConnectionAlive, "PersistentConnections:" << (ctx.Lookup("PersistentConnections").AsBool(false) ? "true" : "false"), Storage::Current());
	if (ctx.Lookup("PersistentConnections").AsBool(false)) {
		// first check if we already know the result
		ROAnything lookupAny;
		if (ctx.Lookup("Keep-Alive", lookupAny) && !lookupAny.IsNull()) {
			retVal = lookupAny.AsBool();
		} else {
			// let the current RequestProcessor decide
			retVal = GetCurrentRequestProcessor(ctx)->DoKeepConnectionAlive(ctx);
			ctx.GetTmpStore()["Keep-Alive"] = retVal;
		}
	}
	StatTrace(RequestProcessor.KeepConnectionAlive, "Keep-Alive:" << (retVal ? "true" : "false"), Storage::Current());
	return retVal;
}

bool RequestProcessor::DoKeepConnectionAlive(Context &ctx) {
	// default is to close connection after request
	return false;
}

void RequestProcessor::RenderProtocolStatus(std::ostream &os, Context &ctx) {
	StartTrace(RequestProcessor.RenderProtocolStatus);
	GetCurrentRequestProcessor(ctx)->DoRenderProtocolStatus(os, ctx);
}

Anything RequestProcessor::LogError(Context& ctx, long errcode, const String &reason, const String &line, const Anything &clientInfo,
		const String &msg, Anything &request, const char *who) {
	StartTrace(RequestProcessor.LogError);
	return GetCurrentRequestProcessor(ctx)->DoLogError(ctx, errcode, reason, line, clientInfo, msg, request, who);
}

void RequestProcessor::Error(std::ostream &reply, const String &msg, Context &ctx) {
	StartTrace(RequestProcessor.Error);
	GetCurrentRequestProcessor(ctx)->DoError(reply, msg, ctx);
}

void RequestProcessor::DoReadInput(std::iostream &Ios, Context &ctx) {
	Anything args;
	args.Import(Ios);
	StatTraceAny(RequestProcessor.DoReadInput, args, "request arguments", Storage::Current());
	ctx.PushRequest(args);
}

void RequestProcessor::DoProcessRequest(std::ostream &reply, Context &ctx) {
	StartTrace(RequestProcessor.DoProcessRequest);
	if (GetServer()) {
		GetServer()->ProcessRequest(reply, ctx);
	}
}

void RequestProcessor::DoWriteOutput(std::iostream &Ios, std::ostream &reply, Context &ctx) {
	StartTrace(RequestProcessor.DoWriteOutput);
	// dump the reply object onto the
	// socket stream
	//reply.PrintOn(Ios);
	// flush it in case not everything is
	// already dumped onto the socket
	Ios.flush();
}

void RequestProcessor::DoRenderProtocolStatus(std::ostream &os, Context &ctx) {
	// unknown protocol -> no status
}

Anything RequestProcessor::DoLogError(Context& ctx, long errcode, const String &reason, const String &line, const Anything &clientInfo,
		const String &msg, Anything &request, const char *who) {
	// unknown protocol -> no error logging
	return Anything();
}

void RequestProcessor::DoError(std::ostream &reply, const String &msg, Context &ctx) {
	// unknown protocol -> no error msg
}
