/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * Copyright (c) 2015, David Tran, Faculty of Computer Science,
 * University of Applied Sciences Rapperswil (HSR),
 * 8640 Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#if !defined(WIN32)

#include "MmapTest.h"
#include "MmapStream.h"
#include "SystemBase.h"
#include "SystemFile.h"
#include "StringStream.h"

using namespace coast;

#include <fcntl.h>
#include <fstream>

// for additional functionality / syscalls

const char *const MmapTest::fgcContent =
	"This is the content of a Test File\n"
	"it consists of several lines of simple Text\n"
	"and will be written to a file using the mmap iostream\n";

const char *const MmapTest::fgcFilename = "tmp/MmapTest.txt";

void MmapTest::runAllTests(cute::suite &s) {
	s.push_back(CUTE_SMEMFUN(MmapTest, SimpleReadOnly));
	s.push_back(CUTE_SMEMFUN(MmapTest, TestMagicFlags));
	s.push_back(CUTE_SMEMFUN(MmapTest, SimpleWrite));
	s.push_back(CUTE_SMEMFUN(MmapTest, SimpleRead));
	s.push_back(CUTE_SMEMFUN(MmapTest, EmptyFileRead));
	s.push_back(CUTE_SMEMFUN(MmapTest, EmptyFilePutback));
//	s.push_back(CUTE_SMEMFUN(MmapTest, FStreamEmptyFilePutback));
	s.push_back(CUTE_SMEMFUN(MmapTest, SimpleTruncate));
	s.push_back(CUTE_SMEMFUN(MmapTest, SimplePutback));
	s.push_back(CUTE_SMEMFUN(MmapTest, SimpleROPutback));
	s.push_back(CUTE_SMEMFUN(MmapTest, SimpleAppend));
	s.push_back(CUTE_SMEMFUN(MmapTest, SimpleAtEnd));
	s.push_back(CUTE_SMEMFUN(MmapTest, SimpleSeek));
	s.push_back(CUTE_SMEMFUN(MmapTest, LargeWrite));
	s.push_back(CUTE_SMEMFUN(MmapTest, SimulatedLogAppend));
	s.push_back(CUTE_SMEMFUN(MmapTest, OperatorShiftLeftWithReadBuf));
}

void MmapTest::TestMagicFlags()
{
	if ( system::EnvGet("COAST_USE_MMAP_STREAMS").AsLong(1L) == 1L ) {
		MmapMagicFlags fl(std::ios::in, MmapStreamBuf::eSync);
		ASSERT(fl.IsIosIn());
		ASSERT(!fl.IsIosOut());
		ASSERT(fl.IsReadable());
		ASSERT(!(fl.GetSyscallOpenMode()&O_CREAT));
		ASSERT(!(fl.GetSyscallOpenMode()&O_TRUNC));

		fl = MmapMagicFlags (std::ios::in, MmapStreamBuf::eSync);
		ASSERT(fl.IsIosIn());
		ASSERT(!fl.IsIosOut());
		ASSERT(fl.IsReadable());
		ASSERT(!(fl.GetSyscallOpenMode()&O_CREAT));
		ASSERT(!(fl.GetSyscallOpenMode()&O_TRUNC));

		fl = MmapMagicFlags(std::ios::out, MmapStreamBuf::eSync);
		ASSERT(fl.IsIosOut());
		ASSERT(fl.IsWriteable());
		ASSERT(fl.GetSyscallOpenMode()&O_RDWR);
		ASSERT(fl.GetSyscallOpenMode()&O_CREAT);
		ASSERT(fl.GetSyscallOpenMode()&O_TRUNC);

		fl = MmapMagicFlags(std::ios::app, MmapStreamBuf::eSync);
		ASSERT(fl.IsIosOut());
		ASSERT(fl.IsIosApp());
		ASSERT(fl.IsIosAppendOrAtEnd());
		ASSERT(fl.IsWriteable());
		ASSERT(fl.GetSyscallOpenMode()&O_RDWR);
		ASSERT(fl.GetSyscallOpenMode()&O_CREAT);
		ASSERT(!(fl.GetSyscallOpenMode()&O_TRUNC));

		fl = MmapMagicFlags(std::ios::in | std::ios::out, MmapStreamBuf::eSync);
		ASSERT(fl.IsIosIn());
		ASSERT(fl.IsIosOut());
		ASSERT(fl.IsWriteable());
		ASSERT(fl.IsReadable());
		ASSERT(fl.GetSyscallOpenMode()&O_RDWR);
		ASSERT(fl.GetSyscallOpenMode()&O_CREAT);
		ASSERT(fl.GetSyscallOpenMode()&O_TRUNC);
	}
}

void MmapTest::LargeWrite()
// what: check writing more than one mapped page
{
	if ( system::EnvGet("COAST_USE_MMAP_STREAMS").AsLong(1L) == 1L ) {
		system::io::unlink(fgcFilename);
		// note this can be a problem if more than one process runs this test
		OMmapStream os(fgcFilename);
		ASSERT(os.good());
		for (int i = 0; i < 1000; i ++) {
			os.width(5);
			os << i ;
			os.width(0);
			os << ":" << fgcContent << "\n";
		}
		ASSERT(os.good());
		os.close();
		os << "dummy"; // should not crash us here!
		ASSERT(!os.good()); // some error should occur, eof or fail or bad
		// should check if file really exists just with low level syscalls and fcts
		int fd = open(fgcFilename, O_RDONLY);
		ASSERT(fd >= 0);
		long filelen = lseek(fd, 0L, SEEK_END);
		ASSERT_EQUAL(1000 * (7 + strlen(fgcContent)), filelen);
		// do not check the content
		close(fd);
	}
} // LargeWrite

void MmapTest::SimpleWrite()
// what: check a few properties of a simple DoScramble-DoUnscramble cycle
{
	if ( system::EnvGet("COAST_USE_MMAP_STREAMS").AsLong(1L) == 1L ) {
		IntSimpleWrite();
	}
}

void MmapTest::IntSimpleWrite(int openmode)
// what: check a few properties of a simple DoScramble-DoUnscramble cycle
{
	system::io::unlink(fgcFilename);
	// note this can be a problem if more than one process runs this test
	OMmapStream os(fgcFilename, std::ios::out, openmode);
	ASSERT(os.good());
	os << fgcContent;
	os.close();
	os << "dummy"; // should not crash us here!
	ASSERT(!os.good()); // some error should occur, eof or fail or bad
	// should check if file really exists just with low level syscalls and fcts
	int fd = open(fgcFilename, O_RDONLY);
	ASSERT(fd >= 0);
	if ( fd > 0 ) {
		long filelen = lseek(fd, 0L, SEEK_END);
		ASSERT_EQUAL(strlen(fgcContent), filelen);
		char *buf = new char [filelen+1];
		lseek(fd, 0L, SEEK_SET);
		ASSERT_EQUAL(filelen, read(fd, buf, filelen));
		buf[filelen] = 0;
		ASSERT_EQUAL(fgcContent, buf);
		delete [] buf;
		close(fd);
	}

} // IntSimpleWrite

void MmapTest::SimpleRead()
// what:
{
	if ( system::EnvGet("COAST_USE_MMAP_STREAMS").AsLong(1L) == 1L ) {
		IntSimpleWrite(); // set up testfile
		IMmapStream is(fgcFilename);
		ASSERT(is.good());
		if (is.good()) {
			const char *pc = fgcContent;
			char c;
			do {
				ASSERT(!!is);
				is.get(c);
				if (is.eof()) {
					break;
				}
				char c1 = *pc++;
				ASSERT_EQUAL(c1, c); // compare content
			} while (true); //lint !e506
			ASSERT_EQUAL(pc - fgcContent, strlen(fgcContent)); // have we reached the end?
		}
		ASSERT(is.eof());
	}
}
void MmapTest::OperatorShiftLeftWithReadBuf()
{
	if ( system::EnvGet("COAST_USE_MMAP_STREAMS").AsLong(1L) == 1L ) {
		String testContent("hank");
		IStringStream is(testContent);
		ASSERT(is.good());
		OMmapStream os("testout2.txt");
		ASSERT(os.good());
		os << is.rdbuf();
		ASSERT(os.good());

		std::istream *is2 = system::OpenIStream("testout2", "txt");
		ASSERT(is2);

		OStringStream buf;
		buf << is2->rdbuf();
		ASSERT_EQUAL(testContent, buf.str());
	}
}

void MmapTest::EmptyFileRead()
// what:
{
	if ( system::EnvGet("COAST_USE_MMAP_STREAMS").AsLong(1L) == 1L ) {
		String mname("tmp/mmapEmptyFile");
		std::ostream *os0 = system::OpenOStream(mname, "tst");
		delete os0;

		mname << ".tst";
		MmapStream is(mname , (std::ios::in));
		ASSERT(is.good());
		if (is.good()) {
			char c;
			ASSERT(!!is);
			is.get(c);
			ASSERT(is.eof());
		}
		ASSERT(is.eof());
	}
}

void MmapTest::EmptyFilePutback()
// what:
{
	if ( system::EnvGet("COAST_USE_MMAP_STREAMS").AsLong(1L) == 1L ) {
		system::io::unlink(fgcFilename);
		// note this can be a problem if more than one process runs this test
		MmapStream ms(fgcFilename, std::ios::in | std::ios::out);

		ASSERT(ms.good());
		if (ms.good()) {
			ms << 'X';
			char c;
			ms.get(c);
			ASSERT(c == 'X');
			ms.putback('Y');
			ASSERT(ms.good());
			ms.get(c);
			ASSERT(c == 'Y');
			ms.get(c);
			ASSERT(ms.eof());
		}
	}
}

void MmapTest::FStreamEmptyFilePutback()
// what:
{
	if ( system::EnvGet("COAST_USE_MMAP_STREAMS").AsLong(1L) == 1L ) {
		system::io::unlink(fgcFilename);
		// note this can be a problem if more than one process runs this test
		std::fstream ms(fgcFilename, std::ios::in | std::ios::out);

		ASSERT(ms.good());
		if (ms.good()) {
			ms << 'X';
			char c;
			ms.putback('Y');
			ASSERT(!ms.eof());
			ms.get(c);// this fails, no explanation
			ASSERT(!ms.eof());
			ASSERT_EQUAL((long)'Y', long(c));
			ms.get(c);
			ASSERT(ms.eof());
		}
	}
}

void MmapTest::SimpleReadOnly()
// what:
{
	if ( system::EnvGet("COAST_USE_MMAP_STREAMS").AsLong(1L) == 1L ) {
		this->IntSimpleWrite(0600); // set up testfile, should fail for SunC++ 4.2
		IMmapStream is(fgcFilename);
		ASSERT(is.good());
		if (is.good()) {
			const char *pc = fgcContent;
			char c;
			do {
				ASSERT(!!is);
				is.get(c);
				if (is.eof()) {
					break;
				}
				char c1 = *pc++;
				ASSERT_EQUAL(c1, c);
			} while (true); //lint !e506 // compare content
			ASSERT_EQUAL(pc - fgcContent, strlen(fgcContent)); // have we reached the end?
		}
		ASSERT(is.eof());
	}
} // SimpleRead

void MmapTest::SimpleROPutback()
// what:
{
	if ( system::EnvGet("COAST_USE_MMAP_STREAMS").AsLong(1L) == 1L ) {
		this->IntSimpleWrite(); // set up testfile
		IMmapStream is(fgcFilename);
		ASSERT(is.good());
		char c;
		String s;

		for (int i = 0; i < 10 ; i++) {
			is.get(c);
			s.Append(c);
		}
		ASSERT_EQUAL("This is th", s);
		is.putback(c);
		ASSERTM("stream good after putback",!!is);
		char c2;
		is.get(c2);
		ASSERT_EQUAL(c2, c);
	}
} // SimplePutback

void MmapTest::SimplePutback()
// what:
{
	if ( system::EnvGet("COAST_USE_MMAP_STREAMS").AsLong(1L) == 1L ) {
		this->IntSimpleWrite(); // set up testfile
		MmapStream is(fgcFilename, std::ios::in | std::ios::out | std::ios::app); // do not trunc the file
		ASSERT(is.good());
		char c;
		String s;

		for (int i = 0; i < 10 ; i++) {
			ASSERT(! !is.get(c));
			s.Append(c);
		}
		ASSERT_EQUAL("This is th", s);
		is.putback(c);
		ASSERTM("stream good after putback",!!is);
		char c2;
		is.get(c2);
		ASSERT_EQUAL(c2, c);
		// add more putbacks to really test it
		is.putback(' ');
		is.putback('\n');
		ASSERTM("stream good after double putback",!!is);
		is.get(c);
		is.get(c2);
		ASSERT_EQUAL('\n', c);
		ASSERT_EQUAL(' ', c2);
	}
} // SimplePutback

void MmapTest::SimpleAppend()
// what:
{
	if ( system::EnvGet("COAST_USE_MMAP_STREAMS").AsLong(1L) == 1L ) {
		this->IntSimpleWrite(); // set up testfile
		OMmapStream os(fgcFilename, std::ios::app);
		ASSERT(os.good());
		os << fgcContent;
		os.close();
		int fd = open(fgcFilename, O_RDONLY);
		ASSERT(fd >= 0);
		long filelen = lseek(fd, 0L, SEEK_END);
		ASSERT_EQUAL(2 * strlen(fgcContent), filelen);
		close(fd);
		system::io::unlink(fgcFilename);
	}
} // SimpleAppend

void MmapTest::SimpleTruncate()
// what:
{
	if ( system::EnvGet("COAST_USE_MMAP_STREAMS").AsLong(1L) == 1L ) {
		this->IntSimpleWrite(); // set up testfile
		MmapStream os(fgcFilename, std::ios::in | std::ios::out); // this truncates the file
		ASSERT(os.good());
		os << "Hallo";
		os.close();
		int fd = open(fgcFilename, O_RDONLY);
		ASSERT(fd >= 0);
		long filelen = lseek(fd, 0L, SEEK_END);
		ASSERT_EQUAL(strlen("Hallo"), filelen);
		close(fd);
		system::io::unlink(fgcFilename);
	}
} // SimpleAppend

void MmapTest::SimulatedLogAppend()
// what:
{
	if ( system::EnvGet("COAST_USE_MMAP_STREAMS").AsLong(1L) == 1L ) {
		String logfilename(fgcFilename);
		logfilename << ".log";
		system::io::unlink(logfilename);

		OMmapStream os(logfilename, std::ios::app, MmapStreamBuf::openprot,
					   MmapStreamBuf::eSync); // eSync is default
		ASSERT(os.good());
		for (int j = 0; j < 2; j++) {
			for (int i = 0; i < 5; i++) {
				os << j << ", " << i << fgcContent << std::endl; // endl should sync implicitely
			}
		}
		os.close();
		system::io::unlink(logfilename); // clean up afterwards
	}
} // SimpleAppend

void MmapTest::SimpleAtEnd()
// what:
{
	if ( system::EnvGet("COAST_USE_MMAP_STREAMS").AsLong(1L) == 1L ) {
		this->IntSimpleWrite(); // set up testfile
		OMmapStream os(fgcFilename, std::ios::ate);
		ASSERT(os.good());
		os << fgcContent;
		os.close();
		int fd = open(fgcFilename, O_RDONLY);
		ASSERT(fd >= 0);
		long filelen = lseek(fd, 0L, SEEK_END);
		ASSERT_EQUAL(2 * strlen(fgcContent), filelen);
		close(fd);
		system::io::unlink(fgcFilename);
	}
} // SimpleAtEnd

void MmapTest::SimpleSeek()
// what:
{
	if ( system::EnvGet("COAST_USE_MMAP_STREAMS").AsLong(1L) == 1L ) {
		const long searchpos = 10000; // larger as a memory page
		this->IntSimpleWrite(); // set up testfile
		OMmapStream os(fgcFilename);
		ASSERT(os.good());
		os.seekp(searchpos); // force enlargement
		os << fgcContent ;
		os.close();
		int fd = open(fgcFilename, O_RDONLY);
		ASSERT(fd >= 0);
		long filelen = lseek(fd, 0L, SEEK_END);
		ASSERT_EQUAL(searchpos + strlen(fgcContent), filelen);
		close(fd);
		system::io::unlink(fgcFilename);
	}
} // SimpleSeek
#endif
