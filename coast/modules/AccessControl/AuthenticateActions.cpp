/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "AuthenticateActions.h"
#include "AccessManager.h"
#include "Renderer.h"
#include "Context.h"
#include "Tracer.h"

//---- AuthenticateWeakAction -----------------------------------------------
RegisterAction(AuthenticateWeakAction);

bool AuthenticateWeakAction::GetAccessManager(ROAnything config, Context &ctx, AccessManager *& am)
{
	StartTrace(AuthenticateWeakAction.GetAccessManager);

	String amName = Renderer::RenderToString(ctx, config["AccessManager"]);
	am = ( amName.IsEqual("") ?
		   AccessManagerModule::GetAccessManager() :
		   AccessManagerModule::GetAccessManager(amName)
		 );

	if (!am) {
		Trace("Failure, couldn't find " << (amName.IsEqual("") ? String("default") : amName) << " AccessManager.");
		return false;
	}
	return true;
}

bool AuthenticateWeakAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config)
{
	StartTrace(AuthenticateWeakAction.DoExecAction);

	AccessManager *am;
	if ( !GetAccessManager(config, ctx, am) ) {
		return false;
	}
	return am->AuthenticateWeak(
			   Renderer::RenderToString(ctx, config["UserId"]),
			   Renderer::RenderToString(ctx, config["Password"]),
			   transitionToken
		   );
}

//---- AuthenticateStrongAction ---------------------------------------------
RegisterAction(AuthenticateStrongAction);

bool AuthenticateStrongAction::DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config)
{
	// this is the new method that also gets a config ( similar to Renderer::RenderAll )
	// write the action code here - you don't have to override DoAction anymore
	StartTrace(AuthenticateStrongAction.DoExecAction);

	AccessManager *am;
	if ( !GetAccessManager(config, ctx, am) ) {
		return false;
	}

	Anything window = Renderer::RenderToStringWithDefault(ctx, config["Window"], "1");

	return am->AuthenticateStrong(
			   Renderer::RenderToString(ctx, config["UserId"]),
			   Renderer::RenderToString(ctx, config["Password"]),
			   Renderer::RenderToString(ctx, config["Code"]),
			   window.AsLong(),
			   transitionToken
		   );
}
