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
		if (ReadInput(*Ios, ctx) && !!(*Ios)) {

			KeepConnectionAlive(ctx); //!@FIXME: should not be a feature of a non-HTTP request

			if (socket->IsReadyForWriting()) {
				if (VerifyRequest(*Ios, ctx)) {
					// process this arguments and
					// write the output to the reply
					IntProcessRequest(*Ios, ctx);
					//!@FIXME BufferReply mechanism possible
				}
			}
		}
	} else {
		String logMsg("Cannot read from client socket");
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

bool RequestProcessor::DoVerifyRequest(Context &ctx) {
	StartTrace(RequestProcessor.DoVerifyRequest);
	return true;
}

void RequestProcessor::DoHandleVerifyError(std::ostream &reply, Context &ctx) {
	StartTrace(RequestProcessor.DoHandleVerifyError);
}

void RequestProcessor::DoHandleReadInputError(std::ostream &reply, Context &ctx) {
	StartTrace(RequestProcessor.DoHandleReadInputError);
}

void RequestProcessor::DoHandleProcessRequestError(std::ostream &reply, Context &ctx) {
	StartTrace(RequestProcessor.DoHandleProcessRequestError);
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
	GetCurrentRequestProcessor(ctx)->DoRenderProtocolStatus(os, ctx); //!@FIXME: remove as soon as static members are not required anymore
}

bool RequestProcessor::ReadInput(std::iostream &Ios, Context &ctx) {
	StartTrace(RequestProcessor.ReadInput);
	Anything anyValue = "ReadInput.Error";
	Context::PushPopEntry<Anything> aEntry(ctx, "RequestProcessorErrorSlot", anyValue, "RequestProcessorErrorSlot");
	if ( !DoReadInput(Ios, ctx) ) {
		DoHandleReadInputError(Ios, ctx);
		return false;
	}
	return true;
}

bool RequestProcessor::VerifyRequest(std::iostream &Ios, Context &ctx) {
	StartTrace(RequestProcessor.VerifyRequest);
	Anything anyValue = "VerifyRequest.Error";
	Context::PushPopEntry<Anything> aEntry(ctx, "RequestProcessorErrorSlot", anyValue, "RequestProcessorErrorSlot");
	if ( !DoVerifyRequest(ctx) ) {
		DoHandleVerifyError(Ios, ctx);
		return false;
	}
	return true;
}

//!process the arguments and generate a reply
bool RequestProcessor::IntProcessRequest(std::ostream &Ios, Context &ctx) {
	StartTrace(RequestProcessor.IntProcessRequest);
	Anything anyValue = "ProcessRequest.Error";
	Context::PushPopEntry<Anything> aEntry(ctx, "RequestProcessorErrorSlot", anyValue, "RequestProcessorErrorSlot");
	if ( !DoProcessRequest(Ios, ctx) ) {
		DoHandleProcessRequestError(Ios, ctx);
		return false;
	}
	return true;
}

bool RequestProcessor::DoReadInput(std::iostream &Ios, Context &ctx) {
	Anything args;
	args.Import(Ios);
	StatTraceAny(RequestProcessor.DoReadInput, args, "request arguments", Storage::Current());
	ctx.PushRequest(args);
	return true;
}

bool RequestProcessor::DoProcessRequest(std::ostream &reply, Context &ctx) {
	StartTrace(RequestProcessor.DoProcessRequest);
	if (GetServer()) {
		return GetServer()->ProcessRequest(reply, ctx);
	}
	return false;
}

void RequestProcessor::DoRenderProtocolStatus(std::ostream &os, Context &ctx) {
	// unknown protocol -> no status
}
