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
// #include "System.h"

//--- c-library modules used ---------------------------------------------------

//--- RequestProcessor ----------------------------------------------------------
RegCacheImpl(RequestProcessor);	// FindRequestProcessor()
RegisterRequestProcessor(RequestProcessor);

RequestProcessor::RequestProcessor(const char *processorName)
	: RegisterableObject(processorName)
	, fServer(0), fErrors()
{
	StartTrace(RequestProcessor.RequestProcessor);
}

void RequestProcessor::Init(Server *server)
{
	StartTrace(RequestProcessor.Init);
	fServer = server;
}

void RequestProcessor::ProcessRequest(Context &ctx)
{
	StartTrace(RequestProcessor.ProcessRequest);
	ctx.SetServer(fServer);
	Socket *socket = ctx.GetSocket();
	iostream *Ios = 0;

	if ( socket != (Socket *) NULL ) {
		ROAnything timeout;
		fServer->Lookup("SocketReadTimeout", timeout);
		socket->SetTimeout(timeout.AsLong(10 * 1000L));
		TraceAny(socket->ClientInfo(), "socket client info");
		Ios = socket->GetStream();
	}

	if ( Ios && socket->IsReadyForReading() ) {
		// disable nagle algorithm
		socket->SetNoDelay();
		DoReadInput(*Ios, ctx);

		KeepConnectionAlive(ctx);

		if ( !(*Ios) ) {
			return;
		}

		if (socket->IsReadyForWriting()) {
			// process this arguments and
			// write the output to the reply
			DoProcessRequest(*Ios, ctx);
			//FIXME: BufferReply mechanism possible
		}
	} else {
		String logMsg("No valid stream from socket: ");
		logMsg << ((socket) ? socket->GetFd() : -1L);
		if ( socket ) {
			logMsg << " (" << socket->ClientInfo()["REMOTE_ADDR"].AsString() << ':' << socket->ClientInfo()["REMOTE_PORT"].AsString() << ')';
		}
		SystemLog::Warning(logMsg);
	}
}

RequestProcessor *RequestProcessor::GetCurrentRequestProcessor(Context &ctx)
{
	String requestProcessor(ctx.Lookup("RequestProcessor", "RequestProcessor"));
	return RequestProcessor::FindRequestProcessor(requestProcessor);
}

void RequestProcessor::ForceConnectionClose(Context &ctx)
{
	ctx.GetTmpStore()["Keep-Alive"] = 0L;
}

bool RequestProcessor::KeepConnectionAlive(Context &ctx)
{
	bool retVal = false;
	StatTrace(RequestProcessor.KeepConnectionAlive, "PersistentConnections:" << (ctx.Lookup("PersistentConnections").AsBool(false) ? "true" : "false"), Storage::Current());
	if ( ctx.Lookup("PersistentConnections").AsBool(false) ) {
		// first check if we already know the result
		ROAnything lookupAny;
		if (ctx.Lookup("Keep-Alive", lookupAny) && !lookupAny.IsNull() ) {
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

bool RequestProcessor::DoKeepConnectionAlive(Context &ctx)
{
	// default is to close connection after request
	return false;
}

void RequestProcessor::RenderProtocolStatus(ostream &os, Context &ctx)
{
	StartTrace(RequestProcessor.RenderProtocolStatus);
	GetCurrentRequestProcessor(ctx)->DoRenderProtocolStatus(os, ctx);
}

void RequestProcessor::Error(ostream &reply, const String &msg, Context &ctx)
{
	StartTrace(RequestProcessor.Error);
	GetCurrentRequestProcessor(ctx)->DoError(reply, msg, ctx);
}

void RequestProcessor::DoReadInput(iostream &Ios, Context &ctx)
{
	// default implementation just
	// import the anything from the
	// socket stream

	Anything args;
	args.Import(Ios);
	StatTraceAny(RequestProcessor.DoReadInput, args, "request arguments", Storage::Current());
	ctx.PushRequest(args);
}

void RequestProcessor::DoProcessRequest(ostream &reply, Context &ctx)
{
	StartTrace(RequestProcessor.DoProcessRequest);
	fServer->ProcessRequest(reply, ctx);
}

void RequestProcessor::DoWriteOutput(iostream &Ios, ostream &reply, Context &ctx)
{
	StartTrace(RequestProcessor.DoWriteOutput);
	// dump the reply object onto the
	// socket stream
	//reply.PrintOn(Ios);
	// flush it in case not everything is
	// already dumped onto the socket
	Ios.flush();
}

void RequestProcessor::DoRenderProtocolStatus(ostream &os, Context &ctx)
{
	// unknown protocol -> no status
}

void RequestProcessor::DoError(ostream &reply, const String &msg, Context &ctx)
{
	// unknown protocol -> no error msg
}
