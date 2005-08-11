/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ZipStreamTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "ZipStream.h"

//--- standard modules used ----------------------------------------------------
#include "System.h"
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------
#include <stdlib.h>
#include "gzio.h"

//---- ZipStreamTest ----------------------------------------------------------------
ZipStreamTest::ZipStreamTest(TString tstrName) : TestCase(tstrName)
{
	StartTrace(ZipStreamTest.Ctor);
}

ZipStreamTest::~ZipStreamTest()
{
	StartTrace(ZipStreamTest.Dtor);
}

// setup for this TestCase
void ZipStreamTest::setUp ()
{
	StartTrace(ZipStreamTest.setUp);
	istream *is = System::OpenStream(getClassName(), "any");
	if ( is ) {
		fConfig.Import( *is );
		delete is;
		fTestCaseConfig = fConfig[name()];
	} else {
		t_assertm( false, TString("could not read ") << getClassName() << ".any" );
	}
}

void ZipStreamTest::tearDown ()
{
	StartTrace(ZipStreamTest.tearDown);
}

static String content("Hello Hello Peter\nhallo\n");

void ZipStreamTest::GzipHdrTest()
{
	StartTrace(ZipStreamTest.GzipHdrTest);
	GzipHdr aHeader;
	assertEqual(gz_magic[0], aHeader.ID1);
	assertEqual(gz_magic[1], aHeader.ID2);
	assertEqual(GzipHdr::eDeflate, aHeader.CompressionMethod);
	assertEqual(0, aHeader.Flags);
	assertEqual(0L, aHeader.ModificationTime[0]);
	assertEqual(0L, aHeader.ModificationTime[1]);
	assertEqual(0L, aHeader.ModificationTime[2]);
	assertEqual(0L, aHeader.ModificationTime[3]);
	assertEqual(GzipHdr::eUnspecified, aHeader.ExtraFlags);
#if defined(WIN32)
#  if !defined(__CYGWIN__)  /* Cygwin is Unix, not Win32 */
#    define OS_CODE  GzipHdr::eNTFS
#  endif
#elif defined(__linux__) || defined(__sun)
#  define OS_CODE  GzipHdr::eUnix
#endif
	assertEqual(OS_CODE, aHeader.OperatingSystem);
	assertEqual(4L, (long) & (aHeader.ModificationTime) - (long)&aHeader);
	assertEqual(8L, (long) & (aHeader.ExtraFlags) - (long)&aHeader);
	assertEqual(9L, (long) & (aHeader.OperatingSystem) - (long)&aHeader);
	t_assert( aHeader.fbValid );
}

void ZipStreamTest::GzipHdrWriteTest()
{
	StartTrace(ZipStreamTest.GzipHdrWriteTest);
	OStringStream os;
	ZipOStream zos(os);
	t_assert(os.flush() != NULL);
	assertEqual(0L, os.str().Length());

	zos << flush;

	// header must be written after first byte streamd to stream
	assertEqual(10L, os.str().Length());
	GzipHdr aHeader;
	StringStream streamHeader;
	streamHeader << aHeader << flush;

	String writtenHeader = os.str();
	writtenHeader.Trim(10);
	assertCharPtrEqual(streamHeader.str(), writtenHeader);
}

void ZipStreamTest::ReadGzipHdrFileTest()
{
	StartTrace(ZipStreamTest.ReadGzipHdrFileTest);
	for (long lIdx = 0; lIdx < fTestCaseConfig.GetSize(); lIdx++) {
		ROAnything roaConfig = fTestCaseConfig[lIdx];
		String strCprs(roaConfig["BinaryData"].AsString());
		TString strCase = fTestCaseConfig.SlotName(lIdx);
		if ( !strCase.Length() ) {
			strCase << "idx:" << lIdx;
		}
		IStringStream stream(strCprs);
		ZipIStream zis(stream);
		char cDummy;
		zis >> cDummy;
		ZipIStreamBuf *pBuf = zis.rdbuf();
		const GzipHdr &aHeader = pBuf->fHeader;
		if ( t_assertm( aHeader.fbValid == roaConfig["Valid"].AsBool(false), TString("failure at ") << strCase ) && aHeader.fbValid ) {
			long lFlags = 0L;
			if ( roaConfig.IsDefined("ModTime") ) {
				TimeStamp aStampExpected(roaConfig["ModTime"].AsString());
				TimeStamp aStampHeader = aHeader.GetModificationTime();
				t_assertm(aStampExpected == aStampHeader, strCase);
				assertCharPtrEqualm(aStampExpected.AsString(), aStampHeader.AsString(), strCase);
			}
			if ( roaConfig.IsDefined("Flags") ) {
				lFlags = roaConfig["Flags"].AsLong();
				assertEqualm(lFlags, (long)(aHeader.Flags), strCase);
			}
			if ( roaConfig.IsDefined("Os") ) {
				assertEqualm(roaConfig["Os"].AsLong(255), (long)(aHeader.OperatingSystem), strCase);
			}
			if ( lFlags & GzipHdr::eFEXTRA ) {
				assertEqualm(roaConfig["Xlen"].AsLong(0), (long)aHeader.XLEN, strCase);
				if ( roaConfig.IsDefined("ExtraField") ) {
					assertEqualRawm( roaConfig["ExtraField"].AsString(), aHeader.ExtraField, strCase);
				}
			}
			if ( lFlags & GzipHdr::eFNAME ) {
				assertCharPtrEqualm( roaConfig["FileName"].AsString("undefined"), aHeader.FileName, strCase);
			}
			if ( lFlags & GzipHdr::eFCOMMENT ) {
				assertCharPtrEqualm( roaConfig["Comment"].AsString("undefined"), aHeader.Comment, strCase);
			}
		}
	}
}

void ZipStreamTest::SetCompressionTest()
{
	StartTrace(ZipStreamTest.SetCompressionTest);
	String strFileName("MultCprsTest");
	ostream *os = System::OpenOStream (strFileName, "gz", ios::binary);
	if (!t_assert(os != NULL)) {
		return;
	}
	ZipOStream zos(*os);
	t_assert(!!zos);
	zos << ZipStream::setFilename("MySpecialFileName");
	zos << ZipStream::setComment("Hello, this is my comment");
	zos << ZipStream::setModificationTime(TimeStamp("20041224012345"));
	zos << ZipStream::setHeaderCRC();
	zos << content << flush;
	for (long lCprs = 0L; lCprs < Z_BEST_COMPRESSION; lCprs++) {
		zos << ZipStream::setCompression(lCprs) << content << flush;
	}
	t_assert(!!zos);
	zos.close();
	delete os;

	// should check for consistency of the file
	VerifyFile(String(strFileName) << ".gz");

	istream *is = System::OpenIStream(strFileName, "gz", ios::binary);
	if (!t_assert(is != NULL)) {
		return;
	}
	ZipIStream zis(*is);
	t_assert(!!zis);
	String strRead;
	while (!!zis && !zis.eof()) {
		String buff(1024);
		long nread = zis.read((char *)(const char *)buff, buff.Capacity()).gcount();
		strRead.Append((const char *)buff, nread);
	}
	t_assert(!zis != 0);
	zis.close();
	delete is;
}

void ZipStreamTest::GzipZlibTest()
{
	StartTrace(ZipStreamTest.GzipZlibTest);

	// check our prezipped master file
	VerifyFile("master.gz");

	// generate a zipped file
	gzFile *outFile = (gzFile *)gzopen("tt.gz", "w");
	t_assert(outFile != NULL);
	t_assert(gzwrite(outFile, (void *)(const char *)content, content.Length()) > 0L);
	int err = gzclose(outFile);
	Trace("closing zip err=" << (long)err);
	t_assert(err == Z_OK);

	VerifyFile("tt.gz");
}
static String ReadStream(istream &is)
{
	StartTrace(ZipStreamTest.ReadStream);

	String ret(Storage::Global());

	while (is.good() && !is.eof()) {
		char buff[4096];
		long nread = is.read(buff, sizeof(buff)).gcount();
		ret.Append((void *)buff, nread);

		Trace("read: " << nread);
	}
	Trace("returning: " << ret.Length());

	return ret;
}

void ZipStreamTest::GzipLongFileCheck()
{
	StartTrace(ZipStreamTest.GzipLongFileCheck);

	istream *is = System::OpenIStream("longpage", "html");
	if (!t_assertm((is != NULL), "file longpage.html not found")) {
		return;
	}

	String raw;
	raw = ReadStream(*is);
	delete is;

	String encoded;
	StringStream ssenc(encoded);
	ZipOStream zos(ssenc);

	zos << raw;
	zos.close();

	StringStream ssdec(encoded);
	ZipIStream zis(ssdec);

	String cooked;
	cooked = ReadStream(zis);

	t_assert(raw.Length() == cooked.Length());
	t_assert(raw == cooked);
}

void ZipStreamTest::GzipSimpleFileCheck()
{
	StartTrace(ZipStreamTest.GzipSimpleFileCheck);
	ostream *os = System::OpenOStream ("testzip", "gz", ios::binary);
	if (!t_assert(os != NULL)) {
		return;
	}
	ZipOStream zos(*os);
	t_assert(!!zos);
	zos << content << flush;
	t_assert(!!zos);
	zos.close();
	delete os;

	// should check for consistency of the file
	VerifyFile("testzip.gz");

	istream *is = System::OpenIStream("testzip", "gz", ios::binary);
	if (!t_assert(is != NULL)) {
		return;
	}
	ZipIStream zis(*is);
	t_assert(!!zis);
	String strRead;
	while (!!zis && !zis.eof()) {
		String buff(1024);
		long nread = zis.read((char *)(const char *)buff, buff.Capacity()).gcount();
		strRead.Append((const char *)buff, nread);
	}
	t_assert(!zis != 0);
	assertEqual(content, strRead);
	zis.close();
	delete is;
}

void ZipStreamTest::GzipConstantBufferCheck()
{
	StartTrace(ZipStreamTest.GzipConstantBufferCheck);

	srand(12345);
	char inFile[89000];
	{
		for (unsigned i = 0; i < sizeof(inFile); i++) {
			inFile[i] = (char) rand();
		}
	}
	String encoded;
	StringStream ssout(encoded);
	ZipOStream zos(ssout);
	{
		for (unsigned i = 0; i < sizeof(inFile); i += 1024) {
			unsigned long toWrite = 1024;

			if (toWrite > (sizeof(inFile) - i)) {
				toWrite = (sizeof(inFile) - i);
			}

			zos.write(inFile + i, toWrite);
		}
	}
	zos.close();

	Trace("Encoded size: " << encoded.Length());

	StringStream ssin(encoded);
	ZipIStream zis(ssin);
	String decoded;
	while (zis.good() && !zis.eof()) {
		char buff[24096];
		long nread = zis.read(buff, sizeof(buff)).gcount();
		decoded.Append((void *)buff, nread);
	}

	t_assert(sizeof(inFile) == decoded.Length());
	{
		for (unsigned i = 0; i < sizeof(inFile); i++) {
			if (!t_assert(inFile[i] == decoded[(long)i])) {
				break;
			}
		}
	}
	encoded << "a bit of garbage at the end" ;
	StringStream ssin2(encoded);
	ZipIStream zis2(ssin2);
	String decoded2;
	while (zis2.good() && !zis2.eof()) {
		char buff[24096];
		long nread = zis2.read(buff, sizeof(buff)).gcount();
		decoded2.Append((void *)buff, nread);
	}

	t_assert(sizeof(inFile) == decoded2.Length());
	{
		for (unsigned i = 0; i < sizeof(inFile); i++) {
			if (!t_assert(inFile[i] == decoded2[(long)i])) {
				break;
			}
		}
	}
}

void ZipStreamTest::GzipBigFileCheck()
{
	StartTrace(ZipStreamTest.GzipBigFileCheck);

	srand(12345);
	char inFile[1024*64];
	{
		for (unsigned i = 0; i < sizeof(inFile); i++) {
			inFile[i] = (char) rand();
		}
	}
	ostream *os = System::OpenOStream("big", "gz", ios::binary);
	if (!t_assert(os != NULL)) {
		return;
	}
	ZipOStream zos(*os);

	zos.write(inFile, sizeof(inFile));
	zos.close();
	delete os;

	char outFile[sizeof(inFile)];

	istream *is = System::OpenIStream("big", "gz", ios::binary);
	if (!t_assert(is != NULL)) {
		return ;
	}
	ZipIStream zis(*is);
	unsigned nread = 0;

	while ( nread < sizeof(outFile) && !zis.eof()) {
		nread += zis.read(outFile + nread, sizeof(outFile) - nread).gcount();

		Trace("read: " << (long)nread);
	}
	t_assert(zis.get() == EOF);
	t_assert(zis.eof() != 0);
	t_assert(!zis.bad());
	zis.close();

	for (unsigned i = 0; i < sizeof(inFile); i++) {
		if ( !t_assert(inFile[i] == outFile[i])) {
			Trace("files not equal: " << (long)i);
			break;
		}
	}
	delete is;

	istream *is2 = System::OpenIStream("big", "gz", ios::binary);
	if (!t_assert(is2 != NULL)) {
		return ;
	}
	ZipIStream zis2(*is2);
	unsigned nread2 = 0;

	while ( nread2 < 20000 && !zis2.eof()) {
		nread2 += zis2.read(outFile + nread2, sizeof(outFile) - nread2).gcount();

		Trace("read: " << (long)nread2);
	}
	t_assert(!zis2.eof());
	t_assert(!zis2.bad());
	zis2.close();
	delete is2;
}

void ZipStreamTest::GzipCorruptInputCheck()
{
	String noZip("no zip at all!");
	StringStream ss1(noZip);
	ZipIStream zis1(ss1);
	zis1.get();
	t_assert(!zis1 != 0);

	String buff;
	StringStream ss3(buff);
	ZipOStream zos(ss3);
	zos << noZip;
	zos.close();

	StringStream ss4(buff);
	ZipIStream zis2(ss4);
	zis2.get();
	t_assert(!!zis2);
	t_assert(!zis2.eof());
	zis2.close();
}

void ZipStreamTest::StringGetlineTest()
{
	StartTrace(ZipStreamTest.StringGetlineTest);
	istream *is = System::OpenIStream("tt", "gz", ios::binary);
	if (!t_assert(is != NULL)) {
		return ;
	}
	ZipIStream zis(*is);

	String strLine;
	long lCount = 0;
	while ( !!zis && !getline(zis, strLine).bad() && !zis.eof() ) {
		lCount++;
		Trace("count: " << lCount);
		Trace("read [" << strLine << "]");
	}
	assertEqualm(2, lCount, "expected 2 lines to be read");
	t_assert(zis.get() == EOF);
	t_assert(zis.eof() != 0);
	t_assert(!zis.bad());
	zis.close();
}

void ZipStreamTest::VerifyFile(const char *fileName)
{
	const long length = content.Length();
	gzFile *inFile = (gzFile *)gzopen(fileName, "r");
	t_assertm(inFile != NULL, (const char *)(String("File '") << fileName << "' must exist!"));

	char *buffer = new char[length];
	t_assert(gzread(inFile, buffer, length) == length);
	assertEqual(content, String(buffer, length));
	delete[] buffer;
	int err = gzclose(inFile);
	t_assert(err == Z_OK);
}

// builds up a suite of testcases, add a line for each testmethod
Test *ZipStreamTest::suite ()
{
	StartTrace(ZipStreamTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, ZipStreamTest, GzipHdrTest);
	ADD_CASE(testSuite, ZipStreamTest, GzipHdrWriteTest);
	ADD_CASE(testSuite, ZipStreamTest, ReadGzipHdrFileTest);
	ADD_CASE(testSuite, ZipStreamTest, SetCompressionTest);
	ADD_CASE(testSuite, ZipStreamTest, GzipSimpleFileCheck);
	ADD_CASE(testSuite, ZipStreamTest, GzipBigFileCheck);
	ADD_CASE(testSuite, ZipStreamTest, GzipLongFileCheck);
	ADD_CASE(testSuite, ZipStreamTest, GzipZlibTest);
	ADD_CASE(testSuite, ZipStreamTest, GzipCorruptInputCheck);
	ADD_CASE(testSuite, ZipStreamTest, GzipConstantBufferCheck);
	ADD_CASE(testSuite, ZipStreamTest, StringGetlineTest);
	return testSuite;
}
