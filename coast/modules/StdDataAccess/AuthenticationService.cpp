/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */
#include "AuthenticationService.h"
#include "SystemLog.h"
#include "SecurityModule.h"
#include "HTTPProcessor.h"	// for Coast::HTTP
#include "Renderer.h"
#include "AccessManager.h"
#include "Dbg.h"
#include "BasicAuthenticationData.h"
#include "HTTPConstants.h"
RegisterServiceHandler(AuthenticationService);

bool AuthenticationService::DoHandleService(std::ostream &os, Context &ctx) {
	StartTrace(AuthenticationService.DoHandleService);
	if (DoCheck(ctx)) {
		Trace( "DoCheck succeeded --> Calling ForwardToMainHandler" );
		ForwardToMainHandler(os, ctx);
		return true;
	} else {
		Trace( "DoCheck failed --> Calling Produce401Response" );
		Produce401Response(os, ctx);
	}
	return false;
}

bool AuthenticationService::DoCheck(Context &ctx) {
	StartTrace(AuthenticationService.DoCheck);
	bool ret = false;
	String name, pw;
	GetUserNameAndPw(ctx, name, pw);
	ret = AuthenticateUser(ctx, name, pw);
	Trace( "ret = [" << (long)ret << "]" );
	return ret;
}

void AuthenticationService::GetUserNameAndPw(Context &ctx, String &name, String &pw) {
	StartTrace(AuthenticationService.GetUserNameAndPw);
	BasicAuthenticationData baDat(ctx.Lookup("header.AUTHORIZATION", ""));
	pw = baDat.GetPassword();
	name = baDat.GetUserName();

}

bool AuthenticationService::AuthenticateUser(Context &ctx, String &name, String &pw) {
	StartTrace(AuthenticationService.AuthenticateUser);
	bool ret = false;
	if (name.Length()) {
		Trace("name given [" << name << "] pw [" << pw << "]");
		String strAccessMgrName = Lookup("AccessManager", "AccessManager");
		AccessManager *pMgr = AccessManagerModule::GetAccessManager(strAccessMgrName);
		Trace("requested AccessManager [" << strAccessMgrName << "]");
		if (pMgr) {
			//			Anything anyUserName;
			//			String strContextNameSlot = Lookup("AuthUserNameSlot", "AuthUserName");
			//			String strContextPasswordSlot = Lookup("AuthPasswordSlot", "AuthPassword");
			//			ctx.GetTmpStore()[strContextNameSlot] = name;
			//			ctx.GetTmpStore()[strContextPasswordSlot] = pw;
			String strNewRole;
			ret = pMgr->Validate(name);
			if (ret) {
				ret = pMgr->AuthenticateWeak(name, pw, strNewRole);
				Trace( "strNewRole = [" << strNewRole << "] ret = [" << (long) ret << "]" );
			}
			Trace("new Role [" << strNewRole << "] for [" << name << "]");
		} else {
			SYSWARNING(String("requested AccessManager [") << strAccessMgrName << "] not found!");
		}
	}
	Trace( "ret = [" << (long)ret << "]" );
	return ret;
}

void AuthenticationService::ForwardToMainHandler(std::ostream &os, Context &ctx) {
	StartTrace(AuthenticationService.ForwardToMainHandler);

	String service = Lookup("Service", "WebAppService");
	Trace("Forward to : " << service );
	ServiceHandler *sh = ServiceHandler::FindServiceHandler(service);
	if (sh) {
		sh->HandleService(os, ctx);
	} else {
		SYSWARNING(String("ServiceHandler [") << service << "] not found!");
	}
}

void AuthenticationService::Produce401Response(std::ostream &os, Context &ctx) {
	StartTrace(AuthenticationService.Produce401Response);
	Anything anyStatus;
	anyStatus[Coast::HTTP::_httpProtocolCodeSlotname] = 401L;
	Context::PushPopEntry<Anything> aEntry(ctx, "StatusInformation", anyStatus, "HTTPStatus");
	Coast::HTTP::RenderHTTPProtocolStatus(os, ctx);
	os << "WWW-Authenticate: Basic realm=\"";
	ROAnything realmConfig = ctx.Lookup("Realm");
	if (realmConfig.IsNull()) {
		os << "Coast";
	} else {
		Renderer::Render(os, ctx, realmConfig);
	}
	os << "\"" << ENDL << ENDL;

	ROAnything bodyConfig = ctx.Lookup("401MsgBody");
	if (bodyConfig.IsNull()) {
		os << "<html><h1>Please stay away</h1></html>";
	} else {
		Renderer::Render(os, ctx, bodyConfig);
	}
	ctx.GetTmpStore()["BasicAuthRetCode"] = 0; // false
}
