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
#include "TimeStamp.h"

//--- c-modules used -----------------------------------------------------------

//---- LDAPErrorHandler ----------------------------------------------------------------

LDAPErrorHandler::LDAPErrorHandler(Context &ctx, ParameterMapper *in, ResultMapper *out, String daName)
	: fCtx(ctx), fIn(in), fOut(out), fName(daName), fRetryState(eNoRetry)
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
		if ( fRetryState == eNoRetry ) {
			error["LdapMsg"] = "Connection does not exist (no handle available).";
		} else {
			error["LdapMsg"] = "Connection failure using LDAPPooledConnections.";
		}
	}

	// get connection parameters + append
	error["LdapConnectionParams"] = GetConnectionParams().DeepClone();

	String msgAsString = WriteSysLog(error, msg);
	error["MsgAsString"] = msgAsString;
	WriteError(error);
}

void LDAPErrorHandler::HandleUnbindError(LDAP *handle)
{
	StartTrace(LDAPErrorHandler.HandleUnbindError);

	int rc;
	String msg;
	msg << TimeStamp::Now().AsString();
	if ( handle ) {
		ldap_get_option(handle, LDAP_OPT_ERROR_NUMBER, &rc);
		String msg;
		msg << " LdapCode: [" << rc << "] LdapMsg: [" << ldap_err2string(rc) << "] ldap_unbind";
	} else {
		msg << " LdapCode: [no valid handle] LdapMsg: [no valid handle] ldap_unbind";
	}
	SysLog::Error(msg);
	Trace(msg);
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
	error["LdapQueryParams"] = GetQueryParams().DeepClone();

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
	SysLog::Error(TimeStamp::Now().AsString() << " " <<  msg << " " << fName << " " << sSysLog);
	String msgAsString;
	msgAsString << "LdapDataAccess: [" << fName << "]";
	for ( long l = 0; l < error.GetSize(); ++l ) {
		if ( error[l].GetType() == AnyArrayType ) {
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
	StartTrace(LDAPErrorHandler.WriteError);
	String key("LDAPError.");
	key << fName;
	Trace("WriteKey: " << key);
	TraceAny(error, "Error content:");
	fOut->Put(key, error, fCtx);
}

bool LDAPErrorHandler::GetError(Anything &error)
{
	StartTrace(LDAPErrorHandler.GetError);
	String key;
	key << "LDAPError." << fName;
	Trace("ReadKey: " << key);
	return fIn->Get(key, error, fCtx);
}

void LDAPErrorHandler::CleanUp()
{
	StartTrace(LDAPErrorHandler.CleanUp);
	Anything tmp = fCtx.GetTmpStore();
	tmp["LDAPError"].Remove(fName);
	if ( tmp["LDAPError"].GetSize() == 0L ) {
		tmp.Remove("LDAPError");
	}
	tmp["LDAPResult"].Remove(fName);
	if ( tmp["LDAPResult"].GetSize() == 0L ) {
		tmp.Remove("LDAPResult");
	}
}

ROAnything LDAPErrorHandler::GetQueryParams()
{
	return (ROAnything)fQueryParams;
}

void LDAPErrorHandler::PutQueryParams(Anything qp)
{
	fQueryParams = qp;
}

ROAnything LDAPErrorHandler::GetConnectionParams()
{
	return (ROAnything)fConnectionParams;
}

void LDAPErrorHandler::PutConnectionParams(Anything cp)
{
	fConnectionParams = cp;
}

void LDAPErrorHandler::SetRetryState(eRetryState retryState)
{
	StartTrace(LDAPErrorHandler.SetRetryState);
	Trace("Setting: [" << RetryStateAsString(retryState) << "]");
	fRetryState = retryState;
}

void LDAPErrorHandler::SetShouldRetry()
{
	StartTrace(LDAPErrorHandler.SetShouldRetry);
	String msg;
	if (!IsRetry()) {
		fRetryState = LDAPErrorHandler::eRetry;
		msg << "Setting retry state: [" << RetryStateAsString(fRetryState) << "]";
	} else {
		msg << "Retry already attempted. Not setting retry state: [" << RetryStateAsString(fRetryState) << "]";
	}
	SysLog::Info(msg);
}

LDAPErrorHandler::eRetryState LDAPErrorHandler::GetRetryState()
{
	StartTrace(LDAPErrorHandler.GetRetryState);
	Trace("Getting: [" << RetryStateAsString(fRetryState) << "]");
	return fRetryState;
}

bool LDAPErrorHandler::IsRetry()
{
	StartTrace(LDAPErrorHandler.IsRetry);
	bool ret =	(fRetryState == LDAPErrorHandler::eIsInRetrySequence);
	Trace("IsRetry: " << ret);
	return ret;
}

String LDAPErrorHandler::RetryStateAsString(eRetryState retryState)
{
	switch ( retryState) {
		case eRetry:
			return "eRetry";
			break;
		case eIsInRetrySequence:
			return "eIsInRetrySequence";
			break;
		case eNoRetry:
			return "eNoRetry";
			break;
		default:
			return "Unknown!";
			break;
	}
}
