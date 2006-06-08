/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _TemplateParserTest_H
#define _TemplateParserTest_H

//---- baseclass include -------------------------------------------------
#include "WDBaseTestPolicies.h"

//---- TemplateParserTest ----------------------------------------------------------
//! <B>really brief class description</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
class TemplateParserTest : public TestFramework::TestCaseWithGlobalConfigDllAndModuleLoading
{
public:
	//--- constructors

	//! TestCase constructor
	//! \param name name of the test
	TemplateParserTest(TString tstrName);

	//! destroys the test case
	~TemplateParserTest();

	//--- public api

	//! builds up a suite of testcases for this test
	static Test *suite ();

	//! build empty cache
	void BuildEmptyCache();
	//! build a cache from pure literal
	void BuildLiteralCache();
	//! build a cache form simple <FORM> </FORM> content, a action=adefinedCoastaction makes it a wd form
	void BuildSimpleFormCache();
	//! build a cache form simple <FORM> </FORM> content using <input> fields. transform name to become fld_
	void BuildFormWithInputFieldCache();
	//! build a cache by literal copying of the form content. no caching...
	void BuildSimpleFormNoCache();
	//! build a cache form simple <% any %> renderer spec
	void BuildSimpleRendererSpec();
	//! build a cache from an any spec that contains nested HTML.
	void BuildNestedHTMLSpec();
	//! build a cache simple spec using an empty tag with /> as its ending.
	void BuildWithEmptyXHTMLTag();
	//! build a cache mixing macro and php styles
	void BuildCacheWithMacro();
	//! build a cache using macro syntax within a tag, i.e. <img src=[[#wd foo bar]]>
	void MacroWithinTag();
	//! build a cache using macro syntax within a tag, i.e. <img src="[[#wd foo bar]]"> as Monika wished...
	void MacroWithinTagWithinQuote();
	//! look for invalid syntax within a macro
	void InvalidMacroTests();
	void InvalidMacroTest(const char *invalidmacro);
	//! parse classic values within a tag without quotes
	void QuotelessAttributes();
	//! force missing brace and too many closing braces errors
	void AnythingBracesWrong();
	//! test if javascript containing a single < is parsed correctly, NB, for now always place a space after a < in javascript
	void NoTagWithinJavascript();
	//! test all possible kinds of configured transition tokens, e.g. slot in page and token in role-token-map
	void BuildFormWithConfiguredTransitionTokens();
};

#endif
