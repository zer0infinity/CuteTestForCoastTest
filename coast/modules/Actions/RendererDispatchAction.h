/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _RendererDispatchAction_H
#define _RendererDispatchAction_H

//---- Action include -------------------------------------------------
#include "config_actions.h"
#include "Action.h"

//---- RendererDispatchAction ----------------------------------------------------------
//! <B>Uses a Renderer to decide which action gets executed</B><BR>Configuration:
//! <PRE>
//! {
//!		/Renderer    RendererSpec         # resulting a String is used as ActionSpec
//! }
//! </PRE>
//! or just
//! <PRE>
//! { { RendererSpec } }
//! </PRE>
//! and if the RendererSpec is just a String
//! <PRE>
//! { NameOfNextAction }
//! </PRE>
//! Use this action to profit of the various Renderers to make decisions in the flow of control.
class EXPORTDECL_ACTIONS RendererDispatchAction : public Action
{
public:
	//--- constructors
	RendererDispatchAction(const char *name);
	~RendererDispatchAction();

	//!uses a Renderer to decide which action gets executed
	//! \param transitionToken (in/out) the event passed by the caller, can be modified.
	//! \param ctx the context the action runs within.
	//! \param config the configuration of the action.
	//! \return if the renderer produces a String that is found in the Context, the result of the Actionscript, false otherwise
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);
};

#endif
