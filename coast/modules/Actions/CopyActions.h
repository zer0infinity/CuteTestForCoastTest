/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _CopyActions_H
#define _CopyActions_H

#include "Action.h"

//---- CopyAction ----------------------------------------------------------
//! <B>Abstract base action for copying data from a source into one destination slot.</B><BR>Configuration:
//! <PRE>
//! {
//!		/Destination {					destination slot must be given,even if empty
//!			/Store						optional, default TempStore [Role|Session|Request|TempStore], Store where the data is copied to
//!			/Slot		Rendererspec	optional, Slotname in Store where the data is copied to
//!										if omitted, DestinationSlotname will be used
//!		}
//!		/CopyList {						List of /SourceLookupName		DestinationSlotname
//!			/LookupName1	ResultSlotname1
//!			/LookupName2	ResultSlotname2
//!			...
//!		}
//!		/Delim   						optional, default ".", first char is taken as delimiter for named slots
//!		/IndexDelim						optional, default ":", first char is taken as delimiter for indexed slots
//! }
//! </PRE>
//! The concrete subclasses have to define the source and the destination as well as the copying mechanism.
//! The CopyAction loads its configuration from the context using the actions's name.
//! If the Slot \b /Store contains \b Role the destination store is the RoleStore in any other case the data go into
//! the TempStore.<BR>
//! The copying  works as follows:<BR>
//! With each the slotname of each entry a lookup in the source is performed.
//! The entry's value defines the slotname under which the lookuped result is put into the Destination's slot defined by Destination.
class CopyAction : public Action
{
public:
	//--- constructors
	CopyAction(const char *name);
	virtual ~CopyAction();
	//! copies the data according to configuration
	//! \param transitionToken (in/out) the event passed by the caller, can be modified.
	//! \param ctx the context the action runs within.
	//! \param config the configuration of the action.
	//! \return true if the action run successfully, false if an error occurred.
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);

protected:
	//! determines the destination slot and returns it as an Anything
	//! \param dest the Anything reference where the data gets copied into.
	//! \param destConfig an ROAnything that determines the destination slot
	//! \param context the context where the destination can be accessed
	virtual void GetDestination(Anything &dest, const ROAnything &destConfig, Context &context);
	//! copy the data from the context into rolestore's destination slot
	//! \param dest defines the destination slot
	//! \param copyList defines the data source and destination slotnames
	//! \param config the configuration of the action.
	//! \param context the context where the data is retrieved
	virtual void Copy(Anything &dest, const ROAnything &copyList, const ROAnything &config, Context &context) = 0;
};

//---- CopyContextAction ----------------------------------------------------------
//! <B>Copies data from context into slots in a store.</B><BR>Configuration:
//! <PRE>
//! {
//!		/Destination {					destination slot must be given,even if empty
//!			/Store						optional, default TempStore [Role|Session|Request|TempStore], Store where the data is copied to
//!			/Slot		Rendererspec	optional, Slotname in Store where the data is copied to
//!										if omitted, DestinationSlotname will be used
//!		}
//!		/CopyList {						List of /SourceLookupName		DestinationSlotname
//!			/LookupName1	ResultSlotname1
//!			/LookupName2	ResultSlotname2
//!			...
//!		}
//!		/Delim   						optional, default ".", first char is taken as delimiter for named slots
//!		/IndexDelim						optional, default ":", first char is taken as delimiter for indexed slots
//! }
//! </PRE>
//! The CopyContextAction loads its configuration from the context using the actions's name. The copying works as follows:<BR>
//! A lookup of LookupName in the context is performed.
//! ResultSlotname defines the slotname under which the lookuped result is put. It is put into the chosen
//! Store.Slot combination of Destination. If Destination.Slot is omitted the value is put into Store.ResultSlotname.
//! If Store is omitted the key/value pairs will be put into TempStore.
//! The LookupNames are supporting the depth search syntax; therefore /Top.Second.Third refers to an
//! Anything in the context looking like<PRE>
//! /Top {
//!		/Second {
//!			/Third	Value
//!		}
//!	}
//! </PRE>
class CopyContextAction : public CopyAction
{
public:
	//--- constructors
	CopyContextAction(const char *name);
	virtual ~CopyContextAction();
protected:
	//! copy the data from the context into rolestore's destination slot
	//! \param dest defines the destination slot
	//! \param copyList defines the data source and destination slotnames
	//! \param config the configuration of the action.
	//! \param context the context where the data is retrieved
	virtual void Copy(Anything &dest, const ROAnything &copyList, const ROAnything &config, Context &context);
};

//---- CopyQueryAction ----------------------------------------------------------
//! <B>Copies data from the Query into a store.</B><BR>Configuration:
//! <PRE>
//! {
//!		/Destination {					destination slot must be given,even if empty
//!			/Store						optional, default TempStore [Role|Session|Request|TempStore], Store where the data is copied to
//!			/Slot		Rendererspec	optional, Slotname in Store where the data is copied to
//!										if omitted, DestinationSlotname will be used
//!		}
//!		/CopyList {						List of /SourceLookupName		DestinationSlotname
//!			/LookupName1	ResultSlotname1
//!			/LookupName2	ResultSlotname2
//!			...
//!		}
//!		/Delim   						optional, default ".", first char is taken as delimiter for named slots
//!		/IndexDelim						optional, default ":", first char is taken as delimiter for indexed slots
//! }
//! </PRE>
//! The CopyQueryAction loads its configuration from the context using the actions's name. The copying works as follows:<BR>
//! A lookup of LookupName in the query is performed.
//! ResultSlotname defines the slotname under which the lookuped result is put. It is put into the chosen
//! Store.Slot combination of Destination. If Destination.Slot is omitted the value is put into Store.ResultSlotname.
//! If Store is omitted the key/value pairs will be put into TempStore.
//! <BR>The LookupNames are supporting the depth search syntax; therefore /fields.AField refers to "Value",
//! given the following Query<PRE>
//! /Query {
//!		/fields {
//!			/AField	Value
//!		}
//!	}
//! </PRE>
class CopyQueryAction : public CopyContextAction
{
public:
	//--- constructors
	CopyQueryAction(const char *name);
	virtual ~CopyQueryAction();

protected:
	//! copy the data from the query into rolestore's destination slot
	//! \param dest defines the destination slot
	//! \param copyList defines the data source and destination slotnames
	//! \param config the configuration of the action.
	//! \param context the context where the data is retrieved
	virtual void Copy(Anything &dest, const ROAnything &copyList, const ROAnything &config, Context &context);
};

//---- CopyQueryIfNotEmptyAction ----------------------------------------------------------
//! <B>Copies data from the Query into a store slot if they are not emtpy.</B><BR>Configuration:
//! <PRE>
//! {
//!		/Destination {					destination slot must be given,even if empty
//!			/Store						optional, default TempStore [Role|Session|Request|TempStore], Store where the data is copied to
//!			/Slot		Rendererspec	optional, Slotname in Store where the data is copied to
//!										if omitted, DestinationSlotname will be used
//!		}
//!		/CopyList {						List of /SourceLookupName		DestinationSlotname
//!			/LookupName1	ResultSlotname1
//!			/LookupName2	ResultSlotname2
//!			...
//!		}
//!		/Delim   						optional, default ".", first char is taken as delimiter for named slots
//!		/IndexDelim						optional, default ":", first char is taken as delimiter for indexed slots
//! }
//! </PRE>
//! The CopyQueryIfNotEmptyAction loads its configuration from the context using the actions's name.
//! The copying works as follows:<BR>
//! A lookup of LookupName in the query is performed. If the fields value is empty (i.e. == "") nothing gets copied.
//! ResultSlotname defines the slotname under which the lookuped result is put. It is put into the chosen
//! Store.Slot combination of Destination. If Destination.Slot is omitted the value is put into Store.ResultSlotname.
//! If Store is omitted the key/value pairs will be put into TempStore.
//! <BR>The LookupNames are supporting the depth search syntax; therefore /fields.AField refers to "Value",
//! given the following Query<PRE>
//! /Query {
//!		/fields {
//!			/AField	Value
//!		}
//! }
//! </PRE>
class CopyQueryIfNotEmptyAction : public CopyQueryAction
{
public:
	//--- constructors
	CopyQueryIfNotEmptyAction(const char *name);
	virtual ~CopyQueryIfNotEmptyAction();

	//! copies the data accoring to configuration if the source is not empty
	//! \param transitionToken (in/out) the event passed by the caller, can be modified.
	//! \param ctx the context the action runs within.
	//! \param config the configuration of the action.
	//! \return true if the action run successfully, false if an error occurred.
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);
};

#endif
