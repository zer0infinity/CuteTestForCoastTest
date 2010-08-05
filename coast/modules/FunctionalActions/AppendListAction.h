/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AppendListAction_H
#define _AppendListAction_H

//---- baseclass include -------------------------------------------------
#include "config_FunctionalActions.h"
#include "CopyActions.h"

class Context;
//---- AppendListAction ----------------------------------------------------------
//! Appends data from a slot to an other slot 
/*!
\par Configuration
\code
{
	/Destination {
		/Store	Rendererspec	Optional
		/Slot	Rendererspec	Optional			# Slotname in Rolestore where the data are append to
													# If no slotname defined the copylist name will be used
	}
	/CopyList	{						            # List of /SourceLookupNames 	DestinationSlotnames
		/LookupName1	ResultSlotname1
		/LookupName2	ResultSlotname2
	}
}
\endcode

The AppendListAction loads its configuration from the context using the actions's name.
*/
class EXPORTDECL_FUNCTIONALACTIONS AppendListAction : public CopyAction
{
public:
	//--- constructors
	AppendListAction(const char *name);
	virtual ~AppendListAction();
protected:
	//: append the data from the context into rolestore's destination slot
	//!param: dest - defines the destination slot
	//!param: copyList - defines the data source and destination slotnames
	//!param: context - the context where the data is retrieved
	virtual void Copy(Anything &dest, const ROAnything &copyList, const ROAnything &config, Context &context);
};

#endif
