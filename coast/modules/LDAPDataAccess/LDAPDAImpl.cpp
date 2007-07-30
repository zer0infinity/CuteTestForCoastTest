/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#if defined(__sun) || defined(WIN32) || defined(__linux__)

//--- interface include --------------------------------------------------------
#include "LDAPDAImpl.h"

//--- project modules used -----------------------------------------------------
#include "LDAPParams.h"
#include "LDAPMessageEntry.h"

//--- standard modules used ----------------------------------------------------
#include "SysLog.h"
#include "Renderer.h"
#include "Dbg.h"
#include "Timers.h"

#if defined(ONLY_STD_IOSTREAM)
using namespace std;
#endif

//---- LDAPDAImpl ---------------------------------------------------------
RegisterDataAccessImpl(LDAPDAImpl);
LDAPDAImpl::LDAPDAImpl(const char *name) : DataAccessImpl(name)  { }
LDAPDAImpl::~LDAPDAImpl() { }

IFAObject *LDAPDAImpl::Clone() const
{
	return new LDAPDAImpl(fName);
}

bool LDAPDAImpl::Exec(Context &ctx, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(LDAPDAImpl.DoExec);
#ifndef NO_LDAP
	bool retVal = true;
	LDAPParams params(ctx, in);
	DAAccessTimer(LDAPDAImpl.Exec, params.Base(), ctx);

	LDAP *ldapHdl = Connect(ctx, in, out);
	if (ldapHdl != NULL) {
		if ( params.FilterDefined() ) {
			int msgId;
			SubTrace(Params, "Base:<" << params.Base() << ">");
			SubTrace(Params, "Filter:<" << params.Filter() << ">");
			SubTrace(Params, "Attributes:<" << (long)params.Attributes() << ">");

			// if tv_sec and tv_usec are both 0, we pass a NULL pointer which causes the server specified timeout to be observed.
			// tv_sc == 0 and tv_usec == 0 is a parameter error.
			int rc = ldap_search_ext( ldapHdl, params.Base(), params.Scope(),
									  params.Filter(), params.Attributes(), 0,  NULL, NULL, params.Timeout4FuncCall(), params.SizeLimit(), &msgId );

			if ( rc != LDAP_SUCCESS ) {
				String msg("LDAPSearch  Base[");
				msg << params.Base() << "] Filter[" << params.Filter() << "] " << ldap_err2string( rc );
				SysLog::Error(msg);
				Anything aError;
				aError["Error"]["Msg"] = msg;
				aError["Error"]["Code"] = rc;
				out->Put("LDAPResult", aError, ctx);
				out->Put("Error", msg, ctx);
				retVal = false;
			} else {
				retVal = DoSearch(ldapHdl, "LDAPSearch", msgId, 0,  ctx, in, out);
			}
		} else {
			SysLog::Error("No SearchFilter defined");
			out->Put("Error", String("No SearchFilter defined"), ctx);
			retVal = false;
		}
		ldap_unbind( ldapHdl );
	} else {
		retVal = false;
	}
	return retVal;
#else
	return false;
#endif
}

void LDAPDAImpl::HandleError(LDAP *ldapHdl, Context &ctx, ResultMapper *out, const String &msg)
{
	StartTrace(LDAPDAImpl.HandleError);
	long errCode = 0;

	if (ldapHdl) {
		errCode = ldap_get_lderrno( ldapHdl, NULL, NULL );
		ldap_unbind( ldapHdl );
	}
	if (errCode == 0) {
		out->Put("ErrorConnect", msg, ctx);
	} else {
		String errMsg;
		errMsg << msg << ": " << ldap_err2string(errCode) << " (" << errCode << ", " << fName << ")";
		Anything aError;
		aError["Error"]["Msg"] = errMsg;
		aError["Error"]["Code"] = errCode;
		out->Put("ErrorConnect", aError, ctx);
	}
	out->Put("Error", msg, ctx);
	SysLog::Error(msg);
}

LDAP *LDAPDAImpl::Connect(Context &ctx, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(LDAPDAImpl.Connect);
#ifndef NO_LDAP
	int ldapVersion = LDAP_VERSION_MAX;
	const char *server = Lookup("Server", "localhost");
	long port = Lookup("Port", 389L);
	long connTimeout = Lookup("Timeout", 10L);
	int msgId;

	Trace("Server               : " << server);
	Trace("Port                 : " << port);
	Trace("Timeout (in seconds) : " << connTimeout);

	LDAP	  *ldapHdl = ::ldap_init( server, port );
	if ( !ldapHdl ) {
		Trace("==================================== ::ldap_init FAILED");
		String msg("Could not connect to ");
		msg << server << " at " << port;

		HandleError(ldapHdl, ctx, out, msg);
		return 0;
	}

	if ( ::ldap_set_option( ldapHdl, LDAP_OPT_PROTOCOL_VERSION, &ldapVersion ) != LDAP_SUCCESS ) {
		Trace("==================================== ::ldap_set_option: LDAP_OPT_PROTOCOL_VERSION   FAILED");
		HandleError(ldapHdl, ctx, out, "ldap_set_option [LDAP_OPT_PROTOCOL_VERSION]");
		return 0;
	}

	int adjustConnTimeout = connTimeout *= 1000;
	if ( ::ldap_set_option( ldapHdl, LDAP_X_OPT_CONNECT_TIMEOUT, &adjustConnTimeout ) != LDAP_SUCCESS ) {
		Trace("==================================== ::ldap_set_option: LDAP_X_OPT_CONNECT_TIMEOUT   FAILED");
		HandleError(ldapHdl, ctx, out, "ldap_set_option  [LDAP_X_OPT_CONNECT_TIMEOUT]");
		return 0;
	}

	String errorMsg;
	String strBind;
	ROAnything bindSpec(Lookup("Bind"));
	Renderer::RenderOnString(strBind, ctx, bindSpec);
	Trace(strBind);
	TraceAny(bindSpec, "The bind spec");
	if ( strBind.Length() == 0 || strBind == "Anonymous" ) {
		Trace("==================================== Bind Anonymous");
		errorMsg = "Anonymous Bind";
		msgId = ::ldap_simple_bind( ldapHdl, NULL, NULL );
	} else {
		Trace("==================================== Bind NOT Anonymous");
		errorMsg = strBind;
		errorMsg << "  ";
		String bindPW;
		ROAnything bindPWSpec(Lookup("BindPW"));
		Renderer::RenderOnString(bindPW, ctx, bindPWSpec);
		if ( bindPW != "undefined" ) {
			Trace("Binding with: <" << strBind << "><" << bindPW << ">");
			msgId = ::ldap_simple_bind( ldapHdl, strBind, bindPW );
		} else {
			Trace("==================================== Bind NOT Anonymous  Bindpassword not OK = ["  << bindPW << "]");
			errorMsg  << "Bindpassword " << bindPW;
			HandleError(ldapHdl, ctx, out, errorMsg);
			return 0;
		}
	}
	if ( msgId == -1 ) {
		Trace("==================================== Bind not OK !!!");
		HandleError(ldapHdl, ctx, out, errorMsg);
		return 0;
	}

	Trace("==================================== Bind OK !!!");

	if (DoConnect(ldapHdl, "LDAPConnect",  msgId, 0,  ctx, in, out)) {
		Trace("==================================== LDAPConnect OK !!!");
		return ldapHdl;
	} else {
		Trace("==================================== LDAPConnect NOT OK !!!");
		HandleError(ldapHdl, ctx, out, "LDAPConnect");
		return 0;  //Connect failure
	}
#else
	return 0;
#endif
}

int LDAPDAImpl::WaitForResult( LDAPMessage **result, LDAP *ldapHdl, String messageIn,
							   timeval *timeLimitIn, int msgId, int all,
							   Context &c, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(LDAPDAImpl.WaitForResult);
#if !defined(NO_LDAP)

	int rc;

	// timeval sec/usec == 0 : polling behaviour
	// timeval NULL			 : block
	// other				 : timeout after n sec/usec

	if (timeLimitIn->tv_sec == 0 && timeLimitIn->tv_usec == 0) {
		rc = ldap_result(ldapHdl, msgId, all, NULL, result);
	} else {
		rc = ldap_result(ldapHdl, msgId, all, timeLimitIn, result);
	}
	if (rc == -1) {
		// in case we don't have a LDAPMessage structure
		int rc_errno = ldap_get_lderrno( ldapHdl, NULL, NULL );
		String msg(messageIn <<  "  " << ldap_err2string( rc_errno ));

		SysLog::Error(msg);
		out->Put("Error", msg, c);
		out->Put("ErrorWaitForResult", msg, c);
	}
	return rc;
#else
	return 0;
#endif
}

bool LDAPDAImpl::DoAddModifyDel(LDAP *ldapHdl, String messageIn,
								int msgId, int all,
								Context &c, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(LDAPAddDAImpl.DoAddModifyDel);
	Trace( "=========================== MessageIn = [" << messageIn << "]" );
#ifndef NO_LDAP
	bool retVal = true;
	LDAPModifyCompareParams params(c, in);
	bool finished = false;
	while (finished == false) {
		LDAPMessage *result;
		LDAPMessageEntry lmEntry(&result);
		lmEntry.Use();
		int rc = WaitForResult(&result, ldapHdl, messageIn,  params.Timeout(), msgId, 0,  c, in, out);
		if (rc != -1) {
			finished = CheckAddDelModifyResult(rc, retVal , result, ldapHdl, messageIn,  params.Timeout() , msgId, c, in, out);
			if (finished == true) {
				break;
			}
		} else {
			retVal = false;
			break;
		}
	}
	return retVal;
#else
	return false;
#endif
}

bool LDAPDAImpl::DoSearch(LDAP *ldapHdl, String messageIn,
						  int msgId, int all,
						  Context &c, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(LDAPDAImpl.DoSearch);
	DAAccessTimer(LDAPDAImpl.DoSearch, "", c);
#ifndef NO_LDAP
	bool retVal = true;
	LDAPModifyCompareParams params(c, in);
	bool finished = false;
	while (finished == false) {
		// auto memory mgmt
		LDAPMessage *result;
		LDAPMessageEntry lmEntry(&result);
		lmEntry.Use();
		int rc = WaitForResult(&result, ldapHdl, messageIn,  params.Timeout(), msgId, 0,  c, in, out);
		if (rc != -1) {
			finished = CheckSearchResult(rc, retVal , result, ldapHdl, messageIn,  params.Timeout() , msgId, c, in, out);
			if (finished == true) {
				break;
			}
		} else {
			retVal = false;
			break;
		}
	}
	return retVal;
#else
	return false
#endif
}

bool LDAPDAImpl::DoConnect(LDAP *ldapHdl, String messageIn,
						   int msgId, int all,
						   Context &c, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(LDAPAddDAImpl.DoConnect);
#ifndef NO_LDAP
	bool outcome = false;
	timeval timeout;
	bool finished = false;
	long connTimeout = Lookup("Timeout", 10L);
	timeout.tv_sec  = connTimeout;
	timeout.tv_usec = 0;

	while (finished == false) {
		LDAPMessage *result;
		LDAPMessageEntry lmEntry(&result);
		lmEntry.Use();
		int rc = WaitForResult(&result, ldapHdl, messageIn, &timeout,  msgId, 0,  c, in, out);
		if (rc != -1) {
			finished = CheckConnectResult(rc, outcome , result, ldapHdl, messageIn,  &timeout , msgId, c, in, out);
			if (finished == true) {
				break;
			}
		} else {
			outcome = false;
			break;
		}
	}
	return outcome;
#else
	return false;
#endif
}

bool LDAPDAImpl::CheckConnectResult(int opRet, bool &outcome, LDAPMessage *result, LDAP *ldapHdl, String messageIn,
									timeval *timeLimitIn, int msgId,
									Context &c, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(LDAPDAImpl.CheckConnectResult);
#if !defined(NO_LDAP)
	// to comply with HandleTimeout return code, 1 is failure, 0 is success !!!!
	if (opRet == 0) {
		Trace("===================== opRet == 0 !!!");
		bool retCode = HandleTimeout(opRet, outcome, result, ldapHdl, messageIn, timeLimitIn, msgId, c, in, out);
		return retCode;
	}
	// don't free result
	int rc = ldap_result2error(ldapHdl, result, 0);
	if (rc != LDAP_SUCCESS) {
		String msg(messageIn <<  "  " << ldap_err2string( rc ));
		SysLog::Error(msg);
		Trace("===================== ldap_result2error failed !!!  [" << msg << "]");
		outcome = false;
		return true;
	}
	Trace("===================== CheckConnectResult succeeded !!!");
	outcome = true;
	return true;
#else
	return false;
#endif
}

bool LDAPDAImpl::CheckSearchResult(int opRet, bool &outcome, LDAPMessage *result, LDAP *ldapHdl, String messageIn,
								   timeval *timeLimitIn, int msgId,   Context &c, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(LDAPDAImpl.CheckSearchResult);
#if !defined(NO_LDAP)
	outcome = false;
	if (opRet == 0) {
		return (HandleTimeout(opRet, outcome, result, ldapHdl, messageIn, timeLimitIn, msgId, c, in, out));
	}
	switch (opRet) {
		case LDAP_RES_SEARCH_ENTRY: {
			Trace("opRet:LDAP_RES_SEARCH_ENTRY");
			GetData( result, ldapHdl, c, out);
			return 0;
		}
		case LDAP_RES_SEARCH_RESULT: {
			Trace("opRet:LDAP_RES_SEARCH_RESULT");
			char *matched_msg = NULL;   // don't need this
			char *error_msg = NULL;     // don't need this
			LDAPControl **serverctrls;
			// don't free the memory
			int parse_rc = ldap_parse_result( ldapHdl, result, &opRet, &matched_msg, &error_msg, NULL, &serverctrls, 0 );
			if ( parse_rc != LDAP_SUCCESS ) {
				Trace("!LDAP_SUCCESS");
				LDAPParams params(c, in);
				String msg("LDAPSearch  Base[");
				msg << params.Base() << "] Filter[" << params.Filter() << "] " << ldap_err2string( opRet );
				SysLog::Error(msg);
				Anything aError;
				aError["Error"]["Msg"] = msg;
				aError["Error"]["Code"] = opRet;
				out->Put("LDAPResult", aError, c);
				out->Put("Error", msg, c);
				outcome = false;
			} else {
				if (opRet != LDAP_SUCCESS) {
					Trace("opRet:!LDAP_SUCCESS");
					LDAPParams params(c, in);
					String msg("LDAPSearch  Base[");
					msg << params.Base() << "] Filter[" << params.Filter() << "] " << ldap_err2string( opRet );
					String partialMatch;
					ldap_get_lderrno( ldapHdl, &matched_msg, &error_msg );
					if ( error_msg != NULL && *error_msg != '\0' ) {
						msg << " " << error_msg;
					}
					if ( matched_msg != NULL && *matched_msg != '\0' ) {
						partialMatch <<  matched_msg;
					}
					SysLog::Error(msg);
					Anything aError;
					aError["Error"]["Msg"] = msg;
					aError["Error"]["Code"] = opRet;
					aError["Error"]["PartialMatch"] = partialMatch;
					out->Put("LDAPResult", aError, c);
					out->Put("Error", msg, c);
					outcome = false;
				} else {
					Trace("search was success");
					// A search is ok even when nothing is found. Check and
					// create empty result slot.
					Anything tmpStore;
					tmpStore = c.GetTmpStore();
					if (!(tmpStore["Mapper"].IsDefined("LDAPResult"))) {
						// Don't use Mapper to output since different Mappers will
						// store the hint differently and make lookups impossible.
						tmpStore["Mapper"]["Info"]["LdapSearchFoundEntryButNoData"] = 1L;
						String tmpString("");
						out->Put("LDAPResult", tmpString, c);
					}
					outcome = true;
				}
			}
			return 1;
		}
		case LDAP_RES_SEARCH_REFERENCE:
			Trace("opRet:LDAP_RES_SEARCH_REFERENCE");
			return 0;
		default:
			Trace("default-case");
			return 1;
	}
#else
	return false;
#endif
}

bool LDAPDAImpl::CheckAddDelModifyResult(int opRet, bool &outcome, LDAPMessage *result, LDAP *ldapHdl, String messageIn,
		timeval *timeLimitIn, int msgId,   Context &c, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(LDAPModifyDaImpl.CheckResult);
#if !defined(NO_LDAP)
	outcome = false;
	if (opRet == 0) {
		return (HandleTimeout(opRet, outcome, result, ldapHdl, messageIn, timeLimitIn, msgId, c, in, out));
	} else {
		LDAPModifyCompareParams params(c, in);
		char *matched_msg = NULL;   // don't need this
		char *error_msg = NULL;     // don't need this
		LDAPControl **serverctrls;
		// don't free the memory
		int parse_rc = ldap_parse_result( ldapHdl, result, &opRet, &matched_msg, &error_msg, NULL, &serverctrls, 0 );
		if ( parse_rc != LDAP_SUCCESS ) {
			String msg;
			msg << messageIn << "  dn<" << params.DName() << ">: " << ldap_err2string( opRet );
			SysLog::Error(msg);
			Anything aError;
			aError["Error"]["Msg"] = msg;
			aError["Error"]["Code"] = opRet;
			out->Put("LDAPResult", aError, c);
			out->Put("Error", msg, c);
			outcome = false;
		} else {
			if (opRet == LDAP_SUCCESS) {
				String msg;
				msg << messageIn << " dn<" << params.DName() << ">  was successfull";
				out->Put("LDAPResult", msg, c);
				outcome = true;
			} else {
				String partialMatch;
				ldap_get_lderrno( ldapHdl, &matched_msg, &error_msg );
				String msg;
				msg << messageIn << "  dn<" << params.DName() << ">: " << ldap_err2string( opRet );
				if ( error_msg != NULL && *error_msg != '\0' ) {
					msg << " " << error_msg;
				}
				if ( matched_msg != NULL && *matched_msg != '\0' ) {
					partialMatch <<  matched_msg;
				}
				SysLog::Error(msg);
				Anything aError;
				aError["Error"]["Msg"] = msg;
				aError["Error"]["Code"] = opRet;
				aError["Error"]["PartialMatch"] = partialMatch;
				out->Put("Error", msg, c);
				outcome = false;
			}
		}
		return 1;
	}
#else
	return false;
#endif
}

bool LDAPDAImpl::GetDNComponents(const char *dn, Anything &dnParts)
{
	StartTrace(LDAPDAImpl.GetDNComponents);
	char **results;
	results = ldap_explode_dn(dn, 0);
	if (results == NULL) {
		return false;
	}
	for (long i = 0; results[i] != NULL; i++) {
		StringTokenizer strtk(results[i], '=');
		String attrName;
		strtk.NextToken(attrName);
		String attrValue;
		strtk.NextToken(attrValue);
		dnParts[attrName] = attrValue;
	}
	ldap_value_free(results);
	return true;
}

void LDAPDAImpl::GetData( LDAPMessage * /* result */  e, LDAP *ldapHdl, Context &c, ResultMapper *out )
{
	StartTrace(LDAPDAImpl.GetData);
#if !defined(NO_LDAP)
	Anything nextResult;

	BerElement      *ber = 0; // PS: init it
	char *dn = ldap_get_dn( ldapHdl, e );
	if (dn) {
		nextResult["dn"] = dn;
		if (Lookup("GetDNComponents", 0L)) {
			GetDNComponents(dn, nextResult["dnComponents"]);
		}
		ldap_memfree( dn );
	} else {
		nextResult["dn"] = "None";
	}

	for (char *a = ldap_first_attribute( ldapHdl, e, &ber );
		 a != NULL;
		 a = ldap_next_attribute( ldapHdl, e, ber ) ) {
		String a_lower_case = a;
		a_lower_case.ToLower();
		//char** vals = ldap_get_values( ldapHdl, e, a );
		struct berval **vals = ldap_get_values_len( ldapHdl, e, a );
		if ( vals ) {
			for ( unsigned int i = 0; vals[i]; i++ ) {
				String str((void *)vals[i]->bv_val, (long)vals[i]->bv_len);
				MapUTF8Chars(str);
				//CopyUTF8(str, vals[i]);
				if (i == 0 ) {
					nextResult[a_lower_case] = str;
				} else {
					nextResult[a_lower_case].Append(str);
				}

			}
			ldap_value_free_len( vals );
		}
		if ( a ) {
			ldap_memfree( a );
		}
	}
	if ( ber ) {
		ber_free( ber, 0 );
	}
	out->Put("LDAPResult", nextResult, c);
	TraceAny(nextResult, "LDAPResult:");
#endif
}

bool LDAPDAImpl::HandleTimeout(int opRet, bool &outcome, LDAPMessage *result, LDAP *ldapHdl, String messageIn,
							   timeval *timeLimitIn, int msgId,   Context &c, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(LDAPDAImpl.HandleTimeout);
#if !defined(NO_LDAP)
	String msg;

	if (timeLimitIn->tv_sec != 0 || timeLimitIn->tv_usec != 0) {		// no polling, timeout)
		msg << "The request <" << messageIn << "> took more then <" << (long) timeLimitIn->tv_sec << "> seconds. ";
		ldap_abandon_ext(ldapHdl, msgId, NULL, NULL);
		SysLog::Error(msg);
		out->Put("Error", msg, c);
		out->Put("ErrorTimeout", msg, c);
		outcome = false;
		return 1;
	} else {			// Polling case - wait forever
		return 0;
	}
#else
	return false;
#endif
}

//---- LDAPCompareDAImpl ---------------------------------------------------------
RegisterDataAccessImpl(LDAPCompareDAImpl);
LDAPCompareDAImpl::LDAPCompareDAImpl(const char *name) : LDAPDAImpl(name) { }
LDAPCompareDAImpl::~LDAPCompareDAImpl() { }
IFAObject *LDAPCompareDAImpl::Clone() const
{
	return new LDAPCompareDAImpl(fName);
}

bool LDAPCompareDAImpl::Exec(Context &ctx, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(LDAPCompareDAImpl.DoExec);
#ifndef NO_LDAP
	bool retVal = true;
	LDAPModifyCompareParams params(ctx, in);

	LDAP *ldapHdl = Connect(ctx, in, out);
	if (ldapHdl != NULL) {
		if ( params.DNameDefined() ) {
			int msgId;
			berval CompareValue;

			CompareValue.bv_val = params.Value2Compare();
			CompareValue.bv_len = params.V2CLength();

			SubTrace(Params, "dn:<" << params.DName() << ">" );
			SubTrace(Params, "Value:<" << params.Value2Compare() << ">" );

			LDAPMessage     *result = 0;
			int  rc = ldap_compare_ext(  ldapHdl, params.DName(), params.Attribute(),
										 &CompareValue, NULL, NULL, &msgId );
			if ( rc != LDAP_SUCCESS ) {
				String msg("LDAPCompare  ");
				msg << "dn<" << params.DName() << ">: " << ldap_err2string( rc );
				SysLog::Error(msg);
				Anything aError;
				aError["Error"]["Msg"] = msg;
				aError["Error"]["Code"] = rc;
				out->Put("Error", msg, ctx);
				retVal = false;
			} else {

				bool finished = false;
				while (finished == false) {
					int rc2 = WaitForResult(&result, ldapHdl, "LDAP Compare",  params.Timeout(), msgId, 0,  ctx, in, out);
					if (rc2 != -1) {
						finished = CheckCompareResult(rc2, retVal , result, ldapHdl, "LDAP Compare",  params.Timeout() , msgId, ctx, in, out);
						if (finished == true) {
							break;
						}
					} else {
						retVal = false;
						break;
					}
				}
			}
		} else {
			SysLog::Error("No DN defined");
			out->Put("Error", String("No DN defined"), ctx);
			retVal = false;
		}
		ldap_unbind( ldapHdl );
	} else {
		retVal = false;
	}
	return retVal;
#else
	return false;
#endif
}

bool LDAPCompareDAImpl::CheckCompareResult(int opRet, bool &outcome, LDAPMessage *result, LDAP *ldapHdl, String messageIn,
		timeval *timeLimitIn, int msgId,   Context &c, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(LDAPCompareDAImpl.CheckCompareResult);
#if !defined(NO_LDAP)
	outcome = false;
	if (opRet == 0) {
		return (HandleTimeout(opRet, outcome, result, ldapHdl, messageIn, timeLimitIn, msgId, c, in, out));
	} else {
		LDAPModifyCompareParams params(c, in);
		char *matched_msg = NULL;  // don't need this
		char *error_msg = NULL;    // don't need this
		LDAPControl **serverctrls;
		// don't free the memory
		int parse_rc = ldap_parse_result( ldapHdl, result, &opRet, &matched_msg, &error_msg, NULL, &serverctrls, 0 );
		if ( parse_rc != LDAP_SUCCESS ) {
			String msg("LDAPCompare  ");
			msg << "dn<" << params.DName() << ">: " << ldap_err2string( opRet );
			SysLog::Error(msg);
			Anything aError;
			aError["Error"]["Msg"] = msg;
			aError["Error"]["Code"] = opRet;
			out->Put("Error", msg, c);
			outcome = false;
		} else {
			switch (opRet) {

				case LDAP_COMPARE_TRUE: {
					String msg("LDAPCompare of Attribute ");
					msg << *params.Attributes() << " of DN " << params.DName() << " was successfull" ;
					out->Put("LDAPResult", msg, c);
					TraceAny(c.GetTmpStore(), "TempStore: ");
					outcome = true;
					break;
				}
				case LDAP_COMPARE_FALSE: {
					String msg("LDAPCompare  ");
					msg << "dn<" << params.DName() << ">: " << ldap_err2string( opRet );
					SysLog::Error(msg);
					Anything aError;
					aError["Error"]["Msg"] = msg;
					aError["Error"]["Code"] = opRet;
					out->Put("Error", msg, c);
					outcome = false;
					break;
				}
				default: {
					String partialMatch;
					ldap_get_lderrno( ldapHdl, &matched_msg, &error_msg );
					String msg("LDAPCompare  ");
					msg << "dn<" << params.DName() << ">: " << ldap_err2string( opRet );
					if ( error_msg != NULL && *error_msg != '\0' ) {
						msg << " " << error_msg;
					}
					if ( matched_msg != NULL && *matched_msg != '\0' ) {
						partialMatch <<  matched_msg;
					}
					SysLog::Error(msg);
					Anything aError;
					aError["Error"]["Msg"] = msg;
					aError["Error"]["Code"] = opRet;
					aError["Error"]["PartialMatch"] = partialMatch;
					out->Put("Error", msg, c);
					outcome = false;
					break;
				}
			}
		}
		return 1;
	}
#else
	return false;
#endif
}

//---- LDAPModifyDAImpl ---------------------------------------------------------
RegisterDataAccessImpl(LDAPModifyDAImpl);
LDAPModifyDAImpl::LDAPModifyDAImpl(const char *name) : LDAPDAImpl(name) { }
LDAPModifyDAImpl::~LDAPModifyDAImpl() { }
IFAObject *LDAPModifyDAImpl::Clone() const
{
	return new LDAPModifyDAImpl(fName);
}

bool LDAPModifyDAImpl::Exec(Context &ctx, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(LDAPModifyDAImpl.DoExec);
#ifndef NO_LDAP
	bool retVal = true;
	LDAPModifyCompareParams params(ctx, in);

	LDAP *ldapHdl = Connect(ctx, in, out);
	if (ldapHdl != NULL) {
		if ( params.DNameDefined() ) {
			int msgId;

			SubTrace(Params, "dn:<" << params.DName() << ">" );
			params.DebugEntries2Modify(cerr);
			int rc = ldap_modify_ext(ldapHdl, params.DName(), params.Entries2Modify(), NULL, NULL, &msgId);
			if ( rc != LDAP_SUCCESS ) {
				String msg("LDAPModify  ");
				msg << "dn<" << params.DName() << ">: " << ldap_err2string( rc );
				SysLog::Error(msg);
				Anything aError;
				aError["Error"]["Msg"] = msg;
				aError["Error"]["Code"] = rc;
				out->Put("Error", msg, ctx);
				retVal = false;
			} else {
				retVal = DoAddModifyDel(ldapHdl, "LDAPModify", msgId, 0,  ctx, in, out);
			}
		} else {
			SysLog::Error("No DN defined");
			out->Put("Error", String("No DN defined"), ctx);
			retVal = false;
		}
		ldap_unbind( ldapHdl );
	} else {
		retVal = false;
	}
	return retVal;
#else
	return false;
#endif
}

//---- LDAPAddDAImpl ---------------------------------------------------------
RegisterDataAccessImpl(LDAPAddDAImpl);
LDAPAddDAImpl::LDAPAddDAImpl(const char *name) : LDAPDAImpl(name) { }
LDAPAddDAImpl::~LDAPAddDAImpl() { }
IFAObject *LDAPAddDAImpl::Clone() const
{
	return new LDAPAddDAImpl(fName);
}

bool LDAPAddDAImpl::Exec(Context &ctx, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(LDAPAddDAImpl.DoExec);
#ifndef NO_LDAP
	bool retVal = true;
	LDAPModifyCompareParams params(ctx, in);

	LDAP *ldapHdl = Connect(ctx, in, out);
	if (ldapHdl != NULL) {
		Trace( "======================== ldapHdl != NULL !!!");
		if ( params.DNameDefined() ) {
			Trace( "======================== DNameDefined !!!");
			int msgId;
			SubTrace(Params, "DName:<" << params.DName() << ">");
			params.DebugEntries2Modify(cerr);

			int rc = ldap_add_ext(ldapHdl, params.DName(), params.Entries2Modify(), NULL, NULL, &msgId );

			if ( rc != LDAP_SUCCESS ) {
				Trace( "======================== rc != LDAP_SUCCESS !!!");
				String msg("LDAPAdd  ");
				msg << "dn<" << params.DName() << ">: " << ldap_err2string( rc );
				SysLog::Error(msg);
				Anything aError;
				aError["Error"]["Msg"] = msg;
				aError["Error"]["Code"] = rc;
				out->Put("Error", msg, ctx);
				retVal = false;
			} else {
				Trace( "======================== rc != LDAP_SUCCESS !!!");
				retVal = DoAddModifyDel(ldapHdl, "LDAPAdd",  msgId, 0,  ctx, in, out);
			}
		} else {
			Trace( "======================== DName is NOT Defined !!!");
			SysLog::Error("No DN defined");
			out->Put("Error", String("No DN defined"), ctx);
			retVal = false;
		}
		ldap_unbind( ldapHdl );
	} else {
		Trace( "======================== ldapHdl == NULL !!!");
		retVal = false;
	}
	return retVal;
#else
	return false;
#endif
}

//---- LDAPDelDAImpl ---------------------------------------------------------
RegisterDataAccessImpl(LDAPDelDAImpl);
LDAPDelDAImpl::LDAPDelDAImpl(const char *name) : LDAPDAImpl(name) { }
LDAPDelDAImpl::~LDAPDelDAImpl() { }
IFAObject *LDAPDelDAImpl::Clone() const
{
	return new LDAPDelDAImpl(fName);
}

bool LDAPDelDAImpl::Exec(Context &ctx, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(LDAPDelDAImpl.DoExec);
#ifndef NO_LDAP
	bool retVal = true;
	LDAPModifyCompareParams params(ctx, in);

	LDAP *ldapHdl = Connect(ctx, in, out);
	if (ldapHdl != NULL) {
		Trace( "======================== ldapHdl != NULL !!!");
		if ( params.DNameDefined() ) {
			Trace( "======================== DNameDefined !!!");
			int msgId;
			SubTrace(Params, "dn:<" << params.DName() << ">" );

			int rc = ldap_delete_ext(ldapHdl, params.DName(), NULL, NULL, &msgId );

			if ( rc != LDAP_SUCCESS ) {
				Trace( "======================== rc != LDAP_SUCCESS !!!");
				String msg("LDAPDel  ");
				msg << "dn<" << params.DName() << ">: " << ldap_err2string( rc );
				SysLog::Error(msg);
				Anything aError;
				aError["Error"]["Msg"] = msg;
				aError["Error"]["Code"] = rc;
				out->Put("Error", msg, ctx);
				retVal = false;
			} else {
				Trace( "======================== rc == LDAP_SUCCESS !!!");
				retVal = DoAddModifyDel(ldapHdl, "LDAPDel",  msgId, 0,  ctx, in, out);
			}
		} else {
			Trace( "======================== DName is NOT Defined !!!");
			SysLog::Error("No DN defined");
			out->Put("Error", String("No DN defined"), ctx);
			retVal = false;
		}
		ldap_unbind( ldapHdl );
	} else {
		Trace( "======================== ldapHdl == NULL !!!");
		retVal = false;
	}
	return retVal;
#else
	return false;
#endif
}

void LDAPDAImpl::MapUTF8Chars(String &str)
{
#ifndef NO_LDAP
	// FIXME: brain dead config switch...
	bool mapToHtml = !Lookup("NoHTMLCharMapping", 0L);
	if (!Lookup("PlainBinaryValues", 0L)) {

		String result;
		for (long i = 0L; i < str.Length();) {
			const char *theChar = ((const char *)str) + i;
			const char *theStartChar = theChar;
			unsigned long utfChar = ldap_utf8getcc(&theChar);
			i += (theChar - theStartChar); // one utf8 symbol might be several characters
			//u_char c= LDAP_UTF8GETCC(src);
			if (mapToHtml) {
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
						result.Append((char)utfChar); // this is a shortcut
				}
			} else {
				result.Append((char)utfChar);    // undo UTF8 encoding
			}
		}
		str = result;
	}
#endif
}

#endif
