/*
 * Copyright (c) 2006, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _BuildAnyFromSeparatedListAction_H
#define _BuildAnyFromSeparatedListAction_H

//---- Action include -------------------------------------------------
#include "Action.h"

//---- BuildAnyFromSeparatedListAction ----------------------------------------------------------
//! <b>Stores a generated Anything into the defined Destination</b>
/*!
<b>Configuration:</b>
<PRE>
{
	/Destination {
		/Store		String			optional, default Role ["Role"|"TempStore"], the Store where the data are stored in
		/Slot		Rendererspec	mandatory, producing the Slotname in Destination where the data is stored to
    }
	/Text	 			Rendererspec	mandatory
	/RecordSeparator	Rendererspec	optional, default "\r\n", if defined  a record separated anything will be generated
	/FieldSeparator		Rendererspec	optional, no default, if defined  a field separated anything will be generated
	/useSlotName		character		optional, no default, "1" = True, generate numbered slotnames
}</PRE>

Example:
<PRE>
{ /BuildAnyFromSeparatedListAction {
	/Destination {
		/Slot		Testslot
    }
	/Text	 			"1234;abcde;fghik\r\n5678;lmnop;qrstuv"
	/RecordSeparator	"\r\n"
	/FieldSeparator		";"
	/useSlotName		1
} } </PRE>

Generates:
/Testslot {
	/"0"  {
		/0	"1234"
		/1	"abcde"
		/2	"fghik"
	}
	/"1"  {
		/0	"5678"
		/1	"lmnop"
		/2	"qrstuv"
	}
}</PRE>
*/
class BuildAnyFromSeparatedListAction : public Action
{
public:
	//--- constructors
	/*! \param name defines the name of the action */
	BuildAnyFromSeparatedListAction(const char *name);
	~BuildAnyFromSeparatedListAction();

	/*! DoSomething method for configured Actions
		\param transitionToken (in/out) the event passed by the caller, can be modified.
		\param ctx the context the action runs within.
		\param config the configuration of the action.
		\return true if the action run successfully, false if an error occurred.
	*/
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);

private:
	bool   DoTokenize(Anything &anyTokens, String &text, String &strSep );
	bool   DoBuildHeader(Anything &anyHeaderTok, Anything &anyHeader, String &sHeaderSeparator, bool &bBuildHeader);
	bool   ReadConfig(String &text, String &recordSeparator, String &fieldSeparator, bool &bFieldSep,
					  String &useSlotName, bool &bUseSlot, long &lHeaderRows, String &sHeaderSeparator, Context &ctx, const ROAnything &config);
	bool   ReadValue(String &retValue, const ROAnything &store, const String &location, Context &ctx);
	String TrimEndingSpaces(String &value);
	bool   BuildAnything (String &text, String &recordSeparator, String &fieldSeparator, bool &bFieldSep,
						  bool &bUseSlot, long &lHeaderRows, String &sHeaderSeparator, Anything &tmp );
};

#endif
