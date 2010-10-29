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

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) RequestProcessor(fName);
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
	static void RenderProtocolStatus(std::ostream &os, Context &ctx);

	//! Log the error to Security.log
	static Anything LogError(Context& ctx, long errcode, const String &reason, const String &line, const String &msg, const char *who);

	//! render the protocol specific error msg
	static void Error(std::ostream &reply, const String &msg, Context &ctx);

	Server* GetServer() {
		return fServer;
	}
	//! registry api
	RegCacheDef(RequestProcessor);	// FindRequestProcessor()

protected:
	//!read the input arguments from the stream and generate an anything
	virtual bool DoReadInput(std::iostream &Ios, Context &ctx);

	virtual bool DoVerifyRequest(Context &ctx);

	//!process the arguments and generate a reply
	virtual bool DoProcessRequest(std::ostream &reply, Context &ctx);

	//! render the protocol specific status
	virtual void DoRenderProtocolStatus(std::ostream &os, Context &ctx);

	virtual void DoHandleVerifyError(std::ostream &reply, Context &ctx);

	virtual void DoHandleReadInputError(std::ostream &reply, Context &ctx);

	virtual void DoHandleProcessRequestError(std::ostream &reply, Context &ctx);

	//! checks if the connection should keep-alive after the request has processed
	virtual bool DoKeepConnectionAlive(Context &ctx);

	//! Log the error to Security.log
	virtual Anything DoLogError(Context& ctx, long errcode, const String &reason, const String &line, const String &msg, const char *who);

private:
	bool ReadInput(std::iostream &Ios, Context &ctx);

	//! render the protocol specific error msg
	virtual void DoError(std::ostream &reply, const String &msg, Context &ctx);
	bool VerifyRequest(std::iostream &Ios, Context &ctx);

	//!process the arguments and generate a reply
	bool IntProcessRequest(std::ostream &Ios, Context &ctx);

	//!the server we use as callback for application functionality
	Server *fServer;

	RequestProcessor(const RequestProcessor &);
	RequestProcessor &operator=(const RequestProcessor &);
};

#define RegisterRequestProcessor(name) RegisterObject(name, RequestProcessor)

#endif
