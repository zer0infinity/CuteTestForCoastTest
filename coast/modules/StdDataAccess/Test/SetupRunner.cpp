/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------

//--- interface include --------------------------------------------------------
#include "TestRunner.h"

//--- test cases ---------------------------------------------------------------
#include "HTTPResponseMapperTest.h"
#include "HTTPMimeHeaderMapperTest.h"
#include "HTTPDAImplTest.h"
#include "HTTPMapperTest.h"
#include "XMLBodyMapperTest.h"
#include "HTTPProcessorTest.h"
#include "MailDATest.h"
#include "CgiCallerTest.h"
#include "CgiParamsTest.h"
#include "ConfiguredActionTest.h"
#include "AuthenticationServiceTest.h"
#include "HTTPFileLoaderTest.h"
#include "MIMEHeaderTest.h"
#include "RequestBodyParserTest.h"
#include "HTTPProtocolReplyRendererTest.h"
#include "NewRendererTest.h"
#include "RequestReaderTest.h"

#if !defined(WIN32)
#include "SetupCase.h"
#endif

void setupRunner(TestRunner &runner)
{
#if !defined(WIN32)
	ADD_SUITE(runner, SetupCase);
#endif
	ADD_SUITE(runner, HTTPDAImplTest);
	ADD_SUITE(runner, HTTPMapperTest);
	ADD_SUITE(runner, XMLBodyMapperTest);
	ADD_SUITE(runner, HTTPProcessorTest);
	ADD_SUITE(runner, MailDATest);
	ADD_SUITE(runner, CgiCallerTest);
	ADD_SUITE(runner, CgiParamsTest);
	ADD_SUITE(runner, AuthenticationServiceTest);
	ADD_SUITE(runner, ConfiguredActionTest);
	ADD_SUITE(runner, HTTPFileLoaderTest);
	ADD_SUITE(runner, MIMEHeaderTest);
	ADD_SUITE(runner, RequestBodyParserTest);
	ADD_SUITE(runner, HTTPProtocolReplyRendererTest);
	ADD_SUITE(runner, NewRendererTest);
	ADD_SUITE(runner, HTTPResponseMapperTest);
	ADD_SUITE(runner, HTTPMimeHeaderMapperTest);
	ADD_SUITE(runner, RequestReaderTest);
}
