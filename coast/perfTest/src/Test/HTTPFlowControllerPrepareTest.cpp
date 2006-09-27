/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "HttpFlowController.h"

//--- interface include --------------------------------------------------------
#include "HTTPFlowControllerPrepareTest.h"

//--- standard modules used ----------------------------------------------------
#include "System.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------
#include <stdlib.h>

//---- HTTPFlowControllerPrepareTest ----------------------------------------------------------------
HTTPFlowControllerPrepareTest::HTTPFlowControllerPrepareTest(TString tstrName)
	: HTTPFlowControllerTest(tstrName)
{
	StartTrace(HTTPFlowControllerPrepareTest.HTTPFlowControllerPrepareTest);
}

TString HTTPFlowControllerPrepareTest::getConfigFileName()
{
	return "HTTPFlowControllerPrepareTestConfig";
}

HTTPFlowControllerPrepareTest::~HTTPFlowControllerPrepareTest()
{
}

void HTTPFlowControllerPrepareTest::DoCommandTest()
{
	HttpFlowController fc("DoCommandTestFlowController");
	fc.Initialize("FlowController");

	bool retVal = true;
	Anything dummy;
	Context ctx(dummy, dummy, 0, 0, 0);
	t_assertm(fc.PrepareRequest(ctx, retVal), "PrepareRequest step 1 fails");
	t_assertm(retVal, "PrepareRequest bPrepareRequestSucceeded step 1 returns false");

	String cmd = "test \"Hallo\" = `cat DoCommandTest.txt`";
	t_assertm(::system(cmd) == 0, "file was not created properly" );

	t_assertm(fc.PrepareRequest(ctx, retVal), "PrepareRequest step 2 fails");
	t_assertm(retVal, "PrepareRequest bPrepareRequestSucceeded step 2 returns false");
	cmd = "ls DoCommandTest.txt";
	t_assertm(::system(cmd) != 0, "file was not removed" );
}

// Checks if the tmpStore is prepared according the FlowController's configuration
void HTTPFlowControllerPrepareTest::ProcessSetCookieTest()
{
	StartTrace(HTTPFlowControllerTest.ProcessSetCookieTest);
	HttpFlowController fc("ProcessSetCookieTest");
	Anything dummy;
	Context ctx(dummy, dummy, 0, 0, 0);
	Anything tmpStore = ctx.GetTmpStore();
	Anything &setCookie = tmpStore["Mapper" ]["HTTPHeader"]["set-cookie"];

	setCookie[0L] = "FINXSCUSTNO=CH10601; path=/; expires=Mon, 20 Apr 2009 09:16:18 GMT;";
	setCookie[1L] = "FINXSSESSIONID=TKGAE-655194x211877x891402x38624640; path=/;";
	setCookie[2L] =	"FD_USER_ALTEON=b64-YnMwOiquRQcdzqgdRuAdSjgUy3UEO4-4RwpB9EUEP9QDnmOWPKbp70k$HDrNI4XaqmWb85hP-VNDm2hnz0GdMfD93uU=; expires=Thu, 21 Apr 2005 09:30:33 GMT; Path=/; Domain=.hsr.loc;";
	setCookie[3L] =	"FDState=b64-YnMwOgUQE0OJ4W8m$SVvziZle5InMUSZQSW2vGbZLwPQ4FD1QZBsh60PvcuIDuC-QrG6ivqH$ufJ8P4GSaj9pcj9aQrDw5bXfu3uadjI7pNq23n1BU75p5fjUY2axPe$xWyzrm0JEo5DhxP9Z6cpWtSupg4xejed-OgZQA==; expires=Thu, 21 Apr 2005 09:30:33 GMT; Path=/; Domain=.hsr.loc;";
	setCookie[4L] =	"FD_USER_ALTEON=b64-YnMwOiquRQcdzqgdRuAdSjgUy3UEO4-4RwpB9EUEP9QDnmOWPKbp70k$HDrNI4XaqmWb85hP-VNDm2hnz0GdMfD93uU=; expires=Thu, 21 Apr 2005 09:30:33 GMT; Path=/; Domain=.quark.com;";

	fc.ProcessSetCookie(ctx);

	Anything result;
	result["Cookies"][0L]["FINXSCUSTNO"] = String("FINXSCUSTNO=CH10601");
	result["Cookies"][1L]["FINXSSESSIONID"] = String("FINXSSESSIONID=TKGAE-655194x211877x891402x38624640");
	result["Cookies"][".HSR.LOC"]["FD_USER_ALTEON"] = String(
				"FD_USER_ALTEON=b64-YnMwOiquRQcdzqgdRuAdSjgUy3UEO4-4RwpB9EUEP9QDnmOWPKbp70k$HDrNI4XaqmWb85hP-VNDm2hnz0GdMfD93uU=");
	result["Cookies"][".HSR.LOC"]["FDState"] = String(
				"FDState=b64-YnMwOgUQE0OJ4W8m$SVvziZle5InMUSZQSW2vGbZLwPQ4FD1QZBsh60PvcuIDuC-QrG6ivqH$ufJ8P4GSaj9pcj9aQrDw5bXfu3uadjI7pNq23n1BU75p5fjUY2axPe$xWyzrm0JEo5DhxP9Z6cpWtSupg4xejed-OgZQA==");
	result["Cookies"][".QUARK.COM"]["FD_USER_ALTEON"] = String(
				"FD_USER_ALTEON=b64-YnMwOiquRQcdzqgdRuAdSjgUy3UEO4-4RwpB9EUEP9QDnmOWPKbp70k$HDrNI4XaqmWb85hP-VNDm2hnz0GdMfD93uU=");

	assertAnyEqual(result["Cookies"], ctx.GetTmpStore()["Cookies"]);
}

// builds up a suite of testcases, add a line for each testmethod
Test *HTTPFlowControllerPrepareTest::suite ()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, HTTPFlowControllerPrepareTest, TestCases);
	ADD_CASE(testSuite, HTTPFlowControllerPrepareTest, DoCommandTest);
	ADD_CASE(testSuite, HTTPFlowControllerPrepareTest, ProcessSetCookieTest);
	return testSuite;
}
