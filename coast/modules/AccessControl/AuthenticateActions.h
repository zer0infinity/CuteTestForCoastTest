/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AuthenticateActions_H
#define _AuthenticateActions_H

//---- Action include -------------------------------------------------
#include "config_AccessControl.h"
#include "Action.h"

class AccessManager;

//---- AuthenticateWeakAction -----------------------------------------
//! Authenticates an user weakly with uid and password
/*!
\par Configuration
\code
{
	/UserId			rendererspec	mandatory, uid of user to authenticate
	/Password		rendererspec	mandatory, password of user to authenticate
	/AccessManager	String			optional, name of AM to use (default AM if not given)
}
\endcode

Returns the result of call to the given access manager's AuthenticateWeak
method with the given arguments. The transition token is changed to the
role specified for successful/failing authentication (see config
of AccessManager for those values).
*/
class EXPORTDECL_ACCESSCONTROL AuthenticateWeakAction : public Action
{
public:
	//--- constructors
	/*! \param name defines the name of the action */
	AuthenticateWeakAction(const char *name) : Action(name) {}
	~AuthenticateWeakAction() {}

	/*! Returns the result of a AccessManager.AuthenticateWeak
		method with arguments given in config.
		\param transitionToken resulting role according to auth. success/failure
		\param ctx the context the action runs within.
		\param config the configuration of the action.
		\return true if authentication was successful
	*/
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);

protected:

	bool GetAccessManager(ROAnything config, Context &ctx, AccessManager *&am);
};

//---- AuthenticateStrongAction -----------------------------------------
//! Authenticates an user weakly with uid and password
/*!
\par Configuration
\code
{
	/UserId			rendererspec	mandatory, uid of user to authenticate
	/Password		rendererspec	mandatory, password of user to authenticate
	/Code			rendererspec	mandatory, code/otp to use for strong auth.
	/Window			rendererspec	optional, size of accept window (default = 1)
	/AccessManager	String			optional, name of AM to use (default AM if not given)
}
\endcode

Returns the result of call to the given access manager's AuthenticateStrong
method with the given arguments. The transition token is changed to the
role specified for successful/failing authentication (see config
of AccessManager for those values).
*/
class EXPORTDECL_ACCESSCONTROL AuthenticateStrongAction : public AuthenticateWeakAction
{
public:
	//--- constructors
	/*! \param name defines the name of the action */
	AuthenticateStrongAction(const char *name) : AuthenticateWeakAction(name) {}
	~AuthenticateStrongAction() {}

	/*! Returns the result of a AccessManager.AuthenticateStrong
		method with arguments given in config.
		\param transitionToken resulting role according to auth. success/failure
		\param ctx the context the action runs within.
		\param config the configuration of the action.
		\return true if authentication was successful
	*/
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);
};

#endif
