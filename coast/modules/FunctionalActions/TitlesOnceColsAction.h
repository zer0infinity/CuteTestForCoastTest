/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _TitlesOnceColsAction_H
#define _TitlesOnceColsAction_H

//---- Action include -------------------------------------------------
#include "config_FunctionalActions.h"
#include "Action.h"

class String;
class ROAnything;
class Context;

//---- TitlesOnceColsAction ----------------------------------------------------------
//: comment action
//
//	If the SybDBResultFormat for a CallDA is configured to TitlesOnce, the lookup for the
//  box value is quite complicated:
//			/Value	{
//				{ /Lookup { /Lookup { SelectBoxOption:
//						{ /Lookup DBGetVertraege.QueryTitles.vtr_nr }
//				} } }
//			}
//
//	To get more performance this action rearranges the boxcolumns and stores them in tmpStore:
// /tmp {
//  /<BoxColumns>Opt {
//        /"0" {
//          /Name "Vertrag"
//          /IntName "vtr_nr"				=> QueryTitle to use for this column; must be specified in the original BoxColumn
//          /...
//          /Value {						=> original BoxColumn.Value may be an empty string cause this optimized lookup will be used
//            /Lookup "SelectBoxOption:0"
//          }
//        }
//        /"1" {
//			/Name ...
//
//	A second slot will be created:
//  /<BoxColumns>Lookups {
//        /vtr_nr {
//          /Lookup "SelectBoxOption:0"
//        }
//	This slot can be used e.g. in ValueRenderer or SelectedRenderer to get the column value:
//	/Lookup <BoxColumns>Lookups.vtr_nr instead of /Lookup SelectBoxOption.vtr_nr
//
//	Structure of config:
//<PRE>	{
//		/BoxColumns		slotname to list with definition of columns
//		/BoxQuery		slotname to lookup the QueryTitles of the used SQL
//	}</PRE>
//
//Example:
//		{ /Call {
//			/TitlesOnceColsAction {
//				/BoxColumns		VerfuegbareUserColumns
//				/BoxQuery 		DBGetVpoUserAuswahl.QueryTitles
//			}
//		} }
//
//		let the MultifunctionListBoxRenderer use the optimized columns:
//		/ColList	VerfuegbareUserColumnsOpt

class EXPORTDECL_FUNCTIONALACTIONS TitlesOnceColsAction : public Action
{
public:
	//--- constructors
	TitlesOnceColsAction(const char *name);
	~TitlesOnceColsAction();

	//:DoSomething method for configured Actions
	//!param: transitionToken - (in/out) the event passed by the caller, can be modified.
	//!param: ctx - the context the action runs within.
	//!param: config - the configuration of the action.
	//!retv: true if the action run successfully, false if an error occurred.
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);
};

#endif
