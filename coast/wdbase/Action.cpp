/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */
#include "Action.h"
#include "Registry.h"
#include "Page.h"
#include "Timers.h"
#include "AnyIterators.h"
#include "Policy.h"
RegisterModule(ActionsModule);

const char* Action::gpcCategory = "Action";
const char* Action::gpcConfigPath = "Actions";

bool ActionsModule::Init(const ROAnything config) {
	ROAnything roaActions;
	if (config.LookupPath(roaActions, Action::gpcConfigPath)) {
		AliasInstaller ai(Action::gpcCategory);
		return RegisterableObject::Install(roaActions, Action::gpcCategory, &ai);
	}
	return false;
}

bool ActionsModule::ResetFinis(const ROAnything) {
	AliasTerminator at(Action::gpcCategory);
	return RegisterableObject::ResetTerminate(Action::gpcCategory, &at);
}

bool ActionsModule::Finis() {
	return StdFinis(Action::gpcCategory, Action::gpcConfigPath);
}

bool Action::ExecAction(String &transitionToken, Context &c) {
	StartTrace1(Action.ExecAction, "transition <" << transitionToken << ">");
	return ExecAction(transitionToken, c, ROAnything());
}

bool Action::ExecAction(String &transitionToken, Context &c, const ROAnything &config) {
	StartTrace1(Action.ExecAction, "transition <" << transitionToken << "> + config");
	MethodTimer(Action.ExecAction, transitionToken, c);
	bool result(false);
	switch (config.GetType()) {
		case AnyArrayType: {
			TraceAny(config, "processing action script");
			String slotname(32L);
			ROAnything roaEntry;
			AnyExtensions::Iterator<ROAnything> scriptIterator(config);
			while (scriptIterator.Next(roaEntry)) {
				if (scriptIterator.SlotName(slotname)) {
					TraceAny(roaEntry, "trying slotname <" << slotname << "> as action with config");
					result = CallAction(slotname, transitionToken, c, roaEntry);
				} else {
					result = ExecAction(transitionToken, c, roaEntry);
				}
				if (!result) {
					return result;
				}
			}
		}
			break;
		case AnyCharPtrType:
			transitionToken = config.AsString();
			Trace("action script is simple string, implicitly changing transition to <" << transitionToken << ">");
			result = CallAction(transitionToken, transitionToken, c, ROAnything());
			break;
		case AnyNullType:
			TraceAny(config, "action script is neither any nor simple string, token remains <" << transitionToken << ">");
			result = CallAction(transitionToken, transitionToken, c, config);
			break;
		default:
			return false;
	}
	return result;
}

bool Action::CallAction(String &actionName, String &transitionToken, Context &c, const ROAnything &config) {
	StartTrace1(Action.CallAction, "transition <" << transitionToken << "> action <" << actionName << ">");
	if (actionName.Length() > 0) {
		TraceAny(config, "config given");
		Action *a(FindAction(actionName));
		if (a) {
			Trace(actionName << " found, executing it");
			return a->DoExecAction(transitionToken, c, config);
		} else if (!config.IsNull()) {
			return ExecAction(transitionToken, c, config);
		}
		return true;
	}
	return false;
}

class PreprocessAction: public Action {
public:
	PreprocessAction(const char *name) :
		Action(name) {
	}
	virtual bool DoAction(String &action, Context &);
};
RegisterAction(PreprocessAction);

bool PreprocessAction::DoAction(String &action, Context &c) {
	StatTrace(PreprocessAction.DoAction, "<" << action << ">", Coast::Storage::Current());
	Page *s(c.GetPage());
	if (s) {
		s->Preprocess(c);
	}
	return true;
}
