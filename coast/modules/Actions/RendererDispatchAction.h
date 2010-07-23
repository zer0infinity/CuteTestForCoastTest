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
//! <b>Executes the resulting String of a Renderer configuration as an Action</b>
/*! @section RendererDispatchActiondescription Description
 * This action is useful to let a Renderer script decide what gets executed next. Let the various Renderers define the flow of control.
 * The resulting token of the rendering process is used as transition token and as Context.Lookup to get the corresponding Action script.
 * If it is the empty token, the Action returns true.
 * @subsection RendererDispatchActionconfiguration RendererDispatchAction Configuration
 * There are different two ways to configure this Action, either specify the Renderer specification using the \c Renderer slot or just pass
 * it as a whole. See \ref RendererDispatchActionExamples for examples.
 * @par \c Renderer
 * \b optional, default is to not execute anything but return true\n
 * Pass in the Renderer specification to execute and use the resulting String as Action slot specification.
 * @subsection RendererDispatchActionExamples Examples
 * Full specification:
 * @code
/RendererDispatchAction { /Renderer { Renderer Spec To Execute and Dispatch } }
 * @endcode
 * Omitting \c Renderer slot:
 * @code
{ { Renderer Spec To Execute and Dispatch } }
 * @endcode
 * No need to render but only use simple String as next Action configuration:
 * @code
{ ActionToDispatch }
 * @endcode
*/
class EXPORTDECL_ACTIONS RendererDispatchAction : public Action
{
public:
	/*! @copydoc RegisterableObject.RegisterableObject() */
	RendererDispatchAction(const char *name);
	~RendererDispatchAction();

protected:
	/*! uses a Renderer to decide which action gets executed
	\param transitionToken (in/out) the event passed by the caller, can be modified.
	\param ctx the context the action runs within.
	\param config the configuration of the action.
	\return if the renderer produces a String that is found in the Context, the result of the Actionscript, false otherwise */
	virtual bool DoExecAction(String &transitionToken, Context &ctx, const ROAnything &config);
};

#endif
