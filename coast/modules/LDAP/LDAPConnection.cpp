/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "LDAPConnection.h"

//--- project modules used -----------------------------------------------------
#include "LDAPMessageEntry.h"
#include "ldap-extension.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

LDAPConnection::LDAPConnection(ROAnything connectionParams)
{
	fServer = connectionParams["Server"].AsString("localhost");
	fPort = connectionParams["Port"].AsLong(389L);
	fSearchTimeout  = (int) connectionParams["Timeout"].AsLong(60L);
	// A reasonable connection timeout default
	fConnectionTimeout = connectionParams["ConnectionTimeout"].AsLong(10);
	fConnectionTimeout *= 1000;
	fMapUTF8 = connectionParams["MapUTF8"].AsBool(true);
}

LDAPConnection::~LDAPConnection()
{
	Disconnect();
}

LDAP *LDAPConnection::GetConnection()
{
	StartTrace(LDAPConnection.GetConnection);
	return fHandle;
}

int LDAPConnection::GetSearchTimeout()
{
	StartTrace(LDAPConnection.GetSearchTimeout);
	return fSearchTimeout;
}

int LDAPConnection::GetConnectionTimeout()
{
	StartTrace(LDAPConnection.GetConnectionTimeout);
	return fConnectionTimeout;
}

bool LDAPConnection::Connect(ROAnything bindParams, LDAPErrorHandler eh)
{
	StartTrace(LDAPConnection.Connect);

	// set handle to null
	fHandle = NULL;
	String errMsg;

	// get connection handle
	if ( !(fHandle = Init(eh)) ) {
		return false;
	}

	// set protocol + timeout
	if ( !SetProtocol(eh) || !SetConnectionTimeout(eh) || !SetSearchTimeout(eh) ) {
		return false;
	}

	// send bind request (asynchronous)
	String bindName = bindParams["BindName"].AsString("");
	String bindPW = bindParams["BindPW"].AsString("");
	int msgId;
	if ( !Bind(bindName, bindPW, msgId, eh) ) {
		return false;
	}

	// wait for bind result (using msgId)
	Anything result;
	return WaitForResult(msgId, result, eh);
}

LDAP *LDAPConnection::Init(LDAPErrorHandler eh)
{
	StartTrace(LDAPConnection.Init);

	LDAP *ldap = ::ldap_init( fServer, fPort );
	if ( !ldap ) {
		Trace("ldap_init FAILED");
		String errMsg = "ldap_init(";
		errMsg << fServer << "," << fPort << ") failed";
		eh.HandleSessionError(fHandle, errMsg);
	}
	return ldap;
}

bool LDAPConnection::SetProtocol(LDAPErrorHandler eh)
{
	StartTrace(LDAPConnection.SetProtocol);

	int ldapVersion = LDAP_VERSION_MAX;
	if ( ::ldap_set_option( fHandle, LDAP_OPT_PROTOCOL_VERSION, &ldapVersion ) != LDAP_SUCCESS ) {
		Trace("ldap_set_option: LDAP_OPT_PROTOCOL_VERSION   FAILED");
		eh.HandleSessionError(fHandle, "Could not set protocol vesion.");
		return false;
	}
	return true;
}

bool LDAPConnection::SetSearchTimeout(LDAPErrorHandler eh)
{
	StartTrace(LDAPConnection.SetSearchTimeout);

	// This will be overridden in DoLDAPRequest - valid only for searches
	if ( ::ldap_set_option( fHandle, LDAP_OPT_TIMELIMIT, &fSearchTimeout ) != LDAP_SUCCESS ) {
		Trace("ldap_set_option: LDAP_OPT_TIMELIMIT   FAILED");
		eh.HandleSessionError(fHandle, "Could not set timelimit for searches.");
		return false;
	}
	return true;
}

bool LDAPConnection::SetConnectionTimeout(LDAPErrorHandler eh)
{
	StartTrace(LDAPConnection.SetConnectionTimeout);

	if ( ::ldap_set_option( fHandle, LDAP_X_OPT_CONNECT_TIMEOUT, &fConnectionTimeout ) != LDAP_SUCCESS ) {
		Trace("==================================== ::ldap_set_option: LDAP_X_OPT_CONNECT_TIMEOUT   FAILED");
		eh.HandleSessionError(fHandle, "ldap_set_option  [LDAP_X_OPT_CONNECT_TIMEOUT]");
		return false;
	}
	return true;
}

bool LDAPConnection::Bind(String bindName, String bindPW, int &msgId, LDAPErrorHandler eh)
{
	StartTrace(LDAPConnection.Bind);

	String errMsg = "Binding request failed. ";
	errMsg << "[Server: '" << fServer << "', Port: '" << fPort << "'] ";

	if ( bindName.IsEqual("") || bindName.IsEqual("Anonymous") ) {
		Trace("Binding as Anonymous");
		errMsg << " (Anonymous bind.)";
		msgId = ::ldap_simple_bind( fHandle, NULL, NULL );
	} else {
		errMsg << " (Authorized bind.)";

		if ( bindPW == "undefined" ) {
			Trace("Bindpassword NOT OK: pwd = ["  << bindPW << "]");
			eh.HandleSessionError(fHandle, errMsg);
			return false;
		} else {
			Trace("Binding with <" << bindName << "><" << bindPW << ">");
			msgId = ::ldap_simple_bind( fHandle, bindName, bindPW );
		}
	}

	// bind request successful?
	if ( msgId == -1 ) {
		Trace("Binding request FAILED!");
		eh.HandleSessionError(fHandle, errMsg);
		return false;
	}

	Trace("Binding request SUCCEEDED, waiting for connection...");
	return true;
}

bool LDAPConnection::WaitForResult(int msgId, Anything &result, LDAPErrorHandler eh)
{
	StartTrace(LDAPConnection.WaitForResult);

	timeval tv;
	tv.tv_sec  = fSearchTimeout;
	tv.tv_usec = 0;

	timeval *tvp = (fSearchTimeout == 0) ? NULL : &tv;

	bool finished = false;
	bool success = false;

	String errMsg;
	int resultCode;

	LDAPMessage *ldapResult;
	LDAPMessageEntry lmAutoDestruct(&ldapResult);	// automatic destructor for LDAPMessage
	lmAutoDestruct.Use();

	while (!finished) {
		// wait for result
		resultCode = ldap_result(fHandle, msgId, 1, tvp, &ldapResult);

		// check result
		if (resultCode == -1) {
			// error!
			Trace("WaitForResult received an error - trying again...");

			int opRet;
			ldap_parse_result( fHandle, ldapResult, &opRet, NULL, NULL, NULL, NULL, 0 );
			Trace("ErrorCode: " << (long)opRet << ", ErrorMsg: " << ldap_err2string( opRet ));

			// kgu: this is a legacy decision - should we not rather
			// finish here with an appropriate error msg instead
			// of having a loop and trying it over and over?
		} else if (resultCode == 0) {
			// timeout, abandon
			int errCode = ldap_abandon(fHandle, msgId);
			errMsg << "The request <" << (long) msgId << "> timed out. Abandoning request now...";
			errMsg << (errCode == LDAP_SUCCESS ? "Request successfully abandoned." : "Request abandoning FAILED!");

			eh.HandleSessionError(fHandle, errMsg);
			finished = true;
		} else {
			// received a result
			int errCode = ldap_result2error(fHandle, ldapResult, 0);
			if (errCode == LDAP_SUCCESS || errCode == LDAP_SIZELIMIT_EXCEEDED) {
				success = true;

				// transform LDAPResult into an Anything with Meta Information
				TransformResult(ldapResult, result, eh.GetQueryParams());

				// add extra flag to inform client, if sizelimit was exceeded
				if (errCode == LDAP_SIZELIMIT_EXCEEDED) {
					result["SizeLimitExceeded"] = 1;
				}
			} else if (errCode == LDAP_COMPARE_FALSE || errCode == LDAP_COMPARE_TRUE) {
				success = true;

				// this is a bit special
				int rc;
				ldap_get_option(fHandle, LDAP_OPT_ERROR_NUMBER, &rc);
				result["Type"] = "LDAP_RES_COMPARE";
				result["Equal"] = (errCode == LDAP_COMPARE_TRUE);
				result["LdapCode"] = rc;
				result["LdapMsg"] = ldap_err2string(rc);
			} else {
				errMsg = "LDAP request failed.";
				eh.HandleSessionError(fHandle, errMsg);
			}
			finished = true;
		}
	}

	return success;
}

void LDAPConnection::TransformResult(LDAPMessage *ldapResult, Anything &result, Anything query)
{
	StartTrace(LDAPConnection.TransformResult);
	Trace("Do UTF8 to HTML mapping: " << fMapUTF8);

	String type = GetTypeStr(ldap_msgtype(ldapResult));
	Trace("Type of result: " << type);

	if (type.Length() > 0) {
		// meta information about the result
		result["Type"] = type;
		result["Query"] = query;
		int nofResults = ldap_count_messages(fHandle, ldapResult);
		result["NumberOfResultMessages"] = nofResults;
		int nofEntries = ldap_count_entries(fHandle, ldapResult);
		result["NumberOfEntries"] = nofEntries;
		result["SizeLimitExceeded"] = 0;

		Trace("#Messages: " << (long)nofResults << ", #Entries: " << (long)nofEntries);

		// extract data
		LDAPMessage *entry = ldap_first_entry(fHandle, ldapResult);
		String attr;
		String dn;
		String valStr;
		struct berval **vals;
		BerElement *ber;
		int count = 0;
		int nofVals;
		Anything entries;

		// step through all entries
		while (entry) {
			dn = ldap_get_dn(fHandle, entry);
			attr = ldap_first_attribute(fHandle, entry, &ber);

			// ---------------------------------------------------
			// kgu: might want to add dn ass attribute to each
			// entry, some clients of old system might rely on it
			// ---------------------------------------------------
			// entries[dn]["dn"] = dn;

			// step through all attributes
			while (attr.Length() > 0) {
				// normalize all attributes to lowercase
				attr.ToLower();

				vals = ldap_get_values_len(fHandle, entry, attr);
				nofVals = ldap_count_values_len(vals);

				if ( nofVals == 0 ) {
					// there might be attributes without values
					// (eg. if AttrsOnly was requested)
					entries[dn].Append(attr);
				} else {
					for (int i = 0; i < nofVals; i++) {
						valStr = String((void *)vals[i]->bv_val, (long)vals[i]->bv_len);

						// Map UTF8-chars?
						MapUTF8Chars(valStr);

						if (i == 0) {
							entries[dn][attr] = valStr;
						} else {
							entries[dn][attr].Append(valStr);
						}
					}
				}

				attr = ldap_next_attribute(fHandle, entry, ber);

				// free used memory
				ber_bvecfree(vals);
			}

			entry = ldap_next_entry(fHandle, entry);
			count++;

			// free memory used
			if (ber) {
				ber_free(ber, 0);
			}
		}

		result["Entries"] = entries;
	}
}

bool LDAPConnection::Disconnect()
{
	StartTrace(LDAPConnection.Disconnect);

	Trace("Disconnecting LDAP...");
	if (fHandle) {
		int errCode = ldap_unbind(fHandle);
		if (errCode != LDAP_SUCCESS) {
			Trace("Disconnect (unbind) failed!");
			return false;
		}
	}
	Trace("Disconnecting successful.");
	return true;
}

void LDAPConnection::MapUTF8Chars(String &str)
{
	StartTrace(LDAPConnection.MapUTF8Chars);
	// kgu: the following is legacy code from the old system

	String result;
	for (long i = 0; i < str.Length(); ) {
		const char *theChar = ((const char *)str) + i;
		const char *theStartChar = theChar;
		unsigned long utfChar = ldap_utf8getcc(&theChar);
		i += (theChar - theStartChar); // one utf8 symbol might be several characters

		if ( fMapUTF8 ) {
			switch (utfChar) {
				case 'Æ':
				case 252:
					result.Append("&uuml;");
					break;
				case 'ä':
				case 228:
					result.Append("&auml;");
					break;
				case '•':
				case 246:
					result.Append("&ouml;");
					break;
				case '¢':
				case 179:
					result.Append("&Uuml;");
					break;
				case '²':
				case 196:
					result.Append("&Auml;");
					break;
				case 'ã':
				case 214:
					result.Append("&Ouml;");
					break;
				case 'è':
					result.Append("&ecirc;");
					break;
				case 'é':
					result.Append("&acirc;");
					break;
				case 'Ç':
					result.Append("&agrave;");
					break;
				case 'ê':
					result.Append("&egrave;");
					break;
				case '«':
				case 242 :
					result.Append("&Egrave;");
					break;
				case 'œ':
				case 250 :
					result.Append("&Euml;");
					break;
				case 'ë':
					result.Append("&eacute;");
					break;
				case 'Ï':
				case 167 :
					result.Append("&Eacute;");
					break;
				case 'ç':
				case 231 :
					result.Append("&ccedil;");
					break;
				case 174 :
					result.Append("&Ccedil;");
					break;
				default :
					// undo UTF8-encoding
					result.Append((char)utfChar);
			}
		} else {
			// undo UTF8-encoding
			result.Append((char)utfChar);
		}
	}
	str = result;
}

String LDAPConnection::GetTypeStr(int msgType)
{
	StartTrace(LDAPConnection.GetTypeStr);

	String type;
	switch (msgType) {
		case LDAP_RES_BIND:
			type = "LDAP_RES_BIND";
			break;

		case LDAP_RES_SEARCH_ENTRY:
			type = "LDAP_RES_SEARCH_ENTRY";
			break;

		case LDAP_RES_SEARCH_REFERENCE:
			type = "LDAP_RES_SEARCH_REFERENCE";
			break;

		case LDAP_RES_SEARCH_RESULT:
			type = "LDAP_RES_SEARCH_RESULT";
			break;

		case LDAP_RES_MODIFY:
			type = "LDAP_RES_MODIFY";
			break;

		case LDAP_RES_ADD:
			type = "LDAP_RES_ADD";
			break;

		case LDAP_RES_DELETE:
			type = "LDAP_RES_DELETE";
			break;

		case LDAP_RES_MODDN:
			type = "LDAP_RES_MODDN";
			break;

		case LDAP_RES_COMPARE:
			type = "LDAP_RES_COMPARE";
			break;

		case LDAP_RES_EXTENDED:
			type = "LDAP_RES_EXTENDED";
			break;

		default:
			break;
	}
	return type;
}
