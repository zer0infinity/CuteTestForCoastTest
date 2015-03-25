/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ACTION_H
#define _ACTION_H

#include "WDModule.h"
#include "Registry.h"
#include "Tracer.h"

class Context;

class ActionsModule: public WDModule {
public:
	ActionsModule(const char *name) :
		WDModule(name) {
	}
	virtual bool Init(const ROAnything config);
	bool ResetFinis(const ROAnything);
	virtual bool Finis();
};

//! MultiPurpose Do Something building block to script application behaviour
class Action: public NotCloned {
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	//RegisterObject(Action, Action); no objects of the abstract type
	Action(const char *name) :
		NotCloned(name) {
	}
	//!Executes an Action defined by transitionToken
	//! \param transitionToken (in/out) the event passed by the caller, can be modified.
	//! \param c the context the script runs within.
	//! \return true if the script run successfully, false if an error occurred.
	static bool ExecAction(String & transitionToken, Context & c);
	//!Starts the Action script interpreter
	//! \param transitionToken (in/out) the event passed by the caller, can be modified.
	//! \param c the context the script runs within.
	//! \param config the action script
	//! \return true if the script run successfully, false if an error occurred.
	static bool ExecAction(String & transitionToken, Context & c, const ROAnything & config);
	RegCacheDef(Action);

	static const char* gpcCategory;
	static const char* gpcConfigPath;

protected:
	//!abstract method; DoSomething method for unconfigured Action
	//! \param transitionToken (in/out) the event passed by the caller, can be modified.
	//! \param ctx the context the action runs within.
	//! \return true if the action run successfully
	//! \return false if an error occurred.
	virtual bool DoAction(String & transitionToken, Context & ctx) {
		return false;
	}

	/*! Main Action hook; overwrite this method in subclasses
	 * \param transitionToken (in/out) Event token passed by the caller, can be modified.
	 * \param ctx Context the action will be executed with
	 * \param config Configuration of the action.
	 * \return true if the action run successfully
	 * \return false if an error occurred and further processing should be stopped */
	virtual bool DoExecAction(String & transitionToken, Context & ctx, const ROAnything & config) {
		return DoAction(transitionToken, ctx);
	}

	//!Searches and executes an Action Component
	//! \param actionName the Name of the Action to be called.
	//! \param transitionToken (in/out) the event passed by the caller, can be modified by the action called
	//! \param c the context the action runs within.
	//! \param config the action's configuration
	//! \return true if the action run successfully, false if an error occurred.
	static bool CallAction(String &actionName, String &transitionToken, Context &c, const ROAnything &config);
};

#define RegisterAction(name) RegisterObject(name, Action)

#endif
