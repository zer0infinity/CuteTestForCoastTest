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
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

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

	bool ret = false;
	if ( (ret = DoExec(ctx, getter, putter, eh)) == true ) {
		// LDAP transaction completed successfully
		return ret;
	}
	if ( eh.GetRetryState() == LDAPErrorHandler::eRetry ) {
		Trace("Will try a rebind (LDAP might have been restarted.) for DataAccess: " << fName);
		eh.HandleSessionError((LDAP *) NULL, "Will try a rebind (LDAP might have been restarted.)");
		eh.CleanUp();
		eh.SetRetryState(LDAPErrorHandler::eRetryAlreadyDone);
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
	String server, bindName, bindPw;
	long port;
	cp["Server"] = server = ctx.Lookup("LDAPServer", "localhost");
	cp["Port"] = port = ctx.Lookup("LDAPPort", 389L);
	cp["Timeout"] = ctx.Lookup("LDAPTimeout", 10L);
	cp["ConnectionTimeout"] = ctx.Lookup("LDAPConnectionTimeout", 10L);
	cp["BindName"] = bindName = ctx.Lookup("LDAPBindName", "");
	cp["BindPW"] = bindPw = ctx.Lookup("LDAPBindPW", "");
	cp["MapUTF8"] = !ctx.Lookup("NoHTMLCharMapping", 0L);
	cp["PooledConnections"] = ctx.Lookup("LDAPPooledConnections", 0L);
	cp["MaxConnections"] = ctx.Lookup("LDAPMaxConnections", 0L);
	cp["TryAutoRebind"] = ctx.Lookup("LDAPTryAutoRebind", 0L);
	cp["RebindTimeout"] = ctx.Lookup("LDAPRebindTimeout", 3600L);

	// store connection params in ctx (lookup for error-handling)
	eh.PutConnectionParams(cp);

	LDAPConnection *lc = LDAPConnectionFactory(cp);
	if ( lc == (LDAPConnection *) NULL ) {
		eh.HandleError("Fatal LDAP error. Could not create LDAP connection on heap.");
		return false;
	}

	if ( !(lc->Connect(cp, eh)) ) {
		ReleaseHandleInfo(ctx, lc);
		return false;
	}
	Trace("Connection to server established: " << bindName << "@" << server << ":" << port);
	// 4. request ldap operation, then wait for result (loop)
	int msgId = DoLDAPRequest(lc, query);
	Trace("LDAP request sent");

	Anything result;
	if ( !(lc->WaitForResult(msgId, result, eh)) ) {
		TryReleaseHandleInfo(ctx, lc, eh);
		return false;
	}
	Trace("Received LDAP result(s)");
	// 5. check result
	if ( !DoCheckResult(result, eh) ) {
		ReleaseHandleInfo(ctx, lc);
		return false;
	}
	Trace("Result is ok");
	// 6. store result
	String key("LDAPResult.");
	key << fName;
	bool storedOk = putter->Put(key, result, ctx);
	Trace("Result stored successfully : " << storedOk);
	ReleaseHandleInfo(ctx, lc);
	return true;
}

bool LDAPAbstractDAI::DoGetQuery(ParameterMapper *getter, Context &ctx, Anything &query, LDAPErrorHandler eh)
{
	StartTrace(LDAPAbstractDAI.DoGetQuery);

	// search, check + add ldap base DN
	String base;
	getter->Get("LDAPBase", base, ctx);

	if (base.IsEqual("")) {
		eh.HandleError("Illegal Base DN in query (empty or not defined)");
		return false;
	}

	query["Base"] = base;
	return true;
}

// =========================================================================

//--- LDAPAddDAI -----------------------------------------------------
RegisterDataAccessImpl(LDAPAddDAI);

bool LDAPAddDAI::DoGetQuery(ParameterMapper *getter, Context &ctx, Anything &query, LDAPErrorHandler eh)
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

	// attrs must contain attr/value
	long nofAttrs = attrs.GetSize();
	for (long i = 0; i < nofAttrs; i++) {
		if ( String(attrs.SlotName(i)).Length() <= 0 ) {
			String msg;
			msg << "All values listed under 'Attrs' must have an associated attribute name.";
			eh.HandleError(msg);
			return false;
		}
	}
	query["Attrs"] = attrs;

	return true;
}

int LDAPAddDAI::DoLDAPRequest(LDAPConnection *lc, ROAnything query)
{
	StartTrace(LDAPAddDAI.DoLDAPRequest);
	TraceAny(query, "Query for add request:");

	String base = query["Base"].AsString();
	ROAnything attrs = query["Attrs"];

	// init ldapmod structure for add request
	long size = attrs.GetSize();
	long vsize;
	LDAPMod **ldapmods = new LDAPMod*[ size + 1 ];
	LDAPMod *mod;
	char **vals = NULL;

	for ( long i = 0; i < size; i++ ) {
		mod = new LDAPMod;
		mod->mod_op = LDAP_MOD_ADD;
		mod->mod_type = (char *)attrs.SlotName(i);

		vsize = attrs[i].GetSize();
		vals = new char*[ vsize + 1 ];
		for ( long j = 0; j < vsize; j++ ) {
			vals[j] = (char *)attrs[i][j].AsCharPtr();
		}
		vals[vsize] = NULL;	// terminate
		mod->mod_values = vals;

		ldapmods[i] = mod;
	}
	ldapmods[size] = NULL; // terminate

	// send request
	Trace("Sending request...");
	int msgId = ldap_add(lc->GetConnection(), base, ldapmods);

	// free used memory
	Trace("Freeing memory...");
	if (ldapmods) {
		for ( long k = 0; k < size; k++ ) {
			delete [] ldapmods[k]->mod_values;
			delete ldapmods[k];
		}
		delete[] ldapmods;
	}

	return msgId;
}

// =========================================================================

//--- LDAPCompareDAI -----------------------------------------------------
RegisterDataAccessImpl(LDAPCompareDAI);

bool LDAPCompareDAI::DoGetQuery(ParameterMapper *getter, Context &ctx, Anything &query, LDAPErrorHandler eh)
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

int LDAPCompareDAI::DoLDAPRequest(LDAPConnection *lc, ROAnything query)
{
	StartTrace(LDAPCompareDAI.DoLDAPRequest);
	TraceAny(query, "Query for compare request:");

	// get arguments
	String base = query["Base"].AsString();
	String attrName = query["AttrName"].AsString();
	String attrVal = query["AttrValue"].AsString();

	// send request
	return ldap_compare(lc->GetConnection(), base, attrName, attrVal);
}

// =========================================================================

//--- LDAPDeleteDAI -----------------------------------------------------
RegisterDataAccessImpl(LDAPDeleteDAI);

int LDAPDeleteDAI::DoLDAPRequest(LDAPConnection *lc, ROAnything query)
{
	StartTrace(LDAPDeleteDAI.DoLDAPRequest);
	TraceAny(query, "Query for delete request:");

	// get base
	String base = query["Base"].AsString();

	// send request
	return ldap_delete(lc->GetConnection(), base);
}

// =========================================================================

//--- LDAPModifyDAI -----------------------------------------------------
RegisterDataAccessImpl(LDAPModifyDAI);

bool LDAPModifyDAI::DoGetQuery(ParameterMapper *getter, Context &ctx, Anything &query, LDAPErrorHandler eh)
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
	int size = mods.GetSize();
	String slotname;
	for (int i = 0; i < size; i++) {
		slotname = mods.SlotName(i);
		slotname.ToLower();				// normalize
		if (   slotname.IsEqual("add")
			   || slotname.IsEqual("delete")
			   || slotname.IsEqual("replace") ) {
			// ok, accept
			checkedMods[slotname] = mods[i];
		} else {
			// unknown mod, remove (i.e. skip) + drop debug message
			Trace("Unknown modification operation '" << slotname << "', skipped.")
		}
	}

	// checked mods must not be empty (i.e. at least one legal op)
	if ( checkedMods.IsNull() || checkedMods.GetSize() <= 0 ) {
		eh.HandleError("No legal modification operations in modify query");
		return false;
	}

	TraceAny(checkedMods, "Checked Mods: ");
	query["Mods"] = checkedMods;

	return true;
}

int LDAPModifyDAI::DoLDAPRequest(LDAPConnection *lc, ROAnything query)
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
	for ( i = 0; i < size; i++ ) {
		totalmods += mods[i].GetSize();
	}

	// init ldapmod structures for modify request
	LDAPMod **ldapmods = new LDAPMod*[ totalmods + 1 ];
	long vsize;
	char **vals;
	ROAnything attrmods;
	LDAPMod *mod;
	String modname;
	int modcode;
	int counter = 0;

	for ( i = 0; i < size; i++ ) {
		modname = mods.SlotName(i);

		if ( modname.IsEqual("add") ) {
			modcode = LDAP_MOD_ADD;
		} else if ( modname.IsEqual("delete") ) {
			modcode = LDAP_MOD_DELETE;
		} else {
			modcode = LDAP_MOD_REPLACE;
		}

		// every operation can affect a series of attributes, of which
		// each is an individual modification again
		// this means, we have a total of i*j modifications
		attrmods = mods[i];
		for ( long j = 0; j < attrmods.GetSize(); j++ ) {
			mod = new LDAPMod;
			mod->mod_op = modcode;
			mod->mod_type = (char *)attrmods.SlotName(j);	// name of attribute

			// attrmods[j] is affected attribute
			if ( attrmods[j].IsNull() ) {
				// remove attribute completely, let's hope this is
				// not an add request (would probably provoke an error)
				vals = NULL;
			} else {
				vsize = attrmods[j].GetSize();	// # values for attribute
				vals = new char*[ vsize + 1 ];
				for ( long k = 0; k < vsize; k++ ) {
					vals[k] = (char *)attrmods[j][k].AsCharPtr();
				}
				vals[vsize] = NULL;	// terminate list of values
			}
			mod->mod_values = vals;

			// add modification to list
			ldapmods[counter++] = mod;
		}
	}
	ldapmods[totalmods] = NULL; // terminate list of mods

	// send request
	int msgId = ldap_modify(lc->GetConnection(), base, ldapmods);

	// free used memory
	if (ldapmods) {
		for ( i = 0; i < totalmods; i++ ) {
			delete [] ldapmods[i]->mod_values;
			delete ldapmods[i];
		}
		delete[] ldapmods;
	}

	return msgId;
}

// =========================================================================

//--- standard modules used ----------------------------------------------------
#include "SysLog.h"

//--- LDAPSearchDAI -----------------------------------------------------
RegisterDataAccessImpl(LDAPSearchDAI);

bool LDAPSearchDAI::DoGetQuery(ParameterMapper *getter, Context &ctx, Anything &query, LDAPErrorHandler eh)
{
	StartTrace(LDAPSearchDAI.DoGetQuery);

	// call superclass first, base dn must be present
	if ( !LDAPAbstractDAI::DoGetQuery(getter, ctx, query, eh) ) {
		return false;
	}

	// find rest of bits & pieces
	String scopeStr = "base", filter;
	Anything attrs;
	long attrsOnly = -1, scope, sizeLimit = LDAP_NO_LIMIT, timeout = -1;

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
	} else {
		scope = LDAP_SCOPE_BASE;    // default
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

int LDAPSearchDAI::DoLDAPRequest(LDAPConnection *lc, ROAnything query)
{
	StartTrace(LDAPSearchDAI.DoLDAPRequest);
	TraceAny(query, "Query for search request:");

	// get arguments
	String base = query["Base"].AsString();
	int scope = (int) query["Scope"].AsLong();
	String filter = query["Filter"].AsString();
	int attrsonly = (int) query["AttrsOnly"].AsLong();
	int sizeLimit = (int) query["SizeLimit"].AsLong();

	// timeout override
	int timeout = (int) query["TimeLimit"].AsLong();
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
	char **attrs = NULL;
	long size = query["Attrs"].IsNull() ? 0 : query["Attrs"].GetSize();
	if (size > 0) {
		attrs = (char **) calloc(size + 1, sizeof(char *));
		if (!attrs) {
			static const char crashmsg[] = "FATAL: Attribute calloc failed in LDAPSearchDAI::DoLDAPRequest. Will crash.";
			SysLog::WriteToStderr(crashmsg, sizeof(crashmsg));
			return -1;
		}
		for (long i = 0; i < size; i++) {
			attrs[i] = (char *)query["Attrs"][i].AsCharPtr("");
		}
	}

	// send request + free allocated memory
	int msgId;
	int ret = ldap_search_ext(lc->GetConnection(), base, scope, filter, attrs, attrsonly, NULL, NULL, tvp, sizeLimit, &msgId);
	free(attrs);

	return (ret == LDAP_SUCCESS ? msgId : -1);
}
