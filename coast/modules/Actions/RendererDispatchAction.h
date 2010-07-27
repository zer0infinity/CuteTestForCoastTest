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
//! <b>Executes the result of a Context.Lookup() on the rendered String as an Action</b>
/*! @section RendererDispatchActionDescription Description
 * This action is useful to let a Renderer script decide what to execute next. Let the various Renderers define the flow of control.
 * The resulting token of the rendering process is used as Context.Lookup() to get the corresponding Action script and
 * execute it with the initial transition token. If the Context.Lookup() fails, the transition token will be set to the resulting
 * token and an Action.ExecAction() call wil be issued with an empty script.
 * If the resulting token is empty, the Action returns true.
 * @subsection RendererDispatchActionConfiguration Configuration
 * There are different ways to configure this Action, either specify the Renderer specification using the \c Renderer slot or just pass
 * it as a whole. See \ref RendererDispatchActionExamples for examples.
 * @par \c Renderer
 * \b optional, default is to not execute anything but return true\n
 * Pass in the Renderer specification to use as Context.Lookup() to get the corresponding Action script for the next Action.ExecAction() call.
 * @subsection RendererDispatchActionExamples Examples
 * Full specification:
 * @code
/RendererDispatchAction { /Renderer { Renderer Spec To Execute and Dispatch } }
 * @endcode
 * Omitting \c Renderer slot:
 * @code
{ Renderer Spec To Execute and Dispatch }
 * @endcode
 * No need to render but only use simple String as next Action configuration:
 * @code
{ ActionToDispatch }
 * @endcode
*/
class EXPORTDECL_ACTIONS RendererDispatchAction : public Action
{
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	RendererDispatchAction(const char *name) : Action(name) {}

protected:
	/*! Uses a Renderer script to decide what to execute next
	 * @copydetails Action::DoExecAction(String &, Context &, const ROAnything &)
	 * @return The result of the executed Actionscript if the renderer produces a String that is found in the Context
	 * @return true otherwise */
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);
};

#endif
