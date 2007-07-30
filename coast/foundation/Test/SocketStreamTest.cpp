/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include -----------
#include "SocketStreamTest.h"

//--- module under test --------------------------------------------------------
#include "SocketStream.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------

#if defined(ONLY_STD_IOSTREAM)
using namespace std;
#endif

SocketStreamTest::SocketStreamTest(TString tname)
	: TestCaseType(tname)
{
}
SocketStreamTest::~SocketStreamTest()
{
}

void SocketStreamTest::setUp()
{
	// try to reach the test 'server' on the local host
	fConnector = new Connector(GetConfig()["Testhost"]["ip"].AsString(), GetConfig()["Testhost"]["port"].AsLong(), 1000L); // PS: wait only one second
}

void SocketStreamTest::tearDown ()
{
	// close socket connection
	delete fConnector;
	fConnector = 0;
}

void SocketStreamTest::simpleRead()
{
	simpleWrite(); // send request to http server
	iostream *Ios = fConnector->GetStream();
	t_assert( Ios != NULL ); // http server doesn't run if assert fails
	if ( Ios ) {
		String str;
		// make sure Ios is valid str;
		if (t_assert(!!(*Ios))) { // make sure Ios is valid
			(*Ios) >> str;
			assertEqual( "HTTP", str.SubString(0, 4) ); // test first line of reply by http server
			t_assert(!!(*Ios));
			long charcounter = str.Length();
			while ((*Ios).good()) {
				char c;
				if (Ios->get(c)) {
					charcounter ++;
				}
			}
			// Test counted bytes of this read:
			Socket *sock = fConnector->Use();
			long bytesRead = 0;
			if (t_assert(sock != NULL)) {
				bytesRead = sock->GetReadCount();
			}
			assertEqual(charcounter, bytesRead);
		}
	}
}

void SocketStreamTest::simpleWrite()
{
	Socket *sock = fConnector->Use();
	if (t_assert(sock != NULL)) {
		//FIXME: is it really useful that Connector and Stream have the same timeout
		assertEqual(1000L, sock->GetTimeout());
		sock->SetTimeout(15L * 1000L); // increase timeout for reading and writing on the stream
		iostream *Ios = fConnector->GetStream();
		if ( t_assert( Ios != NULL ) ) {
			assertEqual(15 * 1000L, fConnector->Use()->GetTimeout());
			// make sure Ios is valid
			if (t_assert(!!(*Ios))) {
				(*Ios) << "GET / HTTP/1.0" << ENDL << ENDL << flush;
				// make sure Ios is valid
				if (t_assert(!!(*Ios))) {
					// Test counted bytes of this write:
					if (t_assert(sock != NULL)) {
						long bytesSent = sock->GetWriteCount();
						assertEqual( 18, bytesSent );
					}
				}
			}
		}
	}
}

void SocketStreamTest::timeoutTest()
{
	Connector connector(GetConfig()["Timeouthost"]["name"].AsString(), GetConfig()["Timeouthost"]["port"].AsLong(), 5000L);
	Socket *socket = connector.MakeSocket();
	// http server doesn't run if assert fails
	if ( t_assertm( socket != NULL, TString("socket creation to [") << GetConfig()["Timeouthost"]["name"].AsString() << ":" << GetConfig()["Timeouthost"]["port"].AsLong() << "] failed" ) ) {
		t_assert(socket->IsReadyForWriting());
		SocketStream Ios(socket);
		t_assert(!!Ios); // make sure Ios is valid
		Ios << "GET / HTTP/1.0" << ENDL << ENDL << flush; //PS??
		t_assert(!!Ios); // make sure is valid
		String str;
		{
			TimeoutModifier aTimeoutModifier(&Ios, 1L);
			aTimeoutModifier.Use();// wait 1 ms
			Ios >> str;
		}
		assertEqualm(0L, str.Length(), "OOPS not Timeout");
		t_assert(!(!!Ios));
		if (!t_assert( str.SubString(0, 4) != "HTTP"  )) {
			cerr << str << endl << flush;    // test first line of reply by http server should timeout
		}
	}
	delete socket;// SocketStream otherwise is destructed after socket is deleted!!!
}

void SocketStreamTest::opLeftShiftTest()
{
	const long theTimeout = 1000L;
	Connector connector(GetConfig()["Testhost"]["ip"].AsString(), GetConfig()["Testhost"]["port"].AsLong(), 1000L);
	Socket *socket = connector.MakeSocket();

	if ( t_assertm( socket != NULL, TString("socket creation to [") << GetConfig()["Testhost"]["name"].AsString() << ":" << GetConfig()["Testhost"]["port"].AsLong() << "] failed"  ) ) {
		assertEqual(theTimeout, socket->GetTimeout());
		SocketStream Ios(socket);
		t_assert(!!Ios); // make sure Ios is valid
		Ios << "GET / HTTP/1.0" << ENDL << ENDL;
		Ios.flush();
		t_assert(!!Ios); // make sure is valid
		String str;
		{
			OStringStream os(&str);
			os << Ios.rdbuf();
			t_assert(!!os);
			t_assert(!!Ios);
		}
		assertEqual( "HTTP", str.SubString(0, 4) ); // test first line of reply by http server
	}
	delete socket;
}

void SocketStreamTest::parseHTTPReplyTest()
{
	Connector connector(GetConfig()["Replyhost"]["name"].AsString(), GetConfig()["Replyhost"]["port"].AsLong(), 2000L);
	Socket *socket = connector.MakeSocket();

	if ( t_assertm( socket != NULL, TString("socket creation to [") << GetConfig()["Replyhost"]["name"].AsString() << ":" << GetConfig()["Replyhost"]["port"].AsLong() << "] failed"  ) ) { // http server doesn't run if assert fails
		SocketStream Ios(socket, 1000L); // wait at most a second
		assertEqual(1000L, Ios.rdbuf()->GetTimeout());
		t_assert(!!Ios); // make sure Ios is valid
		Ios << "GET / HTTP/1.0" << ENDL << ENDL;
		t_assert(!!Ios); // make sure is valid
		String line;
		Anything request;
		char c;
		bool firstLine = true;

		while ( !(Ios.get(c).eof()) ) {
			line << c;
			if (line == "\r\n" ) {
				break;
			}
			if (  c == '\n' ) {
				line.Trim(line.Length() - 2);
				if (firstLine) {
					request["answer"] = line;
					firstLine = false;
				} else {
					// parse all additional information in the header
					parseParams(line, request);
				}
				line = "";
			}
		}
		Anything contentLength;

		if ( request.LookupPath(contentLength, "Content-Length") ) {
			String content;
			content.Append(Ios, contentLength.AsLong(0));
			t_assert(!!Ios); // make sure Ios valid
			assertEqual(contentLength.AsLong(0), content.Length());
		}
	}
	delete socket; // delete 0 is uncritical,
	//but deleting the socket when the Socketstream object is still alive, is
}

void SocketStreamTest::parseParams(String &line, Anything &request)
{
	StringTokenizer st(line, ':');
	String key, value;

	if ( st.NextToken(key) ) {
		value = line;
		value.TrimFront(key.Length() + 1);

		StringTokenizer st1(value, ',');
		String item;
		while (st1.NextToken(item)) {
			// remove leading blanks
			long blanks = 0;
			while ( ' ' == item[blanks] ) {
				blanks++;
			}
			item.TrimFront(blanks);

			if ( !request.IsDefined(key) ) {
				request[key] = item;
			} else {
				request[key].Append(item);
			}
		}
	}
}

Test *SocketStreamTest::suite ()
{
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, SocketStreamTest, simpleRead);
	ADD_CASE(testSuite, SocketStreamTest, parseHTTPReplyTest);
	ADD_CASE(testSuite, SocketStreamTest, opLeftShiftTest);
	ADD_CASE(testSuite, SocketStreamTest, timeoutTest);

	return testSuite;

}
