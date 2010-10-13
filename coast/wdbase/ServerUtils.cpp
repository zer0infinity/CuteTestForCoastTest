/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ServerUtils.h"

//--- standard modules used ----------------------------------------------------
#include "Socket.h"
#include "SystemLog.h"
#include "StringStream.h"
#include "Server.h"
#include "RequestProcessor.h"
#include "Timers.h"
#include "SystemBase.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------
#if defined(WIN32)
#include <signal.h>
#endif

//---- InterruptHandler ------------------------------------------------------------------
#if defined(WIN32)

// implementation for the WIN32 interrupthandler
// which is quite different from the solaris one

Server *WIN32InterruptHandler::fgServer;

BOOL ConsoleHandler(DWORD dwCtrlType)
{
	StartTrace(WIN32InterruptHandler.ConsoleHandler);
	switch (dwCtrlType) {
			// Handle the CTRL+C signal.
		case CTRL_C_EVENT:
			Trace("CTRL_C_EVENT");
			break;
			// CTRL+CLOSE: Use return of TRUE here to confirm that the user wants to exit.
			// occurs when console gets closed
		case CTRL_CLOSE_EVENT:
			Trace("CTRL_CLOSE_EVENT");
			break;
		case CTRL_BREAK_EVENT:
			Trace("CTRL_BREAK_EVENT");
			break;
		case CTRL_LOGOFF_EVENT:
			Trace("CTRL_LOGOFF_EVENT");
			break;
		case CTRL_SHUTDOWN_EVENT:
			Trace("CTRL_SHUTDOWN_EVENT");
			break;
		default:
			Trace("default case should not happen");
	}
	WIN32InterruptHandler::ShutDown();
	// Use false to pass signals to the next handler.
	return TRUE;
}

void SignalHandler(int sig)
{
	StartTrace1(WIN32InterruptHandler.SignalHandler, "signal:" << (long)sig);
	switch (sig) {
			/* interrupt */
		case SIGINT:
			Trace("SIGINT");
			break;
			/* illegal instruction - invalid function image */
		case SIGILL:
			Trace("SIGILL");
			break;
			/* floating point exception */
		case SIGFPE:
			Trace("SIGFPE");
			break;
			/* segment violation */
		case SIGSEGV:
			Trace("SIGSEGV");
			break;
			/* Software termination signal from kill */
		case SIGTERM:
			Trace("SIGTERM");
			break;
			/* Ctrl-Break sequence */
		case SIGBREAK:
			Trace("SIGBREAK");
			break;
			/* abnormal termination triggered by abort call */
		case SIGABRT:
			Trace("SIGABRT");
			break;
		default:
			Trace("unknown signal:" << (long)sig << " received");
	}
	WIN32InterruptHandler::ShutDown();
}

WIN32InterruptHandler::WIN32InterruptHandler(Server *server) : fServer(server)
{
	StartTrace(WIN32InterruptHandler.Ctor);
	// set a console handler callback function that shuts down the server in case a key press occurs in the console
	SetConsoleCtrlHandler((PHANDLER_ROUTINE) ConsoleHandler, true);
	// setup signal handler for other signal which could occur
	// ... and occur in case we use cygwins kill utility
	signal(SIGINT, SignalHandler);
	signal(SIGILL, SignalHandler);
	signal(SIGFPE, SignalHandler);
	signal(SIGSEGV, SignalHandler);
	signal(SIGTERM, SignalHandler);
	signal(SIGBREAK, SignalHandler);
	signal(SIGABRT, SignalHandler);

	fgServer = fServer;
	if ( fServer ) {
		fServer->WritePIDFile();
	}
}

void WIN32InterruptHandler::ShutDown()
{
	StartTrace(WIN32InterruptHandler.ShutDown);
	if (fgServer) {
		SystemLog::Info("caught SIGINT");
		fgServer->PrepareShutdown();
		fgServer->RemovePIDFile();
		fgServer = NULL;
		// finally remove the console handler
		SetConsoleCtrlHandler((PHANDLER_ROUTINE) ConsoleHandler, false);
	}
}
#else

// the solaris implementation of the InterruptHandler
// is based on the signal handling API's of the
// solaris thread library

InterruptHandler::InterruptHandler(Server *server)
	: Thread("InterruptHandler", true)
	, fServer(server)
{
	StartTrace(InterruptHandler.Ctor);
	Start();
}

InterruptHandler::~InterruptHandler()
{
	StartTrace(InterruptHandler.Dtor);
	if ( GetId() != 0 ) {
		THRKILL(GetId(), SIGTERM);
	}
	Terminate();
	CheckState(Thread::eTerminated);
	if (fServer) {
		fServer->RemovePIDFile();
	}
}

void InterruptHandler::Run()
{
	StartTrace(InterruptHandler.Run);

	// the just wait for the signal to happen
	Trace("Thread: " << Thread::MyId());
	Trace("Pid: " << (long)Coast::System::getpid());
	Trace("Server [" << (fServer ? (const char *)(fServer->fName) : "no-name") << "]");

	if ( fServer ) {
		fServer->WritePIDFile(Coast::System::getpid());
	}

	sigset_t set;
	int isignal;
	// setup the signal set that catches control-c
	sigemptyset(&set);
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGTERM);
	sigaddset(&set, SIGHUP);

handlesignal:
	SetWorking();
	Trace("calling sigwait");
	sigwait(&set, &isignal);
	SetReady();

	Trace("got signal: " << (long)isignal);
	if (fServer) {
		String sigName;
		sigName << (long)isignal;
		if ( isignal == SIGHUP ) {
			SYSINFO("caught SIGHUP, resetting server");
			int retVal = fServer->GlobalReinit();
			if ( retVal == 0 ) {
				// everything ok, wait on next signal
				goto handlesignal;
			}
			sigName = "SIGHUP";
			// shutdown if any mishap happens
			SYSERROR("GlobalReinit failed, shutting down...");
		} else if ( isignal == SIGINT ) {
			sigName = "SIGINT";
		} else if ( isignal == SIGTERM ) {
			sigName = "SIGTERM";
		} else {
			SYSINFO("caught unknown signal " << sigName << " ...ignoring!");
			goto handlesignal;
		}
		SYSINFO("caught " << sigName << " shutting down server");
		// prepare the server for shutdown
		// by causing him to leave the accept-loop
		fServer->PrepareShutdown();
	}
	// the job is done, just die...
}
#endif

//--- HandleRequest implementation -------------------------------
HandleRequest::HandleRequest(const char *name)
	: WorkerThread(name)
	, fClientSocket(0)
	, fProcessor(0)
	, fRequestNumber(0)
{
}

void HandleRequest::DoInit(ROAnything workerInit)
{
	StartTrace(HandleRequest.DoInit);
	// request independant setup
	fProcessor = SafeCast(workerInit["processor"].AsIFAObject(0), RequestProcessor);
	fClientSocket = 0;
	fRequestNumber = 0;
}

void HandleRequest::DoWorkingHook(ROAnything workloadArgs)
{
	StartTrace(HandleRequest.DoWorkingHook);
	// request specific setup
	fClientSocket = (Socket *)workloadArgs["socket"].AsIFAObject(0);
	fRequestNumber = workloadArgs["request"].AsLong(-1);
	TraceAny(workloadArgs, "my working arguments for request " << fRequestNumber);
}

void HandleRequest::DoProcessWorkload()
{
	StartTrace(HandleRequest.DoProcessWorkload);
	if (fClientSocket) {
		{
			// **the** one and only context for this request
			Context ctx(fClientSocket);
			{
				// set the request timer to time
				// the duration of this request
				RequestTimer(Cycle, "Nr: " << fRequestNumber, ctx);
				if (fProcessor) {
					fProcessor->ProcessRequest(ctx);
				}
			}
			// log all time related items
			RequestTimeLogger(ctx);
		}
		Trace("deleting socket");
		delete fClientSocket;
		fClientSocket = 0;
		fRequestNumber = 0;
	}
}

void HandleRequest::DoTerminationRequestHook(ROAnything workloadArgs)
{
	StartTrace(HandleRequest.DoTerminationRequestHook);
	// terminate running request by setting bad bit on stream
	if (fClientSocket) {
		Trace("client socket still active");
		std::iostream *Ios = fClientSocket->GetStream();
		Trace("");
		if (Ios && !!(*Ios)) {
			Trace("setting badbit on stream to terminate me");
			Ios->clear(std::ios::badbit | Ios->rdstate());
		}
	}
}

//---- RequestThreadsManager ------------------------------------------------
RequestThreadsManager::RequestThreadsManager(String name)
	: WorkerPoolManager(name)
	, fRequests(0)
	, fProcessor(0)
{
}

RequestThreadsManager::~RequestThreadsManager()
{
	StartTrace(RequestThreadsManager.~RequestThreadsManager);
	bool reallyterminated = Terminate();
	Assert(reallyterminated);
	if (!reallyterminated) {
		SYSERROR("failed to terminate all request threads");
	}
	Anything dummy;
	DoDeletePool(dummy);

	if (fProcessor) {
		delete fProcessor;
		fProcessor = 0;
	}
}

void RequestThreadsManager::DoAllocPool(ROAnything args)
{
	StartTrace(RequestThreadsManager.DoAllocPool);
	TraceAny(args, "args");
	RequestProcessor *processor = SafeCast(args["processor"].AsIFAObject(0), RequestProcessor);

	if (processor) {
		fProcessor = processor;
	} else {
		const char *msg = "OOPS, couldn't find a RequestProcessor in args";
		Trace(msg);
		SYSERROR(msg);
	}
	fRequests = new HandleRequest[GetPoolSize()];
}

WorkerThread *RequestThreadsManager::DoGetWorker(long i)
{
	StartTrace(RequestThreadsManager.DoGetWorker);
	// concrete subclass needs to determine the object ptr
	if (fRequests) {
		return &(fRequests[i]);
	}
	return 0;
}

void RequestThreadsManager::DoDeletePool(ROAnything args)
{
	StartTrace(RequestThreadsManager.DoDeletePool);
	if (fRequests) {
		// SOP, should check if threads are really done.
		//for (int i = 0; i < GetPoolSize(); i ++){
		//	Assert(fRequests[i].Terminate(1));
		//}
		delete [] fRequests;	// cast to correct type
		fRequests = 0;			// reset so that base class will not try to delete it again
	}
	if (fProcessor) {
		delete fProcessor;
		fProcessor = 0;
	}
}

bool RequestThreadsManager::CanReInitPool()
{
	return fProcessor ? true : false;
}
