/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "LDAPDataAccessImpls.h"

//--- project modules used -----------------------------------------------------
#include "Renderer.h"

//--- standard modules used ----------------------------------------------------
#include "SystemLog.h"
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------
#include <cstdlib>

//--- LDAPAbstractDAI -----------------------------------------------------

void ReleaseHandleInfo(Context &ctx, LDAPConnection *lc)
{
	StartTrace(LDAPAbstractDAI.ReleaseHandleInfo);
	TraceAny(ctx.GetTmpStore(), "tmp store dump:");
	lc->ReleaseHandleInfo();
	if ( lc != (LDAPConnection *) NULL ) {
		Trace("Deleting lc");
		delete lc;
	}
}

void TryReleaseHandleInfo(Context &ctx, LDAPConnection *lc, LDAPErrorHandler &eh)
{
	StartTrace(LDAPAbstractDAI.TryReleaseHandleInfo);
	if ( eh.GetRetryState() != LDAPErrorHandler::eRetry ) {
		Trace("Calling ReleaseHandleInfo");
		lc->ReleaseHandleInfo();
	}
	if ( lc != (LDAPConnection *) NULL ) {
		Trace("Deleting lc");
		delete lc;
	}
}

RegisterDataAccessImpl(LDAPAbstractDAI);

bool LDAPAbstractDAI::Exec( Context &ctx, ParameterMapper *getter, ResultMapper *putter)
{
	StartTrace(LDAPAbstractDAI.Exec);
	Trace("Executing LDAP data access: " << fName);
	LDAPErrorHandler eh(ctx, getter, putter, fName);

	bool ret( false );
	if ( (ret = DoExec(ctx, getter, putter, eh)) == true ) {
		// LDAP transaction completed successfully
		return ret;
	}
	if ( eh.GetRetryState() == LDAPErrorHandler::eRetry ) {
		Trace("Will try a rebind (LDAP might have been restarted.) for DataAccess: " << fName);
		eh.HandleSessionError((LDAP *) NULL, "Will try a rebind (LDAP might have been restarted.)");
		eh.CleanUp();
		eh.SetRetryState(LDAPErrorHandler::eIsInRetrySequence);
		return DoExec(ctx, getter, putter, eh);
	}
	return ret;
}

LDAPConnection *LDAPAbstractDAI::LDAPConnectionFactory(ROAnything cp)
{
	StartTrace(LDAPAbstractDAI.LDAPConnectionFactory);
	if ( cp["PooledConnections"].AsLong(0) == 0L ) {
		return new LDAPConnection(cp);
	} else {
		return new PersistentLDAPConnection(cp);
	}
}

bool LDAPAbstractDAI::DoExec( Context &ctx, ParameterMapper *getter, ResultMapper *putter, LDAPErrorHandler &eh)
{
	StartTrace(LDAPAbstractDAI.DoExec);
	Trace("Executing LDAP data access: " << fName);

	// 1. get + check query
	Anything query;
	if ( !DoGetQuery(getter, ctx, query, eh) ) {
		TraceAny(ctx.GetTmpStore(), "tmp store dump:");
		return false;
	}
	Trace("Got query and checked it: query is ok");

	// store query params in ctx (lookup for error-handling)
	eh.PutQueryParams(query);

	// 3. connect
	Anything cp;

	String server("No server specified!");
	getter->Get("LDAPServer", server, ctx);
	cp["Server"] = server;

	long port = 389L;
	getter->Get("LDAPPort", port, ctx);
	cp["Port"] = port;

	String bindName("No bind name specified!");
	getter->Get("LDAPBindName", bindName, ctx);
	cp["BindName"] = bindName;

	String bindPw("No bind password specified!");
	getter->Get("LDAPBindPW", bindPw, ctx);
	cp["BindPW"] = bindPw;

	long timeout = 10L;
	getter->Get("LDAPTimeout", timeout, ctx);
	cp["Timeout"] = timeout;

	long connTimeout = 10L;
	getter->Get("LDAPConnectionTimeout", connTimeout, ctx);
	cp["ConnectionTimeout"] = connTimeout;

	long noHTMLCharMapping = 0L;
	getter->Get("NoHTMLCharMapping", noHTMLCharMapping, ctx);
	cp["MapUTF8"] = !noHTMLCharMapping;

	long plainBinaryValues = 0L;
	getter->Get("PlainBinaryValues", plainBinaryValues, ctx);
	cp["PlainBinaryValues"] = plainBinaryValues;

	long pooledConnections = 0L;
	getter->Get("LDAPPooledConnections", pooledConnections, ctx);
	cp["PooledConnections"] = pooledConnections;

	long maxConnections = 0L;
	getter->Get("LDAPMaxConnections", maxConnections, ctx);
	cp["MaxConnections"] = maxConnections;

	long tryAutoRebind = 0L;
	getter->Get("LDAPTryAutoRebind", tryAutoRebind, ctx);
	cp["TryAutoRebind"] = tryAutoRebind;

	long rebindTimeout = 3600L;
	getter->Get("LDAPRebindTimeout", rebindTimeout, ctx);
	cp["RebindTimeout"] = rebindTimeout;

	eh.PutConnectionParams(cp);

	LDAPConnection *lc( LDAPConnectionFactory(cp) );
	if ( lc == (LDAPConnection *) NULL ) {
		eh.HandleError("Fatal LDAP error. Could not create LDAP connection on heap.");
		return false;
	}

	// put away Query for logging purposes etc
	putter->Put("Query", query, ctx);

	if ( !(lc->Connect(cp, eh)) ) {
		ReleaseHandleInfo(ctx, lc);
		return false;
	}
	Trace("Connection to server established: " << bindName << "@" << server << ":" << port);
	// 4. request ldap operation, then wait for result (loop)
	int msgId(-1);
	int iRc( DoLDAPRequest(ctx, getter, lc, query, msgId) );
	Trace("LDAP request sent, retCode: " << iRc);

	Anything result;
	if ( !(lc->WaitForResult(msgId, result, eh)) ) {
		TryReleaseHandleInfo(ctx, lc, eh);
		return false;
	}
	Trace("Received LDAP result(s)");
	// 5. store result
	String key("LDAPResult.");
	key.Append( fName );
	bool storedOk( putter->Put(key, result, ctx) );
	Trace("Result stored successfully : " << storedOk);
	ReleaseHandleInfo(ctx, lc);
	return true;
}

bool LDAPAbstractDAI::DoGetQuery(ParameterMapper *getter, Context &ctx, Anything &query, LDAPErrorHandler &eh)
{
	StartTrace(LDAPAbstractDAI.DoGetQuery);

	// search, check + add ldap base DN
	String base(64L);
	getter->Get("LDAPBase", base, ctx);

	if ( !base.Length() ) {
		eh.HandleError("Illegal Base DN in query (empty or not defined)");
		return false;
	}
	query["Base"] = base;
	return true;
}

bool LDAPAbstractDAI::CheckAttributes(Anything &attrs, bool bRemoveNullValues, LDAPErrorHandler &eh)
{
	StartTrace(LDAPAbstractDAI.CheckAttributes);
	return DoCheckAttributes(attrs, bRemoveNullValues, eh);
}

bool LDAPAbstractDAI::DoCheckAttributes(Anything &attrs, bool bRemoveNullValues, LDAPErrorHandler &eh)
{
	StartTrace(LDAPAbstractDAI.DoCheckAttributes);
	// attrs must contain attr/value
	TraceAny(attrs, "attributes to clean");
	long nofAttrs( attrs.GetSize() );
	String strAttrName;
	for (long i = 0; i < nofAttrs; ++i) {
		strAttrName = attrs.SlotName(i);
		if ( !strAttrName.Length() ) {
			Trace("failed at index:" << i);
			String msg( "All values listed under 'Attrs' must have an associated attribute name. Failed at attr: ");
			msg.Append(i);
			eh.HandleError(msg);
			return false;
		}
		// remove attributes without any value assigned -> NullAny
		if ( bRemoveNullValues && attrs[i].IsNull() ) {
			Trace("removing [" << attrs.SlotName(i) << "] which is null");
			attrs.Remove(i);
			// readjust index
			--i;
			--nofAttrs;
		}
	}
	return true;
}

// =========================================================================

//--- LDAPAddDAI -----------------------------------------------------
RegisterDataAccessImpl(LDAPAddDAI);

bool LDAPAddDAI::DoGetQuery(ParameterMapper *getter, Context &ctx, Anything &query, LDAPErrorHandler &eh)
{
	StartTrace(LDAPAddDAI.DoGetQuery);

	// use superclass to get base argument
	if ( !LDAPAbstractDAI::DoGetQuery(getter, ctx, query, eh) ) {
		return false;
	}

	// get attributes, check + add
	Anything attrs;
	getter->Get("LDAPAttrs", attrs, ctx);
	TraceAny(attrs, "Got Attrs:");

	// attrs must be defined and not empty!
	if (attrs.IsNull() || attrs.GetSize() <= 0) {
		eh.HandleError("No attribute type/value pairs defined in add query");
		return false;
	}

	if ( !CheckAttributes(attrs, true, eh) ) {
		return false;
	}
	query["Mods"]["add"] = attrs;
	return true;
}

int LDAPAddDAI::DoSpecificOperation(LDAPConnection *lc, String base, LDAPMod **ldapmods, int &iMsgId)
{
	StatTrace(LDAPAddDAI.DoSpecificOperation, "sending add request", Storage::Current() );
	return ldap_add_ext(lc->GetConnection(), base, ldapmods, NULL, NULL, &iMsgId );
}

// =========================================================================

//--- LDAPCompareDAI -----------------------------------------------------
RegisterDataAccessImpl(LDAPCompareDAI);

bool LDAPCompareDAI::DoGetQuery(ParameterMapper *getter, Context &ctx, Anything &query, LDAPErrorHandler &eh)
{
	StartTrace(LDAPCompareDAI.DoGetQuery);

	// call superclass first, base dn must be present
	if ( !LDAPAbstractDAI::DoGetQuery(getter, ctx, query, eh) ) {
		return false;
	}

	// check attrname and value
	String attrName, attrVal;

	if ( !getter->Get("LDAPAttrName", attrName, ctx) || attrName.IsEqual("") ) {
		eh.HandleError("Attribute name not defined (or empty) in compare query");
		return false;
	}
	query["AttrName"] = attrName;

	if ( !getter->Get("LDAPAttrValue", attrVal, ctx) || attrVal.IsEqual("") ) {
		eh.HandleError("Value to compare not defined (or empty) in compare query");
		return false;
	}
	query["AttrValue"] = attrVal;

	return true;
}

int LDAPCompareDAI::DoLDAPRequest(Context &ctx, ParameterMapper *getter, LDAPConnection *lc, ROAnything query, int &iMsgId)
{
	StartTrace(LDAPCompareDAI.DoLDAPRequest);
	TraceAny(query, "Query for compare request:");

	// get arguments
	String base( query["Base"].AsString() );
	String attrName( query["AttrName"].AsString() );
	String attrVal( query["AttrValue"].AsString() );
	Trace("comparison of value with length: " << attrVal.Length() << " value [" << attrVal.DumpAsHex() << "]");
	int iOpCode(0);
	berval CompareValue;
	CompareValue.bv_val = (char *)(const char *)attrVal;
	CompareValue.bv_len = attrVal.Length();
	if ( ( iOpCode = ldap_compare_ext( lc->GetConnection(), base, attrName, &CompareValue, NULL, NULL, &iMsgId ) ) == LDAP_SUCCESS ) {
		Trace("compare successful, iOpCode:" << (long)iOpCode << " iMsgId:" << (long)iMsgId);
	} else {
		Trace("failed to compare, iOpCode:" << (long)iOpCode << " iMsgId:" << (long)iMsgId);
		iOpCode = -1;
	}
	return iOpCode;
}

// =========================================================================

//--- LDAPDeleteDAI -----------------------------------------------------
RegisterDataAccessImpl(LDAPDeleteDAI);

int LDAPDeleteDAI::DoLDAPRequest(Context &ctx, ParameterMapper *getter, LDAPConnection *lc, ROAnything query, int &iMsgId)
{
	StartTrace(LDAPDeleteDAI.DoLDAPRequest);
	TraceAny(query, "Query for delete request:");

	// get base
	String base( query["Base"].AsString() );

	// send request
	return ldap_delete_ext(lc->GetConnection(), base, NULL, NULL, &iMsgId );
}

// =========================================================================

//--- LDAPModifyDAI -----------------------------------------------------
RegisterDataAccessImpl(LDAPModifyDAI);

bool LDAPModifyDAI::DoGetQuery(ParameterMapper *getter, Context &ctx, Anything &query, LDAPErrorHandler &eh)
{
	StartTrace(LDAPModifyDAI.DoGetQuery);

	// check and add base using superclass
	if ( !LDAPAbstractDAI::DoGetQuery(getter, ctx, query, eh) ) {
		return false;
	}

	// get + check modifications
	Anything mods;
	getter->Get("LDAPMods", mods, ctx);
	TraceAny(mods, "Got Mods: ");

	// mods must be defined!
	if ( mods.IsNull() ) {
		eh.HandleError("No modifications defined in modify query");
		return false;
	}

	// check for legal mods and skip misspellings/unknown mods
	Anything checkedMods;
	int size( mods.GetSize() );
	String slotname;
	bool bCont(true);
	for (int i = 0; bCont && i < size; ++i) {
		slotname = mods.SlotName(i);
		// normalize
		slotname.ToLower();
		bool bRemoveNullValues( slotname.IsEqual("add") );
		if ( slotname.IsEqual("add") || slotname.IsEqual("replace") || slotname.IsEqual("delete") ) {
			// check given attributes not to contain unnamed slots
			// remove NullValue entries only when adding, otherwise they mark slots to remove
			if ( ( bCont = CheckAttributes(mods[i], bRemoveNullValues, eh) ) ) {
				// ok, accept
				checkedMods[slotname] = mods[i];
			}
		} else {
			// unknown mod, remove (i.e. skip) + drop debug message
			Trace("Unknown modification operation '" << slotname << "', skipped.")
		}
	}

	// checked mods must not be empty (i.e. at least one legal op)
	if ( !bCont || checkedMods.IsNull() || checkedMods.GetSize() <= 0 ) {
		eh.HandleError("No legal modification operations in modify query");
		return false;
	}

	TraceAny(checkedMods, "Checked Mods: ");
	query["Mods"] = checkedMods;

	return true;
}

int LDAPModifyDAI::IntPrepareLDAPMods(LDAPMod **ldapmods, int modcode, int &lModsCounter, ROAnything attrmods, bool bBinaryOperation)
{
	StartTrace(LDAPModifyDAI.IntPrepareLDAPMods);
	long lCurrentAttribute( 0L );
	for ( long sza = attrmods.GetSize(); lCurrentAttribute < sza; ++lCurrentAttribute, ++lModsCounter ) {
		char **vals( NULL );
		struct berval **bvals( NULL );
		LDAPMod *mod = (LDAPMod *)Storage::Current()->Calloc( 1, sizeof( LDAPMod ) );
		mod->mod_op = modcode;
		mod->mod_type = (char *)attrmods.SlotName(lCurrentAttribute);	// name of attribute

		SubTrace(TraceMallocFree, "preparing attribute [" << attrmods.SlotName(lCurrentAttribute) << "] (" << (long)lCurrentAttribute << ")");
		// attrmods[lCurrentAttribute] is affected attribute
		if ( attrmods[lCurrentAttribute].IsNull() ) {
			// remove attribute completely, let's hope this is
			// not an add request (would probably provoke an error)
			bvals = NULL;	// terminate list of values
			vals = NULL;	// terminate list of values
		} else {
			long vsize = attrmods[lCurrentAttribute].GetSize();	// # values for attribute
			if ( bBinaryOperation ) {
				bvals = (berval **)Storage::Current()->Calloc( ( vsize + 1 ), sizeof( berval * ) );
				for ( long k = 0; k < vsize; ++k ) {
					// do not use AsString() in the next assignment because we need a non-temporary reference
					//  until the ldap-call is done
					bvals[k] = (berval *)Storage::Current()->Calloc( 1, sizeof( berval ) );
					bvals[k]->bv_val = (char *)attrmods[lCurrentAttribute][k].AsCharPtr();
					bvals[k]->bv_len = attrmods[lCurrentAttribute][k].AsString().Length();
				}
				bvals[vsize] = NULL;	// terminate list of values
			} else {
				vals = (char **)Storage::Current()->Calloc( ( vsize + 1 ), sizeof( char * ) );
				for ( long k = 0; k < vsize; ++k ) {
					vals[k] = (char *)attrmods[lCurrentAttribute][k].AsCharPtr();
				}
				vals[vsize] = NULL;	// terminate list of values
			}
		}
		if ( bBinaryOperation ) {
			mod->mod_bvalues = bvals;
		} else {
			mod->mod_values = vals;
		}
		// add modification to list
		ldapmods[lModsCounter] = mod;
	}
	return lCurrentAttribute;
}

int LDAPModifyDAI::DoLDAPRequest(Context &ctx, ParameterMapper *getter, LDAPConnection *lc, ROAnything query, int &iMsgId)
{
	// ------------------------------------------------------------
	// hint: look at syntax of modify request in .h file, this will
	// help the understanding of this method a great deal
	// ------------------------------------------------------------

	StartTrace(LDAPModifyDAI.DoLDAPRequest);
	TraceAny(query, "Query for modify request:");

	String base = query["Base"].AsString();
	ROAnything mods = query["Mods"];

	// count total number of modifications (= sum over all attribute mods)
	long size = mods.GetSize();
	long totalmods = 0, i = 0;
	for ( i = 0; i < size; ++i ) {
		totalmods += mods[i].GetSize();
	}

	// init ldapmod structures for modify request
	LDAPMod **ldapmods = (LDAPMod **)Storage::Current()->Calloc( ( totalmods + 1 ), sizeof( LDAPMod * ) );
	ROAnything attrmods;
	String modname;
	int modcode, counter( 0 ), iTotalMods( 0 );
	bool bBinaryOperation( false );
	getter->Get("PlainBinaryValues", bBinaryOperation, ctx);

	// loop over specific modifications entries [add|replace|delete]
	for ( i = 0; i < size; ++i ) {
		modname = mods.SlotName(i);

		if ( modname.IsEqual("add") ) {
			modcode = LDAP_MOD_ADD;
		} else if ( modname.IsEqual("delete") ) {
			modcode = LDAP_MOD_DELETE;
		} else {
			modcode = LDAP_MOD_REPLACE;
		}

		if ( bBinaryOperation ) {
			modcode |= LDAP_MOD_BVALUES;
		}

		// every operation can affect a series of attributes, of which
		// each is an individual modification again
		// this means, we have a total of i*j modifications
		iTotalMods += IntPrepareLDAPMods(ldapmods, modcode, counter, mods[i], bBinaryOperation);
	}
	Assert( counter <= totalmods );
	ldapmods[totalmods] = NULL; // terminate list of mods

	int iRc( DoSpecificOperation(lc, base, ldapmods, iMsgId) );

	Trace("Freeing memory...");
	if ( ldapmods ) {
		for ( i = 0; i < totalmods; ++i ) {
			if ( bBinaryOperation ) {
				int mod_bvaluesIndex( 0 );
				while ( ldapmods[i]->mod_bvalues != NULL && ldapmods[i]->mod_bvalues[mod_bvaluesIndex] != NULL ) {
					SubTrace(TraceMallocFree, "freeing ldapmods[" << (long)i << "]->mod_bvalues[" << (long)mod_bvaluesIndex << "]");
					Storage::Current()->Free( ldapmods[i]->mod_bvalues[mod_bvaluesIndex] );
					++mod_bvaluesIndex;
				}
				SubTrace(TraceMallocFree, "freeing ldapmods[" << (long)i << "]->mod_bvalues");
				Storage::Current()->Free( ldapmods[i]->mod_bvalues );
			} else {
				SubTrace(TraceMallocFree, "freeing ldapmods[" << (long)i << "]->mod_values");
				Storage::Current()->Free( ldapmods[i]->mod_values );
			}
			SubTrace(TraceMallocFree, "freeing ldapmods[" << (long)i << "]");
			Storage::Current()->Free( ldapmods[i] );
		}
		SubTrace(TraceMallocFree, "freeing ldapmods");
		Storage::Current()->Free( ldapmods );
	}
	return iRc;
}

int LDAPModifyDAI::DoSpecificOperation(LDAPConnection *lc, String base, LDAPMod **ldapmods, int &iMsgId)
{
	StartTrace(LDAPModifyDAI.DoSpecificOperation);
	Trace("sending modify request");
	return ldap_modify_ext(lc->GetConnection(), base, ldapmods, NULL, NULL, &iMsgId);
}

// =========================================================================

//--- LDAPSearchDAI -----------------------------------------------------
RegisterDataAccessImpl(LDAPSearchDAI);

bool LDAPSearchDAI::DoGetQuery(ParameterMapper *getter, Context &ctx, Anything &query, LDAPErrorHandler &eh)
{
	StartTrace(LDAPSearchDAI.DoGetQuery);

	// call superclass first, base dn must be present
	if ( !LDAPAbstractDAI::DoGetQuery(getter, ctx, query, eh) ) {
		return false;
	}

	// find rest of bits & pieces
	String scopeStr( "base" ), filter;
	Anything attrs;
	long attrsOnly( -1 ), scope( LDAP_SCOPE_BASE ), sizeLimit( LDAP_NO_LIMIT ), timeout( -1 );

	getter->Get("LDAPScope", scopeStr, ctx);
	getter->Get("LDAPFilter", filter, ctx);
	getter->Get("LDAPAttrs", attrs, ctx);
	getter->Get("LDAPAttrsOnly", attrsOnly, ctx);
	getter->Get("LDAPSizeLimit", sizeLimit, ctx);
	getter->Get("LDAPTimeLimit", timeout, ctx);

	// glue rest of query together (and use defaults, where necessary)

	// scope (optional, default = base)
	scopeStr.ToLower();
	if (scopeStr.IsEqual("subtree")) {
		scope = LDAP_SCOPE_SUBTREE;
	} else if (scopeStr.IsEqual("onelevel")) {
		scope = LDAP_SCOPE_ONELEVEL;
	}
	query["Scope"] = scope;
	query["ScopeStringRepr"] = scopeStr;

	// filter (optional, default = (objectclass=*))
	query["Filter"] = filter.IsEqual("") ? String("(objectclass=*)") : filter;

	// attrsonly (optional, default = 0)
	query["AttrsOnly"] = (attrsOnly == -1) ? 0 : 1;

	// sizelimit (optional, default = LDAP_NO_LIMIT)
	query["SizeLimit"] = sizeLimit;

	// other timeout (optional, default = use timeout of session/connection)
	query["TimeLimit"] = timeout;

	// attributes (optional, default = NULL)
	query["Attrs"] = attrs;

	return true;
}

int LDAPSearchDAI::DoLDAPRequest(Context &ctx, ParameterMapper *getter, LDAPConnection *lc, ROAnything query, int &iMsgId)
{
	StartTrace(LDAPSearchDAI.DoLDAPRequest);
	TraceAny(query, "Query for search request:");

	// get arguments
	String base( query["Base"].AsString() );
	int scope( (int) query["Scope"].AsLong() );
	String filter( query["Filter"].AsString() );
	int attrsonly( (int) query["AttrsOnly"].AsLong() );
	int sizeLimit( (int) query["SizeLimit"].AsLong() );

	// timeout override
	int timeout( (int) query["TimeLimit"].AsLong() );
	timeval tv;
	timeval *tvp;
	if (timeout < 0) {
		tv.tv_sec =  lc->GetSearchTimeout();
		tv.tv_usec = 0;
	} else if (timeout > 0) {
		tv.tv_sec  = timeout;
		tv.tv_usec = 0;
	}
	tvp = (timeout == 0) ? NULL : &tv;

	// build attrs-array
	char **attrs( NULL );
	long size( query["Attrs"].IsNull() ? 0 : query["Attrs"].GetSize() );
	if (size > 0) {
		attrs = (char **) calloc(size + 1, sizeof(char *));
		if (!attrs) {
			static const char crashmsg[] = "FATAL: Attribute calloc failed in LDAPSearchDAI::DoLDAPRequest. Will crash.";
			SystemLog::WriteToStderr(crashmsg, sizeof(crashmsg));
			return -1;
		}
		for (long i = 0; i < size; ++i) {
			attrs[i] = (char *)query["Attrs"][i].AsCharPtr("");
		}
	}

	// send request + free allocated memory
	int ret( ldap_search_ext(lc->GetConnection(), base, scope, filter, attrs, attrsonly, NULL, NULL, tvp, sizeLimit, &iMsgId) );
	free(attrs);

	return ret;
}
