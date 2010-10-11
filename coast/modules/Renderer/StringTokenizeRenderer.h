/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _StringTokenizeRenderer_H
#define _StringTokenizeRenderer_H

//---- Renderer include -------------------------------------------------
#include "config_renderer.h"
#include "Renderer.h"

//---- StringTokenizeRenderer ----------------------------------------------------------
//! Tokenizes a given string by a given character and prints out whatever wanted
/*!
\par Configuration
\code
{
	/String			Rendererspec	mandatory, string to be tokenized according to given token
	/Token			Rendererspec	mandatory, token to split the given string into pieces
	/RenderToken	Rendererspec	optional, default 1, single index or list of indizes to be rendered beginning with 1
									- use negative values to indicate reverse order indizes, eg. -1 is the last token
									- a token list can be specified but must be separated by ';'
									- to specify a token range a '-' must be given between the two numbers
									  optionally the second number can be omitted to print all tokens up to the end
	/OutputFiller	Rendererspec	optional, default is empty string, specify separator to insert into rendered output
									when RenderToken is a list of tokens
	/Default		Rendererspec	optional, default is empty string, specify what should be returned when the token was not found
}
\endcode

Examples in the TestCases
*/
class EXPORTDECL_RENDERER StringTokenizeRenderer : public Renderer
{
public:
	//--- constructors
	StringTokenizeRenderer(const char *name);
	~StringTokenizeRenderer();

	virtual void RenderAll(std::ostream &reply, Context &c, const ROAnything &config);

protected:
	bool SplitStringIntoTokens(Context &ctx, const ROAnything &config, Anything &anyTokens);
	void BuildTokenList(Context &ctx, const ROAnything &config, Anything &anyTokens, Anything &anyOutputTokenList);
	void BuildOutputString(Context &ctx, const ROAnything &config, String &strOut, Anything &anyTokensToOutput, Anything &anyTokens);
};

#endif
