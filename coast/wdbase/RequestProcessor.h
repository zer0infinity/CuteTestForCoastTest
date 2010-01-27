/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _RequestProcessor_H
#define _RequestProcessor_H

//---- baseclass include -------------------------------------------------
#include "config_wdbase.h"
#include "IFAConfObject.h"
#include "Context.h"

//--- RequestProcessor ----------------------------------------------------------
//! Policy object shared by all threads to handle a request message;
class EXPORTDECL_WDBASE RequestProcessor : public RegisterableObject
{
public:
	//!named object shared by all requests
	RequestProcessor(const char *processorName);
	virtual ~RequestProcessor()	{ }

	//! support for prototypes is required
	virtual IFAObject *Clone() const {
		return new RequestProcessor(fName);
	}

	//!configure request processor with server object
	virtual void Init(Server *server);

	//!general entry point called by handle request thread
	virtual void ProcessRequest(Context &ctx);

	//! checks if the connection should keep-alive after the request has been processed
	static bool KeepConnectionAlive(Context &ctx);

	//! force the server to close the connection after the request has been processed
	//! the caller must be sure, that a "Connection: close" is in the reply header
	static void ForceConnectionClose(Context &ctx);

	//! render the protocol specific status
	static void RenderProtocolStatus(ostream &os, Context &ctx);

	//! render the protocol specific error msg
	static void Error(ostream &reply, const String &msg, Context &ctx);

	//! registry api
	RegCacheDef(RequestProcessor);	// FindRequestProcessor()

protected:
	//!read the input arguments from the stream and generate an anything
	virtual void DoReadInput(iostream &ios, Context &ctx);

	//!process the arguments and generate a reply
	virtual void DoProcessRequest(ostream &reply, Context &ctx);

	//!process the arguments and generate a reply
	virtual void DoWriteOutput(iostream &ios, ostream &reply, Context &ctx);

	//! render the protocol specific status
	virtual void DoRenderProtocolStatus(ostream &os, Context &ctx);

	//! checks if the connection should keep-alive after the request has processed
	virtual bool DoKeepConnectionAlive(Context &ctx);

	//! render the protocol specific error msg
	virtual void DoError(ostream &reply, const String &msg, Context &ctx);

	//!the server we use as callback for application functionality
	Server *fServer;
	Anything fErrors;

	//!get the resulting anything if read request was successful
	Anything GetErrors() {
		return fErrors;
	};
	bool	 HasErrors() {
		return fErrors.GetSize() > 0 ? true : false;
	};

	friend class RequestProcessorTest;

private:
	//! get the current type of "RequestProcessor" from the context
	//! warning: the returned object is an unitialized clone of the
	//! prtototype that was registered.
	static RequestProcessor *GetCurrentRequestProcessor(Context &ctx);

	RequestProcessor(const RequestProcessor &);
	RequestProcessor &operator=(const RequestProcessor &);
};

#define RegisterRequestProcessor(name) RegisterObject(name, RequestProcessor)

#endif
