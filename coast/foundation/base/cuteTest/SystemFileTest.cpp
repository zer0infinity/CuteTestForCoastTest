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

#include "SystemFileTest.h"
#include "SystemLog.h"
#include "boost/bind.hpp"

using namespace coast;

void SystemFileTest::initPathTest() {
	String pathList(system::GetPathList());
	String rootDir(system::GetRootDir());

	ASSERT(pathList.Length() > 0);
	ASSERT(rootDir.Length() > 0);

	const char tmpRoot[] = "/usr/local";
	const char tmpPath[] = "bin";
	system::InitPath(tmpRoot, tmpPath);

	ASSERT_EQUAL("/usr/local", system::GetRootDir());
	ASSERT_EQUAL("bin", system::GetPathList());

#if defined(__linux)
	unsetenv("COAST_ROOT");
	unsetenv("COAST_PATH");
#else
	putenv("COAST_ROOT=");
	putenv("COAST_PATH=");
#endif
	// should have no effect now, path and root should remain
	system::InitPath();

	ASSERT_EQUAL("/usr/local", system::GetRootDir());
	ASSERT_EQUAL("bin", system::GetPathList());

	// test InitPath with root argument, path will be restored to default ".:config:src:"
	system::InitPath("/usr/local/bin");
	ASSERT_EQUAL("/usr/local/bin", system::GetRootDir());
	// path will be restored to ".:config:src:"
	ASSERT_EQUAL(".:config:src:", system::GetPathList());

	// test InitPath with path argument, root will be restored to default "."
	system::InitPath(0, "config");

	ASSERT_EQUAL(".", system::GetRootDir());
	ASSERT_EQUAL("config", system::GetPathList());

#if defined(WIN32)
	// test InitPath with NT specific arguments
	system::InitPath("d:\\gugus/blabla", "bin");

	ASSERT_EQUAL("d:/gugus/blabla", system::GetRootDir());
	ASSERT_EQUAL("bin", system::GetPathList());
#endif

	// restore old settings
	system::SetPathList(pathList, false);
	system::SetRootDir(rootDir, false);
	ASSERT_EQUAL(rootDir, system::GetRootDir());
	ASSERT_EQUAL(pathList, system::GetPathList());
}

void SystemFileTest::statTests() {
	ASSERTM("expected '.' to be a directory", system::IsDirectory("."));
	ASSERTM("expected '.' to be a directory", system::IsDirectory(".."));
	ASSERTM("expected 'Test.any' to be a file", !system::IsDirectory("config/Test.any"));
	ASSERTM("expected 'Tracer.any' to be a file", !system::IsDirectory("config/Tracer.any"));
	ASSERTM("expected '.' to be a directory", !system::IsRegularFile("."));
	ASSERTM("expected 'SystemTest.any' to be a file", system::IsRegularFile("config/SystemFileTest.any"));
	ASSERTM("expected '.' to be a directory", !system::IsRegularFile(".."));
	ASSERTM("expected 'Test.any' to be a file", system::IsRegularFile("config/Test.any"));
	ASSERTM("expected 'Tracer.any' to be a file", system::IsRegularFile("config/Tracer.any"));
	String strLinkToPrjRunTest("aLinkToTestAny");
#if !defined(WIN32)
	ASSERT_EQUALM("expected creation of symbolic link to file to succeed", system::eSuccess, system::CreateSymbolicLink("config/Test.any", strLinkToPrjRunTest));
		ASSERTM("expected link to be valid", system::IsSymbolicLink(strLinkToPrjRunTest));
		ASSERTM("expected link not to be a directory", !system::IsDirectory(strLinkToPrjRunTest));
		ASSERTM("expected link to point to a regular file", system::IsRegularFile(strLinkToPrjRunTest));
		ASSERT_EQUALM("expected removal of symbolic link to succeed" , system::eSuccess, system::RemoveDirectory(strLinkToPrjRunTest) );

	String strLinkToDirectory("aLinkToDirectory");
	ASSERT_EQUALM("expected creation of symbolic link to file to succeed", system::eSuccess, system::CreateSymbolicLink("config", strLinkToDirectory));
		ASSERTM("expected link to be valid", system::IsSymbolicLink(strLinkToDirectory));
		ASSERTM("expected link to point to a directory", system::IsDirectory(strLinkToDirectory));
		ASSERTM("expected link not to be a regular file", !system::IsRegularFile(strLinkToDirectory));
		ASSERT_EQUALM("expected removal of symbolic link to succeed" , system::eSuccess, system::RemoveDirectory(strLinkToDirectory) );

#endif
}

void SystemFileTest::pathListTest() {
	String pathList(system::GetPathList());

	ASSERT(pathList.Length() > 0);
	String newPath("/:/usr:/usr/local");

	system::SetPathList(newPath, false);

	ASSERT_EQUAL(newPath, system::GetPathList());

	// restore old pathList
	system::SetPathList(pathList, false);
}

void SystemFileTest::rooDirTest() {
	String rootDir(system::GetRootDir());

	ASSERT(rootDir.Length() > 0);

	String newRoot("/");

	system::SetRootDir(newRoot, false);

	ASSERT_EQUAL(newRoot, system::GetRootDir());

	// restore old root dir
	system::SetRootDir(rootDir, false);
}

void SystemFileTest::CWDTests() {
	String wd;
	system::GetCWD(wd);
	ASSERTM("working dir should be absolute", system::IsAbsolutePath(wd));
	ASSERTM("working dir should not be empty", wd.Length() > 0);
	ASSERTM("working dir should not be default .", wd != ".");

	String upper = wd.SubString(0, wd.StrRChr(system::Sep()));
	ASSERTM("shouldn't fail, .. dir always accessible", system::ChangeDir(".."));
	ASSERTM("should have upper directory", upper.Length() > 0);
	ASSERT_EQUAL(0L, wd.Contains(upper));
	ASSERTM("should really be upper if .. isn't mount point", upper.Length() < wd.Length());

	String u;
	system::GetCWD(u);
	ASSERT_EQUAL(upper, u);
	ASSERTM("huh, directory empty", u.Length() > 0);
	ASSERTM("working dir should not be default .", u != ".");

	String strCwd;
	system::GetCWD(strCwd);
	String root;
	root << system::Sep();
	ASSERTM("shouldn't fail, root dir always accessible", system::ChangeDir(root));
	String r1;
	system::GetCWD(r1);
#if defined(WIN32)
	// on WIN32, if we change to 'root' it is actually the root Dir of the current drive
	// in the form <drive>:<system::SEP>
	char driveLetter;
	ASSERTM(std::string("couldn't get drive letter from [") << strCwd << "]", system::GetDriveLetter(strCwd, driveLetter));
	root = "";
	root << driveLetter << ":" << system::Sep();
	ASSERT_EQUAL(root, r1);
#else
	ASSERT_EQUAL(root, r1);
#endif
	ASSERTM("huh, directory empty", r1.Length() > 0);
	ASSERTM("working dir should not be default .", r1 != ".");

	String invaliddir;
	invaliddir << system::Sep() << "wuggi" << system::Sep() << "waggi" << system::Sep() << "gugus" << system::Sep() << "this"
			<< system::Sep() << "directory" << system::Sep() << "doesnt" << system::Sep() << "exist";
	ASSERTM("should fail, dir never exists", !system::ChangeDir(invaliddir));
	String r2;
	system::GetCWD(r2);
	ASSERT_EQUAL(root, r2);
	ASSERTM("huh, directory empty", r2.Length() > 0);
	ASSERTM("working dir should not be default .", r2 != ".");

	//-- clean up
	ASSERTM("shouldn't fail, .. original dir always accessible", system::ChangeDir(wd));
	String w1;
	system::GetCWD(w1);
	ASSERT_EQUAL(wd, w1);
	ASSERTM("huh, directory empty", w1.Length() > 0);
	ASSERTM("working dir should not be default .", w1 != ".");
}

void SystemFileTest::IsAbsolutePathTest() {
	ASSERTM("should be absolute", system::IsAbsolutePath("/"));
	ASSERTM("should be absolute", system::IsAbsolutePath("//"));
	ASSERTM("should be absolute", system::IsAbsolutePath("/blabla/"));
	ASSERTM("should not be absolute", !system::IsAbsolutePath("./"));
	ASSERTM("should not be absolute", !system::IsAbsolutePath("../"));
	ASSERTM("should not be absolute", !system::IsAbsolutePath("config"));
#if defined(WIN32)
	ASSERTM("should be absolute", system::IsAbsolutePath("\\"));
	ASSERTM("should be absolute", system::IsAbsolutePath("\\gaga"));
	// 'normal' WIN32 drive notation, is NOT absolute, uses current dir on drive
	ASSERTM("should not be absolute", !system::IsAbsolutePath("d:"));
	// 'normal' WIN32 drive notation with absolute path
	ASSERTM("should be absolute", system::IsAbsolutePath("d:\\"));
	// 'normal' WIN32 drive notation with absolute path
	ASSERTM("should be absolute", system::IsAbsolutePath("d:/"));
	// some shells use the following absolute notation
	ASSERTM("should be absolute", system::IsAbsolutePath("//d/"));
	ASSERTM("should be absolute", system::IsAbsolutePath("/d/"));
#endif
}

void SystemFileTest::ResolvePathTest() {
	String result, expected;
	result = "";
	system::ResolvePath(result);
	expected = "";
	ASSERT_EQUAL(expected, result);
	result = "/";
	system::ResolvePath(result);
	expected = "/";
	ASSERT_EQUAL(expected, result);
	result = ".";
	system::ResolvePath(result);
	expected = "";
	ASSERT_EQUAL(expected, result);
	result = "./";
	system::ResolvePath(result);
	expected = "";
	ASSERT_EQUAL(expected, result);
	result = "..";
	system::ResolvePath(result);
	expected = "";
	ASSERT_EQUAL(expected, result);
	result = "../";
	system::ResolvePath(result);
	expected = "";
	ASSERT_EQUAL(expected, result);
	result = "/../";
	system::ResolvePath(result);
	expected = "/";
	ASSERT_EQUAL(expected, result);
	result = "/../.";
	system::ResolvePath(result);
	expected = "/";
	ASSERT_EQUAL(expected, result);
	result = "gugus";
	system::ResolvePath(result);
	expected = "gugus";
	ASSERT_EQUAL(expected, result);
	result = "./gugus";
	system::ResolvePath(result);
	expected = "gugus";
	ASSERT_EQUAL(expected, result);
	result = "././gugus";
	system::ResolvePath(result);
	expected = "gugus";
	ASSERT_EQUAL(expected, result);
	result = "././././././gugus";
	system::ResolvePath(result);
	expected = "gugus";
	ASSERT_EQUAL(expected, result);
	result = "././././././";
	system::ResolvePath(result);
	expected = "";
	ASSERT_EQUAL(expected, result);
	result = "../../../../abc";
	system::ResolvePath(result);
	expected = "abc";
	ASSERT_EQUAL(expected, result);
	result = "./gugus/..";
	system::ResolvePath(result);
	expected = "";
	ASSERT_EQUAL(expected, result);
	result = "./gugus/../";
	system::ResolvePath(result);
	expected = "";
	ASSERT_EQUAL(expected, result);
	result = "./gugus/../..";
	system::ResolvePath(result);
	expected = "";
	ASSERT_EQUAL(expected, result);
	result = "./gugus/../../blabla";
	system::ResolvePath(result);
	expected = "blabla";
	ASSERT_EQUAL(expected, result);
	result = "./gugus/../../blabla/..";
	system::ResolvePath(result);
	expected = "";
	ASSERT_EQUAL(expected, result);
	result = "./gugus/../../blabla/../";
	system::ResolvePath(result);
	expected = "";
	ASSERT_EQUAL(expected, result);
	result = "./gugus/../../..";
	system::ResolvePath(result);
	expected = "";
	ASSERT_EQUAL(expected, result);
	result = "/bla/../../huhu/./ga/";
	system::ResolvePath(result);
	expected = "/huhu/ga/";
	ASSERT_EQUAL(expected, result);
	result = "./gugus/hallo.txt/";
	system::ResolvePath(result);
	expected = "gugus/hallo.txt/";
	ASSERT_EQUAL(expected, result);
	result = "./gugus./hallo.txt./";
	system::ResolvePath(result);
	expected = "gugus./hallo.txt./";
	ASSERT_EQUAL(expected, result);
	result = "/gugus/../..";
	system::ResolvePath(result);
	expected = "/";
	ASSERT_EQUAL(expected, result);
	result = " .";
	system::ResolvePath(result);
	expected = " .";
	ASSERT_EQUAL(expected, result);
	result = "/gugus/../..";
	result.PutAt(4, '\0');
	system::ResolvePath(result);
	expected = "/gug";
	ASSERT_EQUAL(expected, result);
	result = "/gugus/../..";
	result.PutAt(0, '\0');
	system::ResolvePath(result);
	expected = "";
	ASSERT_EQUAL(expected, result);
	result = "/gugus/../..";
	result.PutAt(result.Length() - 1L, '\0');
	system::ResolvePath(result);
	expected = "/";
	ASSERT_EQUAL(expected, result);
	result = "........";
	system::ResolvePath(result);
	expected = "........";
	ASSERT_EQUAL(expected, result);
	result = "./////";
	system::ResolvePath(result);
	expected = "";
	ASSERT_EQUAL(expected, result);
	result = "/foo/bla/stop";
	system::ResolvePath(result);
	expected = "/foo/bla/stop";
	ASSERT_EQUAL(expected, result);
#if defined(WIN32)
	result = "\\";
	system::ResolvePath(result);
	expected = "/";
	ASSERT_EQUAL(expected, result);
	result = ".\\";
	system::ResolvePath(result);
	expected = "";
	ASSERT_EQUAL(expected, result);
	result = "d:";
	system::ResolvePath(result);
	expected = "d:";
	ASSERT_EQUAL(expected, result);
	result = " :";
	system::ResolvePath(result);
	expected = " :";
	ASSERT_EQUAL(expected, result);
	result = "d:/";
	system::ResolvePath(result);
	expected = "d:/";
	ASSERT_EQUAL(expected, result);
	result = "d:\\";
	system::ResolvePath(result);
	expected = "d:/";
	ASSERT_EQUAL(expected, result);
	result = "//d/";
	system::ResolvePath(result);
	expected = "d:/";
	ASSERT_EQUAL(expected, result);
	result = "d:/gugus";
	system::ResolvePath(result);
	expected = "d:/gugus";
	ASSERT_EQUAL(expected, result);
	result = "d:./";
	system::ResolvePath(result);
	expected = "d:";
	ASSERT_EQUAL(expected, result);
	result = "d:./gugus";
	system::ResolvePath(result);
	expected = "d:gugus";
	ASSERT_EQUAL(expected, result);
	result = "d:./gugus/..";
	system::ResolvePath(result);
	expected = "d:";
	ASSERT_EQUAL(expected, result);
	result = "d:./gugus/../..";
	system::ResolvePath(result);
	expected = "d:";
	ASSERT_EQUAL(expected, result);
	result = "d:./gugus/bla/hu";
	system::ResolvePath(result);
	expected = "d:gugus/bla/hu";
	ASSERT_EQUAL(expected, result);
	result = "d:./gugus\\bla/hu\\";
	system::ResolvePath(result);
	expected = "d:gugus/bla/hu/";
	ASSERT_EQUAL(expected, result);
	result = "d:./\\/\\";
	system::ResolvePath(result);
	expected = "d:";
	ASSERT_EQUAL(expected, result);
	result = "d:./\\/\\/";
	system::ResolvePath(result);
	expected = "d:";
	ASSERT_EQUAL(expected, result);
	result = "d:..";
	system::ResolvePath(result);
	expected = "d:";
	ASSERT_EQUAL(expected, result);
	result = "d:/..";
	system::ResolvePath(result);
	expected = "d:/";
	ASSERT_EQUAL(expected, result);
	result = "d:./gugus/../gugus";
	system::ResolvePath(result);
	expected = "d:gugus";
	ASSERT_EQUAL(expected, result);
	result = "d::";
	system::ResolvePath(result);
	expected = "d::";
	ASSERT_EQUAL(expected, result);
	result = "d:/:";
	system::ResolvePath(result);
	expected = "d:/:";
	ASSERT_EQUAL(expected, result);
	result = "d:........";
	system::ResolvePath(result);
	expected = "d:........";
	ASSERT_EQUAL(expected, result);
	result = "d:d:";
	system::ResolvePath(result);
	expected = "d:d:";
	ASSERT_EQUAL(expected, result);
	result = ":";
	system::ResolvePath(result);
	expected = ":";
	ASSERT_EQUAL(expected, result);
	result = ":.";
	system::ResolvePath(result);
	expected = ":.";
	ASSERT_EQUAL(expected, result);
#endif
}

void SystemFileTest::OpenStreamTest() {
	std::iostream *Ios = system::OpenStream("Tracer.any");
	ASSERT( Ios == NULL ); // should not be found!
	if (Ios) {
		delete Ios;
	}

	// open file with relative path
	Ios = system::OpenStream("config/Tracer.any");
	ASSERT( Ios != NULL );
	if (Ios) {
		delete Ios;
	}

	// deprecated:

	// search file with path
	Ios = system::OpenStream("Tracer", "any");
	ASSERT( Ios != NULL ); // should be found
	if (Ios) {
		delete Ios;
	}

	// open file with relative path
	Ios = system::OpenStream("config/Tracer", "any");
	ASSERT( Ios != NULL );
	if (Ios) {
		delete Ios;
	}

	// open file with relative path for writing
	Ios = system::OpenStream("tmp/Test1", "tst", std::ios::out);
	ASSERT( Ios != NULL );
	if (Ios) {
		(*Ios) << "test" << std::endl;
		ASSERT(!!(*Ios));
		delete Ios;
	}
}

void SystemFileTest::OpenStreamWithSearchTest() {
	std::iostream *Ios = system::OpenStreamWithSearch("Tracer.any");
	ASSERT( Ios != NULL );
	if (Ios) {
		delete Ios;
	}

	// open file with relative path
	Ios = system::OpenStreamWithSearch("config/Tracer.any");
	ASSERT( Ios != NULL );
	if (Ios) {
		delete Ios;
	}

	// open file with relative path for writing
	Ios = system::OpenStreamWithSearch("tmp/Test1.tst", std::ios::out);
	ASSERT( Ios != NULL );
	if (Ios) {
		(*Ios) << "test" << std::endl;
		ASSERT(!!(*Ios));
		delete Ios;
	}
}

void SystemFileTest::OpenIStreamTest() {
	// open file with relative path
	std::iostream *Ios = system::OpenIStream("config/Tracer.any");

	ASSERT( Ios != NULL );
	if (Ios) {
		Anything dbgTest;
		(*Ios) >> dbgTest;
		ASSERT(!!(*Ios));
		ASSERT(dbgTest.GetSize() > 0);
		delete Ios;
	}

	// deprecated:

	// open file with relative path
	Ios = system::OpenIStream("config/Tracer", "any");

	ASSERT( Ios != NULL );
	if (Ios) {
		Anything dbgTest;
		(*Ios) >> dbgTest;
		ASSERT(!!(*Ios));
		ASSERT(dbgTest.GetSize() > 0);
		delete Ios;
	}
}

void SystemFileTest::OpenOStreamTest() {
	// open file with relative path for writing
	std::iostream *Ios = system::OpenOStream("tmp/Test.tst");

	ASSERT( Ios != NULL );
	if (Ios) {
		(*Ios) << "test" << std::endl;
		ASSERT(!!(*Ios));
		delete Ios;
	}

	// deprecated:

	// open file with relative path for writing
	Ios = system::OpenOStream("tmp/Test", "tst");

	ASSERT( Ios != NULL );
	if (Ios) {
		(*Ios) << "test" << std::endl;
		ASSERT(!!(*Ios));
		delete Ios;
	}
}

void SystemFileTest::testGetFilePath(boost::function<String()> func, const String& notFoundResult) {
	StartTrace(SystemFileTest.testGetFilePath);
	String subPath("./config/Tracer.any");

	system::Chmod(subPath, 0400); // set it read only

	String path(func());

	system::ResolvePath(subPath);
	ASSERT_EQUAL(subPath, path.SubString(path.Length() - subPath.Length()));

#if defined(WIN32)
	// because NT lacks easy hiding of files or directories the file is renamed
	String hiddenSubPath;
	long filenamePos = subPath.StrRChr( coast::system::Sep() );
	if (filenamePos > -1) {
		hiddenSubPath = subPath.SubString(0L, filenamePos+1).Append(".").Append(subPath.SubString(filenamePos+2));
	} else {
		hiddenSubPath = subPath;
	}

	int status = system::io::rename(subPath, hiddenSubPath);
#else
	system::Chmod(subPath, 0000); // set it to no access
#endif

	path = func();
	// should fail now.... therefore path is equal
	ASSERT_EQUAL(notFoundResult, path);

#if defined(WIN32)
	system::io::rename(hiddenSubPath, subPath); //clean up to make it usable again
#else
	system::Chmod(subPath, 0640); // clean up to make it usable again
#endif

	path = func();
	subPath = "./Tracer.any";
	system::ResolvePath(subPath);
	ASSERT_EQUAL(subPath, path.SubString(path.Length() - subPath.Length()));
}

void SystemFileTest::GetFilePathTest() {
	testGetFilePath(boost::bind(&coast::system::GetFilePath, "Tracer", "any"), ""); // deprecated
	testGetFilePath(boost::bind(&coast::system::GetFilePath, "Tracer.any"), "");
	testGetFilePath(boost::bind(&coast::system::GetFilePathOrInput, "Tracer.any"), "Tracer.any");
}

void SystemFileTest::dirFileListTest() {
	StartTrace(SystemFileTest.dirFileListTest);
	Anything dir(system::DirFileList("."));

	ASSERT_EQUAL( 0L, dir.GetSize() );

	dir = system::DirFileList("config", "any");
	ASSERT( dir.GetSize() > 0L );

	dir = system::DirFileList("..", "");
	ASSERT( dir.GetSize() > 0L );
	TraceAny(dir, "entries of [..]");
}

//==============================================================================================
//        O P . I N P U T             Beginn
//==============================================================================================
void SystemFileTest::IStreamTest() {
	// istream& operator>>(istream& is, String& s)
	// String noch nicht belegt: Capacity gesetzt (groesser als die Dateilaenge) und Inhalt gesetzt.
	// String schon belegt:  Der Inhalt des Strings wird reset (Capacity NICHT) und neu gesetzt.
	//-------------------------------------------------------------------------------------------------

	String str0;
	std::istream *is0 = system::OpenStream("len5", "tst");
	if (is0) {
		*is0 >> str0;
		//PS? ASSERT ( str0.Capacity() == 66 );
		ASSERT ( str0.Length() == 5 );
		ASSERT ( str0.Capacity() >= str0.Length() );
		ASSERT ( (long)strlen( (const char *)str0 ) == 5 );
		ASSERT ( strcmp( (const char *)str0, "01234" ) == 0 );

		*is0 >> str0;
		//PS? ASSERT ( str0.Capacity() == 66 );
		ASSERT ( str0.Length() == 0 );
		ASSERT ( str0.Capacity() >= str0.Length() );
		ASSERT ( (long)strlen( (const char *)str0 ) == 0 );
		ASSERT ( strcmp( (const char *)str0, "" ) == 0 );

#if !defined(WIN32)
		// win32 istream hangs if read at file end is attempted! (FIXME?)
		*is0 >> str0;
		//PS? ASSERT ( str0.Capacity() == 66 );
		ASSERT ( str0.Length() == 0 );
		ASSERT ( str0.Capacity() >= str0.Length() );
		ASSERT ( (long)strlen( (const char *)str0 ) == 0 );
		ASSERT ( strcmp( (const char *)str0, "" ) == 0 );

		*is0 >> str0;
		//PS? ASSERT ( str0.Capacity() == 66 );
		ASSERT ( str0.Length() == 0 );
		ASSERT ( str0.Capacity() >= str0.Length() );
		ASSERT ( (long)strlen( (const char *)str0 ) == 0 );
		ASSERT ( strcmp( (const char *)str0, "" ) == 0 );
#endif
		delete is0;
	} else {
		ASSERT_EQUAL("'read file len5.tst'", "'could not read len5.tst'");
	}

	// Kombination von 2 ">>":  das zweite ">>" loescht das Resultat vom ersten ">>" und berechnet es neu.
	//---------------------------------------------------------------------------------------------------------
	String str1;
	std::istream *is1 = system::OpenStream("len5", "tst");
	if (is1) {
		*is1 >> str1;
		//PS? ASSERT (str1.Capacity() == 66 );
		ASSERT (str1.Length() == 5);
		ASSERT ( str1.Capacity() >= str1.Length() );
		ASSERT ( strlen( (const char *)str1 ) == 5);
		ASSERT( memcmp( (const char *)str1, "01234", (long)strlen((const char *)str1) ) == 0 );

		delete is1;
	} else {
		ASSERT_EQUAL("'read file len5.tst'", "'could not read len5.tst'");
	}
	std::istream *is2 = system::OpenStream("len5", "tst");
	if (is2) {
		*is2 >> str1;
		//PS? ASSERT (str1.Capacity() == 66 );
		ASSERT (str1.Length() == 5);
		ASSERT ( str1.Capacity() >= str1.Length() );
		ASSERT ( strlen( (const char *)str1 ) == 5);
		ASSERT( memcmp( (const char *)str1, "01234", strlen((const char *)str1) ) == 0 );
		// und nicht 0123401234.  Ist es gewuenscht ????
		delete is2;
	} else {
		ASSERT_EQUAL("'read file len5.tst'", "'could not read len5.tst'");
	}

	String str3 = "qwertzuiopasdfghjklyxcvbnm";
	std::istream *is3 = system::OpenStream("len5", "tst");
	if (is3) {
		*is3 >> str3;
		//PS? ASSERT (str3.Capacity() == 54 );
		ASSERT (str3.Length() == 5);
		ASSERT (str3.Capacity() >= str3.Length() );
		ASSERT ( memcmp( (const char *)str3, "01234", strlen((const char *)str3) ) == 0 );
		ASSERT ( strlen( (const char *)str3 ) == 5);
		ASSERT( memcmp( (const char *)str3, "01234", strlen((const char *)str3) ) == 0 );
		// und nicht qwe...bnm91234.  Ist es gewuenscht ????
		delete is3;
	} else {
		ASSERT_EQUAL("'read file len5.tst'", "'could not read len5.tst'");
	}
}
//==============================================================================================
//        O P . I N P U T             Ende
//==============================================================================================

//==============================================================================================
//        O P . O U T P U T             Beginn
//==============================================================================================
void SystemFileTest::OStreamTest() {
	//	ostream& operator<<(ostream& os, const String &s)
	//-------------------------------------------------------------
	String str0 = "", str00 = "";
	String sname("tmp/emptyStr");
	std::ostream *os0 = system::OpenOStream(sname, "tst");
	std::string msg("couldn't open ");
	msg += system::GetFilePath(sname, "tst");

	ASSERTM(msg, os0 != 0);
	if (os0) {
		*os0 << str0;
		delete os0;
	} else {
		ASSERT_EQUAL("'write to file tmp/emptyStr.tst'", "'could not write tmp/emptyStr.tst'");
	}

	std::istream *is0 = system::OpenStream("tmp/emptyStr", "tst");

	if (is0) {
		*is0 >> str00;
		delete is0;
		ASSERT( str0 == str00 );
		ASSERT_EQUAL( str0.Length(), str00.Length() );
		// ASSERT( str0.Capacity() == str00.Capacity() );
		ASSERT( str0.Capacity() >= str0.Length() );
		ASSERT( str00.Capacity() >= str00.Length() );
		ASSERT( memcmp( (const char *)str0, (const char *)str00, str0.Length() ) == 0 );
	} else {
		ASSERT_EQUAL("'read file tmp/emptyStr.tst'", "'could not read tmp/emptyStr.tst'");
	}

	String str1, str11;
	str1.Append("0123456789");
	std::ostream *os1 = system::OpenOStream("tmp/zahlen", "tst");

	if (os1) {
		*os1 << str1;
		delete os1;
	} else {
		ASSERT_EQUAL("'write to file tmp/zahlen.tst'", "'could not write tmp/zahlen.tst'");
	}
	std::istream *is1 = system::OpenStream("tmp/zahlen", "tst");

	if (is1) {
		*is1 >> str11;
		delete is1;
		ASSERT( str1 == str11 );
		ASSERT_EQUAL( str1.Length(), str11.Length() );
		// ASSERT( str1.Capacity() == str11.Capacity() );
		ASSERT( str1.Capacity() >= str1.Length() );
		ASSERT( str11.Capacity() >= str11.Length() );
		ASSERT( memcmp( (const char *)str1, (const char *)str11, str1.Length() ) == 0 );
	} else {
		ASSERT_EQUAL("'read file tmp/zahlen.tst'", "'could not read tmp/zahlen.tst'");
	}

	String str2, str22;
	str2.Append("qwertzuiopasdfghjklyxcvbnm");
	std::ostream *os2 = system::OpenOStream("tmp/buchst", "tst");

	if (os2) {
		*os2 << str2;
		delete os2;
	} else {
		ASSERT_EQUAL("'write to file tmp/buchst.tst'", "'could not write tmp/buchst.tst'");
	}
	std::istream *is2 = system::OpenStream("tmp/buchst", "tst");

	if (is2) {
		*is2 >> str22;
		delete is2;
		ASSERT( str2 == str22 );
		ASSERT_EQUAL( str2.Length(), str22.Length() );
		// ASSERT( str2.Capacity() == str22.Capacity() );
		ASSERT( str2.Capacity() >= str2.Length() );
		ASSERT( str22.Capacity() >= str22.Length() );
		ASSERT( memcmp( (const char *)str2, (const char *)str22, str2.Length() ) == 0 );
	} else {
		ASSERT_EQUAL("'read file tmp/buchst.tst'", "'could not read tmp/buchst.tst'");
	}

	//	unsafe_ostream& operator<<(unsafe_ostream& os, const String &s)
	//-----------------------------------------------------------------
	String str0u = "", str00u;
	std::ostream *os0u = system::OpenOStream("tmp/emptyStrU", "tst");

	if (os0u) {
		*os0u << str0u;
		delete os0u;
	} else {
		ASSERT_EQUAL("'write to file tmp/emptyStrU.tst'", "'could not write tmp/emptyStrU.tst'");
	}

	std::istream *is0u = system::OpenStream("tmp/emptyStrU", "tst");

	if (is0u) {
		*is0u >> str00u;
		delete is0u;
		ASSERT( str0u == str00u );
		ASSERT_EQUAL( str0u.Length(), str00u.Length() );
		// ASSERT( str0u.Capacity() == str00u.Capacity() );
		ASSERT( str0u.Capacity() >= str0u.Length() );
		ASSERT( str00u.Capacity() >= str00u.Length() );
		ASSERT( memcmp( (const char *)str0u, (const char *)str00u, str0u.Length() ) == 0 );
	} else {
		ASSERT_EQUAL("'read file tmp/emptyStrU.tst'", "'could not read tmp/emptyStrU.tst'");
	}

	String str1u;
	str1u.Append("0123456789");
	std::ostream *os1u = system::OpenOStream("tmp/zahlenU", "tst");

	if (os1u) {
		*os1u << str1u;
		delete os1u;
	} else {
		ASSERT_EQUAL("'write to file tmp/zahlenU.tst'", "'could not write tmp/zahlenU.tst'");
	}
	std::istream *is1u = system::OpenStream("tmp/zahlenU", "tst");

	if (is1u) {
		String str11u;
		*is1u >> str11u;
		delete is1u;
		ASSERT( str1u == str11u );
		ASSERT_EQUAL( str1u.Length(), str11u.Length() );
		// ASSERT( str1u.Capacity() == str11u.Capacity() );
		ASSERT( str1u.Capacity() >= str1u.Length() );
		ASSERT( str11u.Capacity() >= str11u.Length() );
		ASSERT( memcmp( (const char *)str1u, (const char *)str11u, str1u.Length() ) == 0 );
	} else {
		ASSERT_EQUAL("'read file tmp/zahlenU.tst'", "'could not read tmp/zahlenU.tst'");
	}

	String str2u;
	str2u.Append("qwertzuiopasdfghjklyxcvbnm");
	std::ostream *os2u = system::OpenOStream("tmp/buchstU", "tst");

	if (os2u) {
		*os2u << str2u;
		delete os2u;
	} else {
		ASSERT_EQUAL("'write to file tmp/buchstU.tst'", "'could not write tmp/buchstU.tst'");
	}

	std::istream *is2u = system::OpenStream("tmp/buchstU", "tst");

	if (is2u) {
		String str22u;
		*is2u >> str22u;
		delete is2u;
		ASSERT( str2u == str22u );
		ASSERT_EQUAL( str2u.Length(), str22u.Length() );
		// ASSERT( str2u.Capacity() == str22u.Capacity() );
		ASSERT( str2u.Capacity() >= str2u.Length() );
		ASSERT( str22u.Capacity() >= str22u.Length() );
		ASSERT( memcmp( (const char *)str2u, (const char *)str22u, str2u.Length() ) == 0 );
	} else {
		ASSERT_EQUAL("'read file tmp/buchstU.tst'", "'could not read tmp/buchstU.tst'");
	}
}
//==============================================================================================
//        O P . O U T P U T             Ende
//==============================================================================================

void SystemFileTest::TimeTest() {
	time_t now = ::time(0);

	struct tm agmtime;
	agmtime.tm_year = 0;
	system::GmTime(&now, &agmtime);
	ASSERT(agmtime.tm_year > 0);

	struct tm alocaltime;
	alocaltime.tm_year = 0;
	system::LocalTime(&now, &alocaltime);
	ASSERT(alocaltime.tm_year > 0);

	ASSERT_EQUAL(agmtime.tm_sec, alocaltime.tm_sec);
	ASSERT_EQUAL(agmtime.tm_min, alocaltime.tm_min);
}

//==============================================================================================
//        O P . I N O U T P U T S             Beginn
//==============================================================================================
void SystemFileTest::IOStreamTest() {
	// Interaktion von >> und <<  :  Mit dem Operator "<<" wird der Inhalt gespeichert und mit ">>" wird der Inhalt gelesen.
	// Die Capacity nicht!!
	//----------------------------------------------------------------------------------------------------------------------

	String str0;
	std::istream *is0 = system::OpenStream("tmp/zahlen", "tst");

	if (is0) {
		*is0 >> str0;
		ASSERT (str0.Capacity() >= str0.Length() );
		ASSERT ( (long)strlen( (const char *)str0 ) == str0.Length() );
		ASSERT ( (long)strlen("0123456789") == str0.Length() );
		ASSERT( memcmp( (const char *)str0, "0123456789", strlen((const char *)str0) ) == 0 );
		delete is0;
	} else {
		ASSERT_EQUAL("'read file tmp/zahlen.tst'", "'could not read tmp/zahlen.tst'");
	}

	String str1;
	std::istream *is1 = system::OpenStream("tmp/buchst", "tst");

	if (is1) {
		*is1 >> str1;
		ASSERT (str1.Capacity() >= str1.Length() );
		ASSERT (str1.Length() == (long)strlen( (const char *)str1 ));
		ASSERT (str1.Length() == (long)strlen("qwertzuiopasdfghjklyxcvbnm") );
		ASSERT( memcmp( (const char *)str1, "qwertzuiopasdfghjklyxcvbnm", strlen((const char *)str1) ) == 0 );
		delete is1;
	} else {
		ASSERT_EQUAL("'read file tmp/buchst.tst'", "'could not read tmp/buchst.tst'");
	}

	// Konkatenation von 2 "<<"
	// Das zweite "<<" loescht das Resultat des ersten "<<" ( ???? ist das gewuenscht ???? )
	//-------------------------------------------------------------------------------------------
	String str2 = "qwertzuiopasdfghjklyxcvbnm";
	std::ostream *os2 = system::OpenOStream("tmp/dummy", "tst", std::ios::trunc);

	if (os2) {
		*os2 << str2;
		ASSERT (str2.Capacity() >= str2.Length() );
		ASSERT ( (long)strlen( (const char *)str2 ) == str2.Length() );
		ASSERT ( (long)strlen("qwertzuiopasdfghjklyxcvbnm") == str2.Length() );
		ASSERT( memcmp( (const char *)str2, "qwertzuiopasdfghjklyxcvbnm", strlen((const char *)str2) ) == 0 );
		delete os2;
	} else {
		ASSERT_EQUAL("'write to file dummy.tst'", "'could not write dummy.tst'");
	}
	String str3 = "0123456789";
	std::ostream *os3 = system::OpenOStream("tmp/dummy", "tst", std::ios::app);

	if (os3) {
		*os3 << str3;
		ASSERT (str3.Capacity() >= str3.Length() );
		ASSERT (str3.Length() == (long)strlen("0123456789") );
		ASSERT ( (long)strlen( (const char *)str3 ) == str3.Length());
		ASSERT( memcmp( (const char *)str3, "0123456789", strlen((const char *)str3) ) == 0 );
		delete os3;
	} else {
		ASSERT_EQUAL("'write to file dummy.tst'", "'could not write dummy.tst'");
	}
	std::istream *isHlp = system::OpenStream("tmp/dummy", "tst");

	if (isHlp) {
		String strHlp;
		String compare;
		compare << str2 << str3;
		*isHlp >> strHlp;
		delete isHlp;
		ASSERT_EQUAL(compare, strHlp);
		ASSERT_EQUAL(compare.Length(), strHlp.Length() );
	} else {
		ASSERT_EQUAL("'read file dummy.tst'", "'could not read dummy.tst'");
	}

	// Mehrfaches "<<"-Operator
	// Bei jedem Aufruf wird die Datei tmp/strChain.tst geloescht und neu geschrieben
	String str4 = "StringA", str5 = "StringB";
	std::ostream *os4 = system::OpenOStream("tmp/strChain", "tst");

	if (os4) {
		// You must check if the file is OK   ( ???? )
		*os4 << str4 << std::endl << str5 << 'a' << 'b' << 123456789L << ' ' << -123456789L << ' ' << true << ' ' << false << " qqq" << " "
				<< 123 << " " << -123.234 << " " << 0.2345e-7 << ' ' << 1.123456789e9;
		delete os4;
	} else {
		ASSERT_EQUAL("'write to file tmp/strChain.tst'", "'could not write tmp/strChain.tst'");
	}
	// Mehrfaches ">>"-Operator
	// Bei jedem Aufruf wird die Datei tmp/strChain.tst geloescht und neu geschrieben
	String str6;
	std::istream *is2 = system::OpenStream("tmp/buchst", "tst");
	std::istream *is3 = system::OpenStream("tmp/zahlen", "tst");

	if (is2 && is3) {
		// Resultat ist wie erwartet oder falsch  ????
		// str6 << *is2;  Compilierbar aber falsches Resultat
		*is2 >> str6;
		ASSERT_EQUAL("qwertzuiopasdfghjklyxcvbnm", str6);
		ASSERT( str6.Length() == (long)strlen("qwertzuiopasdfghjklyxcvbnm") );
		ASSERT( str6.Capacity() >= str6.Length() );
		*is3 >> str6;
		ASSERT_EQUAL("0123456789", str6);
		ASSERT( str6.Length() == (long)strlen("0123456789") );
		ASSERT( str6.Capacity() >= str6.Length() );
		delete is2;
		delete is3;
	} else {
		if (!is2) {
			ASSERT_EQUAL("'read file tmp/buchst.tst'", "'could not read tmp/buchst.tst'");
		}
		if (!is3) {
			ASSERT_EQUAL("'read file tmp/zahlen.tst'", "'could not read tmp/zahlen.tst'");
		}
	}

	// Einfluss von (char)0:  Length() = 5 ABER strlen = 2
	String str7;
	char bufHlp[5] = { 'a', 'b', (char) 0, 'c', 'd' };
	str7.Append((void *) bufHlp, 5);
	std::ostream *os5 = system::OpenOStream("tmp/strMit0", "tst");

	if (os5) {
		*os5 << str7;
		ASSERT (str7.Capacity() >= str7.Length() );
		ASSERT (str7.Length() == 5);
		ASSERT ( strlen( (const char *)str7 ) == 2);
		ASSERT( memcmp( (const char *)str7, "ab", strlen((const char *)str7) ) == 0 );
		ASSERT( memcmp( (const char *)str7, bufHlp, str7.Length() ) == 0 );
		delete os5;
	} else {
		ASSERT_EQUAL("'write to file strMit0.tst'", "'could not write strMit0.tst'");
	}
	String str8;
	std::istream *is4 = system::OpenStream("tmp/strMit0", "tst");

	if (is4) {
		*is4 >> str8;
		ASSERT (str8.Capacity() >= str8.Length() );
		ASSERT (str8.Length() == 5);
		ASSERT ( strlen( (const char *)str8 ) == 2);
		ASSERT( memcmp( (const char *)str8, "ab", strlen((const char *)str8) ) == 0 );
		ASSERT( memcmp( (const char *)str8, bufHlp, str8.Length() ) == 0 );
		delete is4;
	} else {
		ASSERT_EQUAL("'read file strMit0.tst'", "'could not read strMit0.tst'");
	}

	// Test automatic creation of files for std::ios::app and std::ios::ate
	{
		StartTrace(SystemFileTest.IOStreamTest);
		// precondition: files should not exist already!!
		String strAppFile("tmp/ios_app.tst"), strAteFile("tmp/ios_ate.tst");
		if (system::IsRegularFile(strAppFile)) {
			system::io::unlink(strAppFile);
		}
		if (system::IsRegularFile(strAteFile)) {
			system::io::unlink(strAteFile);
		}
		String strOut("FirstEntryInAppFile"), strOutApp("-AppendedContent"), strReadIn;
		std::iostream *pStream = system::OpenStream(strAppFile, NULL, std::ios::app);
		ASSERTM("expected file to be created", pStream != NULL);
			*pStream << strOut;
			delete pStream;

		pStream = system::OpenStream(strAppFile, NULL, std::ios::app);
		ASSERTM("expected file to be opened", pStream != NULL);
			*pStream << strOutApp;
			delete pStream;

		pStream = system::OpenIStream(strAppFile, NULL);
		ASSERTM("expected file to be opened", pStream != NULL);
			*pStream >> strReadIn;
			delete pStream;
			String strExpected(strOut);
			strExpected << strOutApp;
			ASSERT_EQUAL(strExpected, strReadIn);

		pStream = system::OpenStream(strAppFile, NULL, std::ios::app);
		ASSERTM("expected file to be opened", pStream != NULL);
			// can position in file but content gets still appended at the end
			pStream->seekp(strOut.Length());
			//??			ASSERT_EQUAL(strOut.Length(), (long)pStream->tellp()); // foo: should this one work on append-only streams?
			*pStream << strOut;
			delete pStream;

		pStream = system::OpenIStream(strAppFile, NULL);
		ASSERTM("expected file to be opened", pStream != NULL);
			*pStream >> strReadIn;
			delete pStream;
			strExpected = String(strOut);
			strExpected << strOutApp;
			strExpected << strOut;
			ASSERT_EQUAL(strExpected, strReadIn);

		pStream = system::OpenStream(strAteFile, NULL, std::ios::ate);
		ASSERTM("expected file not to be opened", pStream == NULL);
			delete pStream;

		Trace("before first app");
		pStream = system::OpenStream(strAteFile, NULL, std::ios::app);
		ASSERTM("expected file to be opened", pStream != NULL);
			*pStream << strOut;
			delete pStream;

		Trace("testing appended content");
		pStream = system::OpenIStream(strAteFile, NULL);
		ASSERTM("expected file to be opened", pStream != NULL);
			*pStream >> strReadIn;
			delete pStream;
			ASSERT_EQUAL(strOut, strReadIn);

		Trace("before second ate");
		pStream = system::OpenStream(strAteFile, NULL, std::ios::ate);
		ASSERTM("expected file to be opened", pStream != NULL);
			*pStream << strOutApp;
			delete pStream;

		Trace("testing ate");
		pStream = system::OpenIStream(strAteFile, NULL);
		ASSERTM("expected file to be opened", pStream != NULL);
			*pStream >> strReadIn;
			delete pStream;
			strExpected = String(strOut);
			strExpected << strOutApp;
			ASSERT_EQUAL(strExpected.cstr(), strReadIn);

		Trace("before third ate");
		pStream = system::OpenStream(strAteFile, NULL, std::ios::ate);
		ASSERTM("expected file to be opened", pStream != NULL);
			// can position in file, contents get appended beginning from this location then
			pStream->seekp(strOut.Length());
			ASSERT_EQUAL(strOut.Length(), (long)pStream->tellp());
			*pStream << strOut;
			delete pStream;

		Trace("testing ate");
		pStream = system::OpenIStream(strAteFile, NULL);
		ASSERTM("expected file to be opened", pStream != NULL);
			*pStream >> strReadIn;
			delete pStream;
			strExpected = String(strOut);
			strExpected << strOut;
			ASSERT_EQUAL(strExpected.cstr(), strReadIn);

	}
}

//==============================================================================================
//        O P . I N O U T P U T S             Ende
//==============================================================================================

void SystemFileTest::LoadConfigFileTest() {
	Anything dbgany;
	ASSERT(system::LoadConfigFile(dbgany, "Tracer")); // any extension automatic
	ASSERT(!dbgany.IsNull());
	ASSERT(dbgany.IsDefined("LowerBound"));

	Anything dbg2;
	String realfilename;
	ASSERT(system::LoadConfigFile(dbg2, "Tracer", "any", realfilename));
	ASSERT_EQUAL("Tracer.any", realfilename.SubString(realfilename.StrRChr(system::Sep())+1L));
	ASSERT_ANY_EQUAL(dbgany, dbg2);

	Anything dbg3;
	ASSERT(!system::LoadConfigFile(dbg3, "NotExisting", "any", realfilename));
	ASSERT_EQUAL("", realfilename);
	ASSERT(dbg3.IsNull());
}

ROAnything SystemFileTest::GetTestCaseConfig(String strClassName, String strTestName) {
	Anything fConfig;
	if(!system::LoadConfigFile(fConfig, "SystemFileTest", "any")) {
		ASSERT_EQUAL("'read SystemFileTest.any'", "'could not read SystemFileTest.any'");
	}
	return fConfig[strClassName][strTestName];
}

void SystemFileTest::MkRmDirTest() {
	StartTrace(SystemFileTest.MkRmDirTest);
	String strTmpDir = GetTestCaseConfig("TmpDir").AsString("/tmp");
	String str1LevelRel(__FUNCTION__);
	String str2LevelRel(str1LevelRel);
	str2LevelRel.Append(system::Sep()).Append("Level2");
	String str1Level(strTmpDir), str2Level(strTmpDir);
	str1Level << system::Sep() << str1LevelRel;
	str2Level << system::Sep() << str2LevelRel;
	Trace("strTmpDir [" << strTmpDir << "]");
	Trace("str1LevelRel [" << str1LevelRel << "]");
	Trace("str2LevelRel [" << str2LevelRel << "]");
	Trace("str1Level [" << str1Level << "]");
	Trace("str2Level [" << str2Level << "]");
	// assume that we have a tmp-directory to access and to play with
	ASSERTM("expected an accessible directory" , system::IsDirectory(strTmpDir));
		ASSERTM("expected creation of directory to fail", system::io::mkdir(strTmpDir, 0755) == -1 );
		ASSERTM("expected creation of directory to succeed", system::io::mkdir(str1Level, 0755) == 0 );
			ASSERTM("expected an accessible directory" , system::IsDirectory(str1Level));
			ASSERTM("expected creation of existing directory to fail", system::io::mkdir(str1Level, 0755) == -1 );
			ASSERTM("expected deletion of directory to succeed", system::io::rmdir(str1Level) == 0 );
			ASSERTM("expected deletion of nonexisting directory to fail", system::io::rmdir(str1Level) == -1 );

		ASSERTM("expected creation of multiple directory levels at once to fail", system::io::mkdir(str2Level, 0755) == -1 );
		// relative tests
		String wd;
		system::GetCWD(wd);
		ASSERT( system::ChangeDir(strTmpDir) );
			ASSERTM("expected creation of relative directory to succeed" , system::io::mkdir(str1LevelRel, 0755) == 0 );
			ASSERTM("expected creation of two level relative directory to succeed" , system::io::mkdir(str2LevelRel, 0755) == 0 );
			ASSERTM("expected deletion of non-empty relative directory to fail" , system::io::rmdir(str1LevelRel) == -1 );
			ASSERTM("expected deletion of relative directory to succeed" , system::io::rmdir(str2LevelRel) == 0 );
			ASSERTM("expected deletion of relative directory to succeed" , system::io::rmdir(str1LevelRel) == 0 );

		system::ChangeDir(wd);

}

void SystemFileTest::MakeRemoveDirectoryTest() {
	StartTrace(SystemFileTest.MakeRemoveDirectoryTest);
	String strTmpDir = GetTestCaseConfig("TmpDir").AsString("/tmp");
	String str1LevelRel(__FUNCTION__);
	String str2LevelRel(str1LevelRel);
	str2LevelRel.Append(system::Sep()).Append("Level2");
	String str1Level(strTmpDir), str2Level(strTmpDir);
	str1Level << system::Sep() << str1LevelRel;
	str2Level << system::Sep() << str2LevelRel;
	Trace("strTmpDir [" << strTmpDir << "]");
	Trace("str1LevelRel [" << str1LevelRel << "]");
	Trace("str2LevelRel [" << str2LevelRel << "]");
	Trace("str1Level [" << str1Level << "]");
	Trace("str2Level [" << str2Level << "]");
	// assume that we have a tmp-directory to access and to play with
	ASSERTM("expected an accessible directory" , system::IsDirectory(strTmpDir));
		String strSaveParam(strTmpDir);
		// one level tests
		ASSERT_EQUALM("expected creation of directory to fail", system::eExists, system::MakeDirectory(strTmpDir, 0755, false));
		system::ResolvePath(strSaveParam);
		ASSERT_EQUAL(strSaveParam, strTmpDir);
		strSaveParam = str1Level;
		ASSERT_EQUALM("expected creation of directory to succeed", system::eSuccess, system::MakeDirectory(str1Level, 0755, false));
			system::ResolvePath(strSaveParam);
			ASSERT_EQUAL(strSaveParam, str1Level);
			ASSERTM("expected an accessible directory" , system::IsDirectory(str1Level));
			ASSERT_EQUALM("expected creation of existing directory to fail", system::eExists, system::MakeDirectory(str1Level, 0755, false));
			system::ResolvePath(strSaveParam);
			ASSERT_EQUAL(strSaveParam, str1Level);
			ASSERT_EQUALM("expected deletion of directory to succeed", system::eSuccess, system::RemoveDirectory(str1Level, false));
			ASSERTM("expected directory to be deleted" , !system::IsDirectory(str1Level));

		// multiple level tests
		strSaveParam = str2Level;
		ASSERT_EQUALM("expected creation of multiple directory levels at once to fail", system::eNotExists, system::MakeDirectory(str2Level, 0755, false));
		ASSERT_EQUAL((const char *)str1Level, str2Level);
		str2Level = strSaveParam;
		ASSERT_EQUALM("expected creation of multiple directory levels at once to succeed", system::eSuccess, system::MakeDirectory(str2Level, 0755, true) );
			system::ResolvePath(strSaveParam);
			ASSERT_EQUAL(strSaveParam, str2Level);
			ASSERTM("expected an accessible directory tree" , system::IsDirectory(str2Level));
			ASSERT_EQUALM("expected deletion of multiple absolute dir levels to fail" , system::eRecurseDeleteNotAllowed, system::RemoveDirectory(str2Level, true));
			String msg;
			msg << "expected deletion of parent dir [" << str1Level << "] to fail";
			ASSERT_EQUALM(msg.cstr() , system::eExists, system::RemoveDirectory(str1Level, false));
			ASSERT_EQUALM("expected deletion of one absolute dir level to succeed" , system::eSuccess, system::RemoveDirectory(str2Level, false));
			ASSERT_EQUALM("expected deletion of one absolute dir level to succeed" , system::eSuccess, system::RemoveDirectory(str1Level, false));
			ASSERTM("expected directory to be deleted" , !system::IsDirectory(str2Level));
			ASSERTM("expected directory to be deleted" , !system::IsDirectory(str1Level));

		// relative tests
		String wd;
		system::GetCWD(wd);
		ASSERT( system::ChangeDir(strTmpDir) );
			// one level tests
			Trace("str1LevelRel [" << str1LevelRel << "] str2LevelRel [" << str2LevelRel << "]");
			ASSERT_EQUALM("expected creation of relative directory to succeed" , system::eSuccess, system::MakeDirectory(str1LevelRel, 0755, false));
			ASSERT_EQUALM("expected creation of second level relative directory to succeed" , system::eSuccess, system::MakeDirectory(str2LevelRel, 0755, false));
			Trace("str1LevelRel [" << str1LevelRel << "] str2LevelRel [" << str2LevelRel << "]");
			ASSERT_EQUALM("expected deletion of parent relative directory to fail" , system::eExists, system::RemoveDirectory(str1LevelRel, false));
			ASSERT_EQUALM("expected deletion of relative directory to succeed" , system::eSuccess, system::RemoveDirectory(str2LevelRel, false));
			ASSERT_EQUALM("expected deletion of relative directory to succeed" , system::eSuccess, system::RemoveDirectory(str1LevelRel, false));
			// multiple level tests
			ASSERT_EQUALM("expected creation of multiple level relative directories to succeed" , system::eSuccess, system::MakeDirectory(str2LevelRel, 0755, true));
			ASSERT_EQUALM("expected deletion of multiple relative directories to succeed" , system::eSuccess, system::RemoveDirectory(str2LevelRel, true));

		system::ChangeDir(wd);

}

void SystemFileTest::MakeDirectoryTest() {
	StartTrace(SystemFileTest.MakeDirectoryTest);
	String strStartDir = GetTestCaseConfig(__FUNCTION__, "BasePath").AsString("/tmp");
	long lNumDirsMax = GetTestCaseConfig(__FUNCTION__, "MaxNumDirs").AsLong(20L), lIdx = 0L;
	Trace("BasePath [" << strStartDir << "], MaxDirs: " << lNumDirsMax);
	String strDirToCreate;

	// assume that we have a tmp-directory to access and to play with
	if (!system::IsDirectory(strStartDir)) {
		ASSERT_EQUALM("expected creation of directory to succeed", system::eSuccess, system::MakeDirectory(strStartDir, 0755, true));
	}
	ASSERTM("expected start directory to be valid", system::IsDirectory(strStartDir));
		for (; lIdx < lNumDirsMax; ++lIdx) {
			strDirToCreate.Trim(0L);
			strDirToCreate.Append(strStartDir).Append(system::Sep()).Append(lIdx);
			if (system::MakeDirectory(strDirToCreate, 0755, false) != system::eSuccess) {
				SYSERROR("failed at index: " << lIdx);
				break;
			}
		}
		ASSERT_EQUALM("expected given number of directories to be created", lIdx, lNumDirsMax);
		SYSINFO("last directory created [" << strDirToCreate << "] Idx: " << lIdx);
		while (--lIdx >= 0L) {
			strDirToCreate.Trim(0L);
			strDirToCreate.Append(strStartDir).Append(system::Sep()).Append(lIdx);
			system::RemoveDirectory(strDirToCreate, false);
		}

	ASSERT_EQUALM("expected deletion of directory to succeed", system::eSuccess, system::RemoveDirectory(strStartDir, false));
}

void SystemFileTest::MakeDirectoryExtendTest() {
	StartTrace(SystemFileTest.MakeDirectoryExtendTest);

	String strBaseDir(GetTestCaseConfig("BaseDir").AsString());
	const char pcFillerPrefix[] = "dummydir_";
	bool bCreatedDirs(false);
	if (strBaseDir.Length() > 0L) {
		long lIdx(1L);
		if (!system::IsDirectory(strBaseDir)) {
			system::MakeDirectory(strBaseDir, 0755, true, false);
		}
		String strFillerDir(strBaseDir);
		strFillerDir.Append(system::Sep()).Append(pcFillerPrefix);
		long lTrimLen(strFillerDir.Length());
		strFillerDir.Append(lIdx);
		while (system::MakeDirectory(strFillerDir, 0755, true, false) == system::eSuccess) {
			strFillerDir.Trim(lTrimLen);
			strFillerDir.Append(++lIdx);
			if ((lIdx % 1000) == 0) {
				SystemLog::WriteToStdout(String("directory [").Append(strFillerDir).Append("] created...\n"));
			}
			bCreatedDirs = true;
		}
		--lIdx;
		// add two directories to the number of created dirs: '.' and '..'
		long iNumLinks = lIdx + 2;
		Trace("number of dirs created: " << lIdx << ", num of hardlinks: " << iNumLinks);

		AnyExtensions::Iterator<ROAnything> aIterator(GetTestCaseConfig("Tests"));
		ROAnything roaConfig;
		while (lIdx > 0L && aIterator(roaConfig)) {
			String strEnsureDir(roaConfig["EnsureDirExists"].AsString());
			String strExpectBaseDir(roaConfig["ExpectedBaseDir"].AsString());
			String strCreateDirRel(roaConfig["PathToCreate"].AsString());
			String strLinkName(strCreateDirRel);
			String strCreateDir(strBaseDir), strExpectDir(strExpectBaseDir);
			strCreateDir.Append(system::Sep()).Append(strCreateDirRel);
			strExpectDir.Append(system::Sep()).Append(strCreateDirRel);
			long lSep = strLinkName.StrChr(system::Sep());
			if (lSep > 0L) {
				strLinkName.Trim(lSep);
			}
			Trace("Dir to create [" << strCreateDir << "] ExpectedDir [" << strExpectDir << "] first seg [" << strLinkName << "]");
			Trace("first seg [" << strLinkName << "] of rel path [" << strCreateDirRel << "]");
			if (strCreateDirRel.Length() > 0L && strCreateDir.Length() > 0L && strExpectDir.Length() > 0L) {
				bool bDidCreateDir(false);
				if (strEnsureDir.Length()) {
					system::DirStatusCode aCode(system::MakeDirectory(strEnsureDir, 0755, true, false));
					bDidCreateDir = (aCode == system::eSuccess);
				}
				if (!system::IsDirectory(strCreateDir)) {
					String strTmpDir(strCreateDir);
					// test should fail without extend link option
					std::string msg("expected creation of directory to fail due to no more available hardlinks, is the test-directory [");
					msg += strTmpDir;
					msg += "] full of subdirs?";
					ASSERT_EQUALM(msg, system::eNoMoreHardlinks, system::MakeDirectory(strTmpDir, 0755, true, false));
						strTmpDir.Trim(strTmpDir.StrRChr(system::Sep()));
						Trace("exhausted directory [" << strTmpDir << "]");
						ASSERT_EQUAL( iNumLinks, (long)system::GetNumberOfHardLinks(strTmpDir));
						strTmpDir = strCreateDir;
						ASSERT_EQUALM("expected creationof extended directory to succeed" , system::eSuccess, system::MakeDirectory(strTmpDir, 0755, true, true));
							String wd;
							system::GetCWD(wd);
							ASSERTM("expected extension directory to be created", system::IsDirectory(strExpectDir));
							if (strLinkName.Length())
									ASSERT( system::ChangeDir(strBaseDir) );
								ASSERTM("expected directory (link) to be created", system::IsSymbolicLink(strLinkName));
								ASSERT_EQUALM("expected removal of symbolic link to succeed" , system::eSuccess, system::RemoveDirectory(strLinkName) );

							ASSERT( system::ChangeDir(strExpectBaseDir) );
								ASSERT_EQUALM("expected removal of directory to succeed", system::eSuccess, system::RemoveDirectory(strCreateDirRel, true));

							system::ChangeDir(wd);

					}
				if (bDidCreateDir && system::IsDirectory(strEnsureDir)) {
					ASSERT_EQUALM("failed to remove directory we created for testing", system::eSuccess, system::RemoveDirectory(strEnsureDir));
				}
			}
		}
		if (bCreatedDirs) {
			Trace("deleting created directories");
			strFillerDir.Trim(lTrimLen);
			strFillerDir.Append(lIdx);
			while (system::IsDirectory(strFillerDir) && system::RemoveDirectory(strFillerDir)) {
				strFillerDir.Trim(lTrimLen);
				strFillerDir.Append(--lIdx);
				if ((lIdx % 1000) == 0) {
					SystemLog::WriteToStdout(String("directory [").Append(strFillerDir).Append("] deleted...\n"));
				}
			}
		}
	}
}

#if !defined(WIN32)
void SystemFileTest::SymbolicLinkTest() {
	StartTrace(SystemFileTest.SymbolicLinkTest);

	AnyExtensions::Iterator<ROAnything> aIterator(GetTestCaseConfig());
	ROAnything roaConfig;
	while (aIterator(roaConfig)) {
		String strTmpDir = roaConfig["TmpDir"].AsString("/tmp");
		String strRelDir = roaConfig["PathToCreate"].AsString();
		String strCreateDir(strTmpDir);
		strCreateDir.Append(system::Sep()).Append(strRelDir);
		String strLinkRel = roaConfig["Link"].AsString(), strLinkAbs(strTmpDir);
		strLinkAbs.Append(system::Sep()).Append(roaConfig["Link"].AsString());
		Trace("Dir to create [" << strCreateDir << "] Link [" << strLinkRel << "]");

		if (strCreateDir.Length() > 0L) {
			// assume that we have a tmp-directory to access and to play with
			if (!system::IsDirectory(strCreateDir)) {
				ASSERT_EQUALM("expected creation of relative directory to succeed", system::eSuccess, system::MakeDirectory(strCreateDir, 0755, false));
			}
			ASSERT(system::IsDirectory(strCreateDir));
				String wd;
				system::GetCWD(wd);
				ASSERT( system::ChangeDir(strTmpDir) );
					Trace("creating relative link [" << strLinkRel << "] to relative dir [" << strRelDir << "]");
					ASSERT_EQUALM("expected creation of relative symbolic link to succeed" , system::eSuccess, system::CreateSymbolicLink(strRelDir,strLinkRel));
					ASSERTM("expected link to be valid", system::IsSymbolicLink(strLinkRel));
					ASSERT_EQUALM("expected removal of relative symbolic link to succeed" , system::eSuccess, system::RemoveDirectory(strLinkRel) );
					ASSERT(system::IsDirectory(strRelDir));
					ASSERT(!system::IsDirectory(strLinkRel) );
						Trace("creating relative link [" << strLinkRel << "] to absolute dir [" << strCreateDir << "]");
						ASSERT_EQUALM("expected creation of relative symbolic link to succeed" , system::eSuccess, system::CreateSymbolicLink(strCreateDir, strLinkRel));
						ASSERTM("expected link to be valid", system::IsSymbolicLink(strLinkRel));
						ASSERT_EQUALM("expected removal of relative symbolic link to succeed" , system::eSuccess, system::RemoveDirectory(strLinkRel) );
						ASSERT(system::IsDirectory(strCreateDir));


				system::ChangeDir(wd);
				Trace("creating absolute link [" << strLinkAbs << "] to dir [" << strCreateDir << "]");
				ASSERT_EQUALM("expected creation of absolute symbolic link to succeed" , system::eSuccess, system::CreateSymbolicLink(strCreateDir, strLinkAbs));
				ASSERTM("expected link to be valid", system::IsSymbolicLink(strLinkAbs));
				ASSERT_EQUALM("expected removal of absolute symbolic link to succeed" , system::eSuccess, system::RemoveDirectory(strLinkAbs) );
				ASSERT(system::IsDirectory(strCreateDir));

			ASSERT_EQUALM("expected removal of directory to succeed" , system::eSuccess, system::RemoveDirectory(strCreateDir) );
		}
	}
}
#endif

void SystemFileTest::GetFileSizeTest() {
	StartTrace(SystemFileTest.GetFileSizeTest);

	AnyExtensions::Iterator<ROAnything> aIterator(GetTestCaseConfig());
	ROAnything roaConfig;
	while (aIterator(roaConfig)) {
		String path(system::GetFilePath(roaConfig["File"].AsString()));
		ASSERTM("expected file path to be valid", path.Length() > 0);
			ul_long ulSize = 0;
			ASSERT(system::GetFileSize(path, ulSize));
				ASSERT_EQUALM("expected same size", roaConfig["ExpectedSize"].AsLong(), ulSize);


	}
}

void SystemFileTest::BlocksLeftOnFSTest() {
	StartTrace(SystemFileTest.BlocksLeftOnFSTest);
	// can only test that we have still some space left, nothing more for now
	ul_long ulBlocks = 0;
	unsigned long ulBlockSize = 0;
	String fsPath(GetTestCaseConfig("FS", __FUNCTION__).AsString("/"));
	ASSERTM("expected function call to succeed", system::BlocksLeftOnFS(fsPath, ulBlocks, ulBlockSize));
		ASSERTM("expected some blocks left on device", ulBlocks > 0);
		ASSERTM("expected block size not to be 0", ulBlockSize > 0);
		Trace("blocks: " << (l_long)ulBlocks << " blocksize: " << (long)ulBlockSize);

}

void SystemFileTest::runAllTests(cute::suite &s) {
	s.push_back(CUTE_SMEMFUN(SystemFileTest, initPathTest));
	s.push_back(CUTE_SMEMFUN(SystemFileTest, pathListTest));
	s.push_back(CUTE_SMEMFUN(SystemFileTest, rooDirTest));
	s.push_back(CUTE_SMEMFUN(SystemFileTest, IsAbsolutePathTest));
	s.push_back(CUTE_SMEMFUN(SystemFileTest, ResolvePathTest));
	s.push_back(CUTE_SMEMFUN(SystemFileTest, OpenStreamTest));
	s.push_back(CUTE_SMEMFUN(SystemFileTest, OpenStreamWithSearchTest));
	s.push_back(CUTE_SMEMFUN(SystemFileTest, OpenOStreamTest));
	s.push_back(CUTE_SMEMFUN(SystemFileTest, OpenIStreamTest));
	s.push_back(CUTE_SMEMFUN(SystemFileTest, GetFilePathTest));
	s.push_back(CUTE_SMEMFUN(SystemFileTest, dirFileListTest));
	s.push_back(CUTE_SMEMFUN(SystemFileTest, IStreamTest));
	s.push_back(CUTE_SMEMFUN(SystemFileTest, OStreamTest));
	s.push_back(CUTE_SMEMFUN(SystemFileTest, IOStreamTest));
	s.push_back(CUTE_SMEMFUN(SystemFileTest, CWDTests));
	s.push_back(CUTE_SMEMFUN(SystemFileTest, LoadConfigFileTest));
	s.push_back(CUTE_SMEMFUN(SystemFileTest, TimeTest));
	s.push_back(CUTE_SMEMFUN(SystemFileTest, MkRmDirTest));
	s.push_back(CUTE_SMEMFUN(SystemFileTest, MakeRemoveDirectoryTest));
	s.push_back(CUTE_SMEMFUN(SystemFileTest, MakeDirectoryTest));
#if !defined(WIN32)
	s.push_back(CUTE_SMEMFUN(SystemFileTest, SymbolicLinkTest));
#endif
	s.push_back(CUTE_SMEMFUN(SystemFileTest, MakeDirectoryExtendTest));
	s.push_back(CUTE_SMEMFUN(SystemFileTest, GetFileSizeTest));
	s.push_back(CUTE_SMEMFUN(SystemFileTest, BlocksLeftOnFSTest));
	s.push_back(CUTE_SMEMFUN(SystemFileTest, statTests)); // needs to be last
}
