/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ACTION_H
#define _ACTION_H

// superclass include
#include "config_wdbase.h"
#include "IFAConfObject.h"
#include "WDModule.h"

class Context;

//---- ActionsModule -----------------------------------------------------------
class EXPORTDECL_WDBASE ActionsModule : public WDModule
{
public:
	ActionsModule(const char *);
	~ActionsModule();

	virtual bool Init(const ROAnything config);
	bool ResetFinis(const ROAnything );
	virtual bool Finis();
};

//---- Action ----------------------------------------------------------------------
//! MultiPurpose Do Something building block to script application behaviour
class EXPORTDECL_WDBASE Action : public NotCloned
{
public:
	//!standard named object constructor
	Action(const char *name);

	//!abstract method; DoSomething method for unconfigured Action
	//! \param transitionToken (in/out) the event passed by the caller, can be modified.
	//! \param c the context the action runs within.
	//! \return true if the action run successfully, false if an error occurred.
	virtual bool DoAction(String &transitionToken, Context &) {
		return false;
	}

	//!DoSomething method for configured Actions
	//! \param transitionToken (in/out) the event passed by the caller, can be modified.
	//! \param c the context the action runs within.
	//! \param config the configuration of the action.
	//! \return true if the action run successfully, false if an error occurred.
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config) {
		return DoAction(transitionToken, ctx);
	}

	//!Executes an Action defined by transitionToken
	//! \param transitionToken (in/out) the event passed by the caller, can be modified.
	//! \param c the context the script runs within.
	//! \return true if the script run successfully, false if an error occurred.
	static bool ExecAction(String &transitionToken, Context &c);

	//!Starts the Action script interpreter
	//! \param transitionToken (in/out) the event passed by the caller, can be modified.
	//! \param c the context the script runs within.
	//! \param config the action script
	//! \return true if the script run successfully, false if an error occurred.
	static bool ExecAction(String &transitionToken, Context &c, const ROAnything &config);

	RegCacheDef(Action);	// FindAction()

protected:
	//!Searches and executes an Action Component
	//! \param actionName the Name of the Action to be called.
	//! \param transitionToken (in/out) the event passed by the caller, can be modified by the action called
	//! \param c the context the action runs within.
	//! \param config the action's configuration
	//! \return true if the action run successfully, false if an error occurred.
	static bool CallAction(String &actionName, String &transitionToken,
						   Context &c, const ROAnything &config);
};

#define RegisterAction(name) RegisterObject(name, Action)

#endif
