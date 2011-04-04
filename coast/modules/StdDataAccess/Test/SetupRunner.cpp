/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

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
#include "HTTPPostRequestBodyParserTest.h"
#include "HTTPProtocolReplyRendererTest.h"
#include "NewRendererTest.h"
#include "HTTPRequestReaderTest.h"
#include "ConnectorDAImplTest.h"
#include "URI2FileNameTest.h"
#include "HTTPProcessorWithChecksTest.h"
#include "MimeHeaderResultMapperTest.h"
#include "SplitCookieResultMapperTest.h"
#include "MSAjaxFixFieldLengthResultMapperTest.h"
#include "SimpleDAServiceTest.h"

void setupRunner(TestRunner &runner)
{
	ADD_SUITE(runner, MIMEHeaderTest);
	ADD_SUITE(runner, HTTPRequestReaderTest);
	ADD_SUITE(runner, HTTPPostRequestBodyParserTest);
	ADD_SUITE(runner, HTTPProtocolReplyRendererTest);
	ADD_SUITE(runner, HTTPProcessorTest);
	ADD_SUITE(runner, HTTPProcessorWithChecksTest);
	ADD_SUITE(runner, SplitCookieResultMapperTest);
	ADD_SUITE(runner, MSAjaxFixFieldLengthResultMapperTest);

	ADD_SUITE(runner, HTTPMapperTest);
	ADD_SUITE(runner, HTTPResponseMapperTest);
	ADD_SUITE(runner, HTTPMimeHeaderMapperTest);
	ADD_SUITE(runner, MimeHeaderResultMapperTest);
	ADD_SUITE(runner, XMLBodyMapperTest);

	ADD_SUITE(runner, ConnectorDAImplTest);
	ADD_SUITE(runner, CgiParamsTest);
	ADD_SUITE(runner, URI2FileNameTest);
	ADD_SUITE(runner, CgiCallerTest);

	ADD_SUITE(runner, SimpleDAServiceTest);
	ADD_SUITE(runner, HTTPFileLoaderTest);
	ADD_SUITE(runner, HTTPDAImplTest);
	ADD_SUITE(runner, MailDATest);
	ADD_SUITE(runner, AuthenticationServiceTest);
	ADD_SUITE(runner, ConfiguredActionTest);
	ADD_SUITE(runner, NewRendererTest);
}
