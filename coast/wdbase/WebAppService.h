/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _WebAppService_H
#define _WebAppService_H

#include "config_wdbase.h"

//---- baseclass include -------------------------------------------------
#include "ServiceHandler.h"

//---- WebAppService ----------------------------------------------------------
//!single line description of the class
//! further explanation of the purpose of the class
//! this may contain <B>HTML-Tags</B>
//! ...
class EXPORTDECL_WDBASE WebAppService : public ServiceHandler
{
	friend class WebAppServiceTest;
public:
	//!standard named object constructor
	WebAppService(const char *serviceHandlerName);
	virtual ~WebAppService();

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) WebAppService(fName);
	}

protected:
	//!handles requested service
	virtual void DoHandleService(ostream &os, Context &ctx);

	//!prepares the request before decoding into query and verifying
	virtual void PrepareRequest(Context &ctx);

	//!verifies the request with regard to some basic attributes
	virtual bool VerifyRequest(ostream &reply, Context &ctx);

	//!build a query from a path expression and a query string
	virtual Anything BuildQuery(const String &pathString, const String &queryString);

	//!decode a query built by ourselve (normally frontdoor does not decode queries)
	virtual void DecodeWDQuery(Anything &query, const Anything &request);

	//! split query into path and query arguments
	virtual void SplitURI2PathAndQuery(Anything &request);

	//!add some items to a query
	void Add2Query(Anything &query, const Anything &queryItems, bool overWrite = false);

	void ExtractPostBodyFields(Anything &query, const Anything &requestBody);

};

#endif
