/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- c-library modules used ---------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "Dbg.h"

//--- interface include --------------------------------------------------------
#include "EmailAddressCheckAction.h"

//---- EmailAddressCheckAction ---------------------------------------------------------------
RegisterAction(EmailAddressCheckAction);

EmailAddressCheckAction::EmailAddressCheckAction(const char *name) : NotEmptyCheckAction(name) { }

EmailAddressCheckAction::~EmailAddressCheckAction() { }

bool EmailAddressCheckAction::DoCheck(String &fieldToCheck, Context &ctx, const ROAnything &config)
{
	StartTrace1(EmailAddressCheckAction.DoCheck, "String to check <" << fieldToCheck << ">");
	long pos = fieldToCheck.StrChr('@');
	Trace("At pos: " << pos);
	if (pos < 1) {
		return false;
	}

	// expect a dot after the @, but not at the first position (@. is bad)
	long dotPos = fieldToCheck.StrChr('.', pos);
	Trace("Dot pos: " << dotPos);
	if (dotPos - pos <= 1) {
		return false;
	}

	// blanks only trailing
	pos = fieldToCheck.StrChr(' ');
	Trace("Blank pos: " << pos);
	if (pos >= 0) {
		long sz = fieldToCheck.Length();
		while (++pos < sz) {
			if (fieldToCheck[pos] != ' ') {
				return false;
			}
		}
	}

	return true;
}
