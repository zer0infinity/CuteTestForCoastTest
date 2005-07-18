/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _LDAPErrorHandler_H
#define _LDAPErrorHandler_H

//---- baseclass include -------------------------------------------------
#include "config_LDAPDA.h"
#include "LDAPAPI.h"
#include "Mapper.h"

//---- LDAPErrorHandler ----------------------------------------------------------
//! <B>Handles LDAP Errors</B>
class EXPORTDECL_LDAPDA LDAPErrorHandler
{
public:
	//--- constructors
	LDAPErrorHandler(Context &ctx, ParameterMapper *getter, ResultMapper *putter, String daName);
	virtual ~LDAPErrorHandler();

	// handle session error with message
	virtual void HandleSessionError(LDAP *ldap, String msg);

	// handle other error (general implementation)
	virtual void HandleError(String msg, Anything args = Anything(), String argDescr = "AdditionalInfo");

	// get last error description of the assiciated DAImpl
	// (returns false, if none was found)
	virtual bool GetError(Anything &error);

	virtual Anything GetQueryParams();
	virtual void PutQueryParams(Anything qp);
	virtual Anything GetConnectionParams();
	virtual void PutConnectionParams(Anything cp);

	//! Set ShouldRetry flag This flag is used when LDAPPooledConnections are used
	virtual void SetShouldRetry(bool shouldRetry) {
		fShouldRetry = shouldRetry;
	};
	//! Query ShouldRetry flag
	virtual bool GetShouldRetry() {
		return fShouldRetry;
	};

	//! Clean up LDAPError and LDAPResultSlot in Context's tmp store
	virtual void LDAPErrorHandler::CleanUp();

protected:
	Context &fCtx;
	ParameterMapper *fIn;
	ResultMapper *fOut;
	String fName;
	Anything fQueryParams, fConnectionParams;
	bool fShouldRetry;

private:
	void WriteError(Anything &error);
	String WriteSysLog(Anything error, String &msg);
};

#endif
