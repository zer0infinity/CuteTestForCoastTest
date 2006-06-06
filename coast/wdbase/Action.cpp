/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "Action.h"

//--- standard modules used ----------------------------------------------------
#include "Registry.h"
#include "Page.h"
#include "Dbg.h"

//---- ActionsModule -----------------------------------------------------------
RegisterModule(ActionsModule);

ActionsModule::ActionsModule(const char *name) : WDModule(name)
{
}

ActionsModule::~ActionsModule()
{
}

bool ActionsModule::Init(const ROAnything config)
{
	if (config.IsDefined("Actions")) {
		AliasInstaller ai("Action");
		return RegisterableObject::Install(config["Actions"], "Action", &ai);
	}
	return false;
}

bool ActionsModule::ResetFinis(const ROAnything )
{
	AliasTerminator at("Action");
	return RegisterableObject::ResetTerminate("Action", &at);
}

bool ActionsModule::Finis()
{
	return StdFinis("Action", "Actions");
}

//---- Action ----------------------------------------------------------
//RegisterObject(Action, Action); no objects of the abstract type
Action::Action(const char *name) : NotCloned(name)
{
}

bool Action::ExecAction(String &transitionToken, Context &c)
{
	StartTrace1(Action.ExecAction, "<" << transitionToken << ">");
	return ExecAction(transitionToken, c, ROAnything());
}

bool Action::ExecAction(String &transitionToken, Context &c, const ROAnything &config)
{
	StartTrace1(Action.ExecAction, "<" << transitionToken << ">");
	bool result(false);
	switch (config.GetType()) {
		case AnyArrayType: {
			long sz = config.GetSize();
			for (long i = 0; i < sz; ++i) {
				String slotname = config.SlotName(i);
				if (slotname.Length() > 0) {
					result = CallAction(slotname, transitionToken, c, config[i]);
				} else {
					result = ExecAction(transitionToken, c, config[i]);
				}
				if ( !result ) {
					return result;
				}
			}
		}
		break;

		case AnyCharPtrType:
			transitionToken = config.AsString();
			result = CallAction(transitionToken, transitionToken, c, ROAnything());
			break;

		default:
			result = CallAction(transitionToken, transitionToken, c, config);
	}
	// it is not a valid action
	return result;
}

bool Action::CallAction(String &actionName, String &transitionToken, Context &c, const ROAnything &config)
{
	StartTrace1(Action.CallAction, "<" << actionName << "/" << transitionToken << ">");

	if (actionName.Length() > 0) {
		Action *a = FindAction(actionName);
		if (a) {
			return a->DoExecAction(transitionToken, c, config);
		} else if (!config.IsNull()) {
			return ExecAction(transitionToken, c, config);
		}
		return true;
	}
	return false;
}

RegCacheImpl(Action);	// FindAction()

//---- PreprocessAction ----------------------------------------------------------------------
class EXPORTDECL_WDBASE PreprocessAction : public Action
{
public:
	PreprocessAction(const char *name);

	virtual bool DoAction(String &action, Context &);
};

RegisterAction(PreprocessAction);
PreprocessAction::PreprocessAction(const char *name) : Action(name)
{
}

bool PreprocessAction::DoAction(String &action, Context &c)
{
	StartTrace1(PreprocessAction.DoAction, "<" << action << ">");
	Page *s = c.GetPage();

	if ( s ) {
		s->Preprocess(c);
	}
	return true;
}
