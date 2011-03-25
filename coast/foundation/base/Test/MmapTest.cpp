/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#if !defined(WIN32)

#include "MmapTest.h"
#include "TestSuite.h"
#include "MmapStream.h"
#include "SystemBase.h"
#include "SystemFile.h"
#include "StringStream.h"

using namespace Coast;

#include <fcntl.h>
#include <fstream>

// for additional functionality / syscalls

const char *const MmapTest::fgcContent =
	"This is the content of a Test File\n"
	"it consists of several lines of simple Text\n"
	"and will be written to a file using the mmap iostream\n";

const char *const MmapTest::fgcFilename = "tmp/MmapTest.txt";

MmapTest::MmapTest(TString tname)
	: TestCaseType(tname)
{
}

MmapTest::~MmapTest()
{
}

Test *MmapTest::suite ()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, MmapTest, SimpleReadOnly);
	ADD_CASE(testSuite, MmapTest, TestMagicFlags);
	ADD_CASE(testSuite, MmapTest, SimpleWrite);
	ADD_CASE(testSuite, MmapTest, SimpleRead);
	ADD_CASE(testSuite, MmapTest, EmptyFileRead);
	ADD_CASE(testSuite, MmapTest, EmptyFilePutback);
//	ADD_CASE(testSuite,MmapTest,FStreamEmptyFilePutback);
	ADD_CASE(testSuite, MmapTest, SimpleTruncate);
	ADD_CASE(testSuite, MmapTest, SimplePutback);
	ADD_CASE(testSuite, MmapTest, SimpleROPutback);
	ADD_CASE(testSuite, MmapTest, SimpleAppend);
	ADD_CASE(testSuite, MmapTest, SimpleAtEnd);
	ADD_CASE(testSuite, MmapTest, SimpleSeek);
	ADD_CASE(testSuite, MmapTest, LargeWrite);
	ADD_CASE(testSuite, MmapTest, SimulatedLogAppend);
	ADD_CASE(testSuite, MmapTest, OperatorShiftLeftWithReadBuf);
	return testSuite;
}

void MmapTest::TestMagicFlags()
{
	if ( System::EnvGet("COAST_USE_MMAP_STREAMS").AsLong(1L) == 1L ) {
		MmapMagicFlags fl(std::ios::in, MmapStreamBuf::eSync);
		t_assert(fl.IsIosIn());
		t_assert(!fl.IsIosOut());
		t_assert(fl.IsReadable());
		t_assert(!(fl.GetSyscallOpenMode()&O_CREAT));
		t_assert(!(fl.GetSyscallOpenMode()&O_TRUNC));

		fl = MmapMagicFlags (std::ios::in, MmapStreamBuf::eSync);
		t_assert(fl.IsIosIn());
		t_assert(!fl.IsIosOut());
		t_assert(fl.IsReadable());
		t_assert(!(fl.GetSyscallOpenMode()&O_CREAT));
		t_assert(!(fl.GetSyscallOpenMode()&O_TRUNC));

		fl = MmapMagicFlags(std::ios::out, MmapStreamBuf::eSync);
		t_assert(fl.IsIosOut());
		t_assert(fl.IsWriteable());
		t_assert(fl.GetSyscallOpenMode()&O_RDWR);
		t_assert(fl.GetSyscallOpenMode()&O_CREAT);
		t_assert(fl.GetSyscallOpenMode()&O_TRUNC);

		fl = MmapMagicFlags(std::ios::app, MmapStreamBuf::eSync);
		t_assert(fl.IsIosOut());
		t_assert(fl.IsIosApp());
		t_assert(fl.IsIosAppendOrAtEnd());
		t_assert(fl.IsWriteable());
		t_assert(fl.GetSyscallOpenMode()&O_RDWR);
		t_assert(fl.GetSyscallOpenMode()&O_CREAT);
		t_assert(!(fl.GetSyscallOpenMode()&O_TRUNC));

		fl = MmapMagicFlags(std::ios::in | std::ios::out, MmapStreamBuf::eSync);
		t_assert(fl.IsIosIn());
		t_assert(fl.IsIosOut());
		t_assert(fl.IsWriteable());
		t_assert(fl.IsReadable());
		t_assert(fl.GetSyscallOpenMode()&O_RDWR);
		t_assert(fl.GetSyscallOpenMode()&O_CREAT);
		t_assert(fl.GetSyscallOpenMode()&O_TRUNC);
	}
}

void MmapTest::LargeWrite()
// what: check writing more than one mapped page
{
	if ( System::EnvGet("COAST_USE_MMAP_STREAMS").AsLong(1L) == 1L ) {
		System::IO::unlink(fgcFilename);
		// note this can be a problem if more than one process runs this test
		OMmapStream os(fgcFilename);
		t_assert(os.good());
		for (int i = 0; i < 1000; i ++) {
			os.width(5);
			os << i ;
			os.width(0);
			os << ":" << fgcContent << "\n";
		}
		t_assert(os.good());
		os.close();
		os << "dummy"; // should not crash us here!
		t_assert(!os.good()); // some error should occur, eof or fail or bad
		// should check if file really exists just with low level syscalls and fcts
		int fd = open(fgcFilename, O_RDONLY);
		t_assert(fd >= 0);
		long filelen = lseek(fd, 0L, SEEK_END);
		assertEqual(1000 * (7 + strlen(fgcContent)), filelen);
		// do not check the content
		close(fd);
	}
} // LargeWrite

void MmapTest::SimpleWrite()
// what: check a few properties of a simple DoScramble-DoUnscramble cycle
{
	if ( System::EnvGet("COAST_USE_MMAP_STREAMS").AsLong(1L) == 1L ) {
		IntSimpleWrite();
	}
}

void MmapTest::IntSimpleWrite(int openmode)
// what: check a few properties of a simple DoScramble-DoUnscramble cycle
{
	System::IO::unlink(fgcFilename);
	// note this can be a problem if more than one process runs this test
	OMmapStream os(fgcFilename, std::ios::out, openmode);
	t_assert(os.good());
	os << fgcContent;
	os.close();
	os << "dummy"; // should not crash us here!
	t_assert(!os.good()); // some error should occur, eof or fail or bad
	// should check if file really exists just with low level syscalls and fcts
	int fd = open(fgcFilename, O_RDONLY);
	t_assert(fd >= 0);
	if ( fd > 0 ) {
		long filelen = lseek(fd, 0L, SEEK_END);
		assertEqual(strlen(fgcContent), filelen);
		char *buf = new char [filelen+1];
		lseek(fd, 0L, SEEK_SET);
		assertEqual(filelen, read(fd, buf, filelen));
		buf[filelen] = 0;
		assertCharPtrEqual(fgcContent, buf);
		delete [] buf;
		close(fd);
	}

} // IntSimpleWrite

void MmapTest::SimpleRead()
// what:
{
	if ( System::EnvGet("COAST_USE_MMAP_STREAMS").AsLong(1L) == 1L ) {
		IntSimpleWrite(); // set up testfile
		IMmapStream is(fgcFilename);
		t_assert(is.good());
		if (is.good()) {
			const char *pc = fgcContent;
			char c;
			do {
				t_assert(!!is);
				is.get(c);
				if (is.eof()) {
					break;
				}
				char c1 = *pc++;
				assertEqual(c1, c); // compare content
			} while (true); //lint !e506
			assertEqual(pc - fgcContent, strlen(fgcContent)); // have we reached the end?
		}
		t_assert(is.eof());
	}
}
void MmapTest::OperatorShiftLeftWithReadBuf()
{
	if ( System::EnvGet("COAST_USE_MMAP_STREAMS").AsLong(1L) == 1L ) {
		String testContent("hank");
		IStringStream is(testContent);
		if (t_assert(is.good())) {
			OMmapStream os("testout2.txt");
			t_assert(os.good());
			os << is.rdbuf();
			t_assert(os.good());

			std::istream *is2 = System::OpenIStream("testout2", "txt");
			if (!t_assert(is2)) {
				return;
			}

			OStringStream buf;
			buf << is2->rdbuf();
			assertEqual(testContent, buf.str());
		}
	}
}

void MmapTest::EmptyFileRead()
// what:
{
	if ( System::EnvGet("COAST_USE_MMAP_STREAMS").AsLong(1L) == 1L ) {
		String mname("tmp/mmapEmptyFile");
		std::ostream *os0 = System::OpenOStream(mname, "tst");
		delete os0;

		mname << ".tst";
		MmapStream is(mname , (std::ios::in));
		t_assert(is.good());
		if (is.good()) {
			char c;
			t_assert(!!is);
			is.get(c);
			t_assert(is.eof());
		}
		t_assert(is.eof());
	}
}

void MmapTest::EmptyFilePutback()
// what:
{
	if ( System::EnvGet("COAST_USE_MMAP_STREAMS").AsLong(1L) == 1L ) {
		System::IO::unlink(fgcFilename);
		// note this can be a problem if more than one process runs this test
		MmapStream ms(fgcFilename, std::ios::in | std::ios::out);

		t_assert(ms.good());
		if (ms.good()) {
			ms << 'X';
			char c;
			ms.get(c);
			t_assert(c == 'X');
			ms.putback('Y');
			t_assert(ms.good());
			ms.get(c);
			t_assert(c == 'Y');
			ms.get(c);
			t_assert(ms.eof());
		}
	}
}

void MmapTest::FStreamEmptyFilePutback()
// what:
{
	if ( System::EnvGet("COAST_USE_MMAP_STREAMS").AsLong(1L) == 1L ) {
		System::IO::unlink(fgcFilename);
		// note this can be a problem if more than one process runs this test
		std::fstream ms(fgcFilename, std::ios::in | std::ios::out);

		t_assert(ms.good());
		if (ms.good()) {
			ms << 'X';
			char c;
			ms.putback('Y');
			t_assert(!ms.eof());
			ms.get(c);// this fails, no explanation
			t_assert(!ms.eof());
			assertEqual((long)'Y', long(c));
			ms.get(c);
			t_assert(ms.eof());
		}
	}
}

void MmapTest::SimpleReadOnly()
// what:
{
	if ( System::EnvGet("COAST_USE_MMAP_STREAMS").AsLong(1L) == 1L ) {
		this->IntSimpleWrite(0600); // set up testfile, should fail for SunC++ 4.2
		IMmapStream is(fgcFilename);
		t_assert(is.good());
		if (is.good()) {
			const char *pc = fgcContent;
			char c;
			do {
				t_assert(!!is);
				is.get(c);
				if (is.eof()) {
					break;
				}
				char c1 = *pc++;
				assertEqual(c1, c);
			} while (true); //lint !e506 // compare content
			assertEqual(pc - fgcContent, strlen(fgcContent)); // have we reached the end?
		}
		t_assert(is.eof());
	}
} // SimpleRead

void MmapTest::SimpleROPutback()
// what:
{
	if ( System::EnvGet("COAST_USE_MMAP_STREAMS").AsLong(1L) == 1L ) {
		this->IntSimpleWrite(); // set up testfile
		IMmapStream is(fgcFilename);
		t_assert(is.good());
		char c;
		TString s;

		for (int i = 0; i < 10 ; i++) {
			is.get(c);
			s.Append(c);
		}
		assertEqual("This is th", s);
		is.putback(c);
		t_assertm(!!is, "stream good after putback");
		char c2;
		is.get(c2);
		assertEqual(c2, c);
	}
} // SimplePutback

void MmapTest::SimplePutback()
// what:
{
	if ( System::EnvGet("COAST_USE_MMAP_STREAMS").AsLong(1L) == 1L ) {
		this->IntSimpleWrite(); // set up testfile
		MmapStream is(fgcFilename, std::ios::in | std::ios::out | std::ios::app); // do not trunc the file
		t_assert(is.good());
		char c;
		TString s;

		for (int i = 0; i < 10 ; i++) {
			t_assert(! !is.get(c));
			s.Append(c);
		}
		assertEqual("This is th", s);
		is.putback(c);
		t_assertm(!!is, "stream good after putback");
		char c2;
		is.get(c2);
		assertEqual(c2, c);
		// add more putbacks to really test it
		is.putback(' ');
		is.putback('\n');
		t_assertm(!!is, "stream good after double putback");
		is.get(c);
		is.get(c2);
		assertEqual('\n', c);
		assertEqual(' ', c2);
	}
} // SimplePutback

void MmapTest::SimpleAppend()
// what:
{
	if ( System::EnvGet("COAST_USE_MMAP_STREAMS").AsLong(1L) == 1L ) {
		this->IntSimpleWrite(); // set up testfile
		OMmapStream os(fgcFilename, std::ios::app);
		t_assert(os.good());
		os << fgcContent;
		os.close();
		int fd = open(fgcFilename, O_RDONLY);
		t_assert(fd >= 0);
		long filelen = lseek(fd, 0L, SEEK_END);
		assertEqual(2 * strlen(fgcContent), filelen);
		close(fd);
		System::IO::unlink(fgcFilename);
	}
} // SimpleAppend

void MmapTest::SimpleTruncate()
// what:
{
	if ( System::EnvGet("COAST_USE_MMAP_STREAMS").AsLong(1L) == 1L ) {
		this->IntSimpleWrite(); // set up testfile
		MmapStream os(fgcFilename, std::ios::in | std::ios::out); // this truncates the file
		t_assert(os.good());
		os << "Hallo";
		os.close();
		int fd = open(fgcFilename, O_RDONLY);
		t_assert(fd >= 0);
		long filelen = lseek(fd, 0L, SEEK_END);
		assertEqual(strlen("Hallo"), filelen);
		close(fd);
		System::IO::unlink(fgcFilename);
	}
} // SimpleAppend

void MmapTest::SimulatedLogAppend()
// what:
{
	if ( System::EnvGet("COAST_USE_MMAP_STREAMS").AsLong(1L) == 1L ) {
		TString logfilename(fgcFilename);
		logfilename << ".log";
		System::IO::unlink(logfilename);

		OMmapStream os(logfilename, std::ios::app, MmapStreamBuf::openprot,
					   MmapStreamBuf::eSync); // eSync is default
		t_assert(os.good());
		for (int j = 0; j < 2; j++) {
			for (int i = 0; i < 5; i++) {
				os << j << ", " << i << fgcContent << std::endl; // endl should sync implicitely
			}
		}
		os.close();
		System::IO::unlink(logfilename); // clean up afterwards
	}
} // SimpleAppend

void MmapTest::SimpleAtEnd()
// what:
{
	if ( System::EnvGet("COAST_USE_MMAP_STREAMS").AsLong(1L) == 1L ) {
		this->IntSimpleWrite(); // set up testfile
		OMmapStream os(fgcFilename, std::ios::ate);
		t_assert(os.good());
		os << fgcContent;
		os.close();
		int fd = open(fgcFilename, O_RDONLY);
		t_assert(fd >= 0);
		long filelen = lseek(fd, 0L, SEEK_END);
		assertEqual(2 * strlen(fgcContent), filelen);
		close(fd);
		System::IO::unlink(fgcFilename);
	}
} // SimpleAtEnd

void MmapTest::SimpleSeek()
// what:
{
	if ( System::EnvGet("COAST_USE_MMAP_STREAMS").AsLong(1L) == 1L ) {
		const long searchpos = 10000; // larger as a memory page
		this->IntSimpleWrite(); // set up testfile
		OMmapStream os(fgcFilename);
		t_assert(os.good());
		os.seekp(searchpos); // force enlargement
		os << fgcContent ;
		os.close();
		int fd = open(fgcFilename, O_RDONLY);
		t_assert(fd >= 0);
		long filelen = lseek(fd, 0L, SEEK_END);
		assertEqual(searchpos + strlen(fgcContent), filelen);
		close(fd);
		System::IO::unlink(fgcFilename);
	}
} // SimpleSeek
#endif
