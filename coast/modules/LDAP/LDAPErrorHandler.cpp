/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "LDAPErrorHandler.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "SysLog.h"
#include "StringStream.h"

//--- c-modules used -----------------------------------------------------------

//---- LDAPErrorHandler ----------------------------------------------------------------

LDAPErrorHandler::LDAPErrorHandler(Context &ctx, ParameterMapper *in, ResultMapper *out, String daName)
	: fCtx(ctx), fIn(in), fOut(out), fName(daName)
{}

LDAPErrorHandler::~LDAPErrorHandler()
{
	fCtx.GetTmpStore().Remove("LDAPConnectionParams");
	fCtx.GetTmpStore().Remove("LDAPQueryParams");
}

void LDAPErrorHandler::HandleSessionError(LDAP *handle, String msg)
{
	StartTrace(LDAPErrorHandler.HandleSessionError);

	Anything error;
	error["Msg"] = msg;
	if (handle) {
		int rc;
		ldap_get_option(handle, LDAP_OPT_ERROR_NUMBER, &rc);
		error["LdapCode"] = rc;
		error["LdapMsg"] = ldap_err2string(rc);
	} else {
		error["LdapMsg"] = "Connection does not exist (no handle available).";
	}

	// get connection parameters + append
	error["LdapConnectionParams"] = GetConnectionParams();

	String msgAsString = WriteSysLog(error, msg);
	error["MsgAsString"] = msgAsString;
	WriteError(error);
}

void LDAPErrorHandler::HandleError(String msg, Anything args, String argDescr)
{
	StartTrace(LDAPErrorHandler.HandleError);

	Anything error;
	error["Msg"] = msg;
	if ( !args.IsNull() ) {
		error[argDescr] = args;
	}

	// get query parameters + append
	error["LdapQueryParams"] = GetQueryParams();

	String msgAsString = WriteSysLog(error, msg);
	error["MsgAsString"] = msgAsString;
	WriteError(error);
}

String LDAPErrorHandler::WriteSysLog(Anything error, String &msg)
{
	Anything anyOldPWD;
	if ( error.LookupPath(anyOldPWD, "LdapConnectionParams.BindPW") ) {
		error["LdapConnectionParams"]["BindPW"] = "WipedOut";
	}
	String sSysLog;
	OStringStream ossSysLog(&sSysLog);
	error.PrintOn(ossSysLog, false);
	ossSysLog.flush();
	// log all ldap session errors in SysLog
	SysLog::Error(msg << " " << fName << " " << ossSysLog);
	String msgAsString;
	msgAsString << "LdapDataAccess: [" << fName << "]";
	for ( long l = 0; l < error.GetSize(); l++ ) {
		if ( error[l].GetType() == Anything::eArray ) {
			String sMsgDetails;
			OStringStream ossMsgDetails(&sMsgDetails);
			error[l].PrintOn(ossMsgDetails, false);
			ossMsgDetails.flush();
			msgAsString << " " << error.SlotName(l) << " [" <<	sMsgDetails << "]";
		} else {
			msgAsString << " " << error.SlotName(l) << " [" << error[l].AsString() << "]";
		}
	}
	// Restore password
	if ( !anyOldPWD.IsNull() ) {
		error["LdapConnectionParams"]["BindPW"] = anyOldPWD.AsCharPtr();
	}
	return msgAsString;
}

void LDAPErrorHandler::WriteError(Anything &error)
{
	String key("LDAPError.");
	key << fName;
	fOut->Put(key, error, fCtx);
}

bool LDAPErrorHandler::GetError(Anything &error)
{
	StartTrace(LDAPErrorHandler.GetError);
	String key;
	key << "LDAPError." << fName;
	return fIn->Get(key, error, fCtx);
}

Anything LDAPErrorHandler::GetQueryParams()
{
	return fQueryParams.IsNull() ? "None available, sorry." : fQueryParams;
}

void LDAPErrorHandler::PutQueryParams(Anything qp)
{
	fQueryParams = qp;
}

Anything LDAPErrorHandler::GetConnectionParams()
{
	return fConnectionParams.IsNull() ? "None available, sorry." : fConnectionParams;
}

void LDAPErrorHandler::PutConnectionParams(Anything cp)
{
	fConnectionParams = cp;
}
