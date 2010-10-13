/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include -----------------------------------------------------
#include "SystemFileTest.h"

//--- module under test --------------------------------------------------------
#include "SystemFile.h"

using namespace Coast;

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used -------------------------------------------------
#include "DiffTimer.h"
#include "SystemLog.h"
#include "AnyIterators.h"

//--- c-library modules used ---------------------------------------------------
#include <fcntl.h>

#include "boost/bind.hpp"

//---- SystemFileTest --------------------------------------------------------
SystemFileTest::SystemFileTest(TString tname)
	: TestCaseType(tname)
{
}

SystemFileTest::~SystemFileTest()
{
}

void SystemFileTest::initPathTest()
{
	String pathList(System::GetPathList());
	String rootDir(System::GetRootDir());

	t_assert(pathList.Length() > 0);
	t_assert(rootDir.Length() > 0);

	const char tmpRoot[] = "/usr/local";
	const char tmpPath[] = "bin";
	System::InitPath(tmpRoot, tmpPath);

	assertEqual("/usr/local", System::GetRootDir());
	assertEqual("bin", System::GetPathList());

#if defined(__linux)
	unsetenv("COAST_ROOT");
	unsetenv("COAST_PATH");
#else
	putenv("COAST_ROOT=");
	putenv("COAST_PATH=");
#endif
	// should have no effect now, path and root should remain
	System::InitPath();

	assertEqual("/usr/local", System::GetRootDir());
	assertEqual("bin", System::GetPathList());

	// test InitPath with root argument, path will be restored to default ".:config:src:"
	System::InitPath("/usr/local/bin");
	assertEqual("/usr/local/bin", System::GetRootDir());
	// path will be restored to ".:config:src:"
	assertEqual(".:config:src:", System::GetPathList());

	// test InitPath with path argument, root will be restored to default "."
	System::InitPath(0, "config");

	assertEqual(".", System::GetRootDir());
	assertEqual("config", System::GetPathList());

#if defined(WIN32)
	// test InitPath with NT specific arguments
	System::InitPath("d:\\gugus/blabla", "bin");

	assertEqual("d:/gugus/blabla", System::GetRootDir());
	assertEqual("bin", System::GetPathList());
#endif

	// restore old settings
	System::SetPathList(pathList, false);
	System::SetRootDir(rootDir, false);
	assertEqual(rootDir, System::GetRootDir());
	assertEqual(pathList, System::GetPathList());
}

void SystemFileTest::statTests()
{
	t_assertm(System::IsDirectory("."), "expected '.' to be a directory");
	t_assertm(System::IsDirectory(".."), "expected '.' to be a directory");
	t_assertm(!System::IsDirectory("config/Test.any"), "expected 'Test.any' to be a file");
	t_assertm(!System::IsDirectory("config/Dbg.any"), "expected 'Dbg.any' to be a file");
	t_assertm(!System::IsRegularFile("."), "expected '.' to be a directory");
	t_assertm(System::IsRegularFile("config/SystemFileTest.any"), "expected 'SystemTest.any' to be a file");
	t_assertm(!System::IsRegularFile(".."), "expected '.' to be a directory");
	t_assertm(System::IsRegularFile("config/Test.any"), "expected 'Test.any' to be a file");
	t_assertm(System::IsRegularFile("config/Dbg.any"), "expected 'Dbg.any' to be a file");
	String strLinkToPrjRunTest("aLinkToTestAny");
	if ( assertComparem( System::eSuccess, equal_to, System::CreateSymbolicLink("config/Test.any", strLinkToPrjRunTest) , "expected creation of symbolic link to file to succeed" ) ) {
		t_assertm(System::IsSymbolicLink(strLinkToPrjRunTest), "expected link to be valid");
		t_assertm(!System::IsDirectory(strLinkToPrjRunTest), "expected link not to be a directory");
		t_assertm(System::IsRegularFile(strLinkToPrjRunTest), "expected link to point to a regular file");
		assertComparem( System::eSuccess, equal_to, System::RemoveDirectory(strLinkToPrjRunTest) , "expected removal of symbolic link to succeed" );
	}
	String strLinkToDirectory("aLinkToDirectory");
	if ( assertComparem( System::eSuccess, equal_to, System::CreateSymbolicLink("config", strLinkToDirectory) , "expected creation of symbolic link to file to succeed" ) ) {
		t_assertm(System::IsSymbolicLink(strLinkToDirectory), "expected link to be valid");
		t_assertm(System::IsDirectory(strLinkToDirectory), "expected link to point to a directory");
		t_assertm(!System::IsRegularFile(strLinkToDirectory), "expected link not to be a regular file");
		assertComparem( System::eSuccess, equal_to, System::RemoveDirectory(strLinkToDirectory) , "expected removal of symbolic link to succeed" );
	}
}

void SystemFileTest::pathListTest()
{
	String pathList(System::GetPathList());

	t_assert(pathList.Length() > 0);
	String newPath("/:/usr:/usr/local");

	System::SetPathList(newPath, false);

	assertEqual(newPath, System::GetPathList());

	// restore old pathList
	System::SetPathList(pathList, false);
}

void SystemFileTest::rooDirTest()
{
	String rootDir(System::GetRootDir());

	t_assert(rootDir.Length() > 0);

	String newRoot("/");

	System::SetRootDir(newRoot, false);

	assertEqual(newRoot, System::GetRootDir());

	// restore old root dir
	System::SetRootDir(rootDir, false);
}

void SystemFileTest::CWDTests()
{
	String wd;
	System::GetCWD(wd);
	t_assertm(System::IsAbsolutePath(wd), "working dir should be absolute");
	t_assertm(wd.Length() > 0, "working dir should not be empty");
	t_assertm(wd != ".", "working dir should not be default .");

	String upper = wd.SubString(0, wd.StrRChr(System::Sep()));
	t_assertm(System::ChangeDir(".."), "shouldn't fail, .. dir always accessible");
	t_assertm(upper.Length() > 0, "should have upper directory");
	assertEqual(0L, wd.Contains(upper));
	t_assertm(upper.Length() < wd.Length(), "should really be upper if .. isn't mount point");

	String u;
	System::GetCWD(u);
	assertEqual(upper, u);
	t_assertm(u.Length() > 0, "huh, directory empty");
	t_assertm(u != ".", "working dir should not be default .");

	String strCwd;
	System::GetCWD(strCwd);
	String root;
	root << System::Sep();
	t_assertm(System::ChangeDir(root), "shouldn't fail, root dir always accessible");
	String r1;
	System::GetCWD(r1);
#if defined(WIN32)
	// on WIN32, if we change to 'root' it is actually the root Dir of the current drive
	// in the form <drive>:<system::SEP>
	char driveLetter;
	t_assertm(System::GetDriveLetter(strCwd, driveLetter), TString("couldn't get drive letter from [") << strCwd << "]");
	root = "";
	root << driveLetter << ":" << System::Sep();
	assertEqual(root, r1);
#else
	assertEqual(root, r1);
#endif
	t_assertm(r1.Length() > 0, "huh, directory empty");
	t_assertm(r1 != ".", "working dir should not be default .");

	String invaliddir;
	invaliddir << System::Sep() << "wuggi"
			   << System::Sep() << "waggi"
			   << System::Sep() << "gugus"
			   << System::Sep() << "this"
			   << System::Sep() << "directory"
			   << System::Sep() << "doesnt"
			   << System::Sep() << "exist";
	t_assertm(!System::ChangeDir(invaliddir), "should fail, dir never exists");
	String r2;
	System::GetCWD(r2);
	assertEqual(root, r2);
	t_assertm(r2.Length() > 0, "huh, directory empty");
	t_assertm(r2 != ".", "working dir should not be default .");

//-- clean up
	t_assertm(System::ChangeDir(wd), "shouldn't fail, .. original dir always accessible");
	String w1;
	System::GetCWD(w1);
	assertEqual(wd, w1);
	t_assertm(w1.Length() > 0, "huh, directory empty");
	t_assertm(w1 != ".", "working dir should not be default .");
}

void SystemFileTest::IsAbsolutePathTest()
{
	t_assertm(System::IsAbsolutePath("/"), "should be absolute");
	t_assertm(System::IsAbsolutePath("//"), "should be absolute");
	t_assertm(System::IsAbsolutePath("/blabla/"), "should be absolute");
	t_assertm(!System::IsAbsolutePath("./"), "should not be absolute");
	t_assertm(!System::IsAbsolutePath("../"), "should not be absolute");
	t_assertm(!System::IsAbsolutePath("config"), "should not be absolute");
#if defined(WIN32)
	t_assertm(System::IsAbsolutePath("\\"), "should be absolute");
	t_assertm(System::IsAbsolutePath("\\gaga"), "should be absolute");
	// 'normal' WIN32 drive notation, is NOT absolute, uses current dir on drive
	t_assertm(!System::IsAbsolutePath("d:"), "should not be absolute");
	// 'normal' WIN32 drive notation with absolute path
	t_assertm(System::IsAbsolutePath("d:\\"), "should be absolute");
	// 'normal' WIN32 drive notation with absolute path
	t_assertm(System::IsAbsolutePath("d:/"), "should be absolute");
	// some shells use the following absolute notation
	t_assertm(System::IsAbsolutePath("//d/"), "should be absolute");
	t_assertm(System::IsAbsolutePath("/d/"), "should be absolute");
#endif
}

void SystemFileTest::ResolvePathTest()
{
	String res, exp;
	res = "";
	System::ResolvePath(res);
	exp = "";
	assertEqual(exp, res);
	res = "/";
	System::ResolvePath(res);
	exp = "/";
	assertEqual(exp, res);
	res = ".";
	System::ResolvePath(res);
	exp = "";
	assertEqual(exp, res);
	res = "./";
	System::ResolvePath(res);
	exp = "";
	assertEqual(exp, res);
	res = "..";
	System::ResolvePath(res);
	exp = "";
	assertEqual(exp, res);
	res = "../";
	System::ResolvePath(res);
	exp = "";
	assertEqual(exp, res);
	res = "/../";
	System::ResolvePath(res);
	exp = "/";
	assertEqual(exp, res);
	res = "/../.";
	System::ResolvePath(res);
	exp = "/";
	assertEqual(exp, res);
	res = "gugus";
	System::ResolvePath(res);
	exp = "gugus";
	assertEqual(exp, res);
	res = "./gugus";
	System::ResolvePath(res);
	exp = "gugus";
	assertEqual(exp, res);
	res = "././gugus";
	System::ResolvePath(res);
	exp = "gugus";
	assertEqual(exp, res);
	res = "././././././gugus";
	System::ResolvePath(res);
	exp = "gugus";
	assertEqual(exp, res);
	res = "././././././";
	System::ResolvePath(res);
	exp = "";
	assertEqual(exp, res);
	res = "../../../../abc";
	System::ResolvePath(res);
	exp = "abc";
	assertEqual(exp, res);
	res = "./gugus/..";
	System::ResolvePath(res);
	exp = "";
	assertEqual(exp, res);
	res = "./gugus/../";
	System::ResolvePath(res);
	exp = "";
	assertEqual(exp, res);
	res = "./gugus/../..";
	System::ResolvePath(res);
	exp = "";
	assertEqual(exp, res);
	res = "./gugus/../../blabla";
	System::ResolvePath(res);
	exp = "blabla";
	assertEqual(exp, res);
	res = "./gugus/../../blabla/..";
	System::ResolvePath(res);
	exp = "";
	assertEqual(exp, res);
	res = "./gugus/../../blabla/../";
	System::ResolvePath(res);
	exp = "";
	assertEqual(exp, res);
	res = "./gugus/../../..";
	System::ResolvePath(res);
	exp = "";
	assertEqual(exp, res);
	res = "/bla/../../huhu/./ga/";
	System::ResolvePath(res);
	exp = "/huhu/ga/";
	assertEqual(exp, res);
	res = "./gugus/hallo.txt/";
	System::ResolvePath(res);
	exp = "gugus/hallo.txt/";
	assertEqual(exp, res);
	res = "./gugus./hallo.txt./";
	System::ResolvePath(res);
	exp = "gugus./hallo.txt./";
	assertEqual(exp, res);
	res = "/gugus/../..";
	System::ResolvePath(res);
	exp = "/";
	assertEqual(exp, res);
	res = " .";
	System::ResolvePath(res);
	exp = " .";
	assertEqual(exp, res);
	res = "/gugus/../..";
	res.PutAt(4, '\0');
	System::ResolvePath(res);
	exp = "/gug";
	assertEqual(exp, res);
	res = "/gugus/../..";
	res.PutAt(0, '\0');
	System::ResolvePath(res);
	exp = "";
	assertEqual(exp, res);
	res = "/gugus/../..";
	res.PutAt( res.Length() - 1L, '\0');
	System::ResolvePath(res);
	exp = "/";
	assertEqual(exp, res);
	res = "........";
	System::ResolvePath(res);
	exp = "........";
	assertEqual(exp, res);
	res = "./////";
	System::ResolvePath(res);
	exp = "";
	assertEqual(exp, res);
	res = "/foo/bla/stop";
	System::ResolvePath(res);
	exp = "/foo/bla/stop";
	assertEqual(exp, res);
#if defined(WIN32)
	res = "\\";
	System::ResolvePath(res);
	exp = "/";
	assertEqual(exp, res);
	res = ".\\";
	System::ResolvePath(res);
	exp = "";
	assertEqual(exp, res);
	res = "d:";
	System::ResolvePath(res);
	exp = "d:";
	assertEqual(exp, res);
	res = " :";
	System::ResolvePath(res);
	exp = " :";
	assertEqual(exp, res);
	res = "d:/";
	System::ResolvePath(res);
	exp = "d:/";
	assertEqual(exp, res);
	res = "d:\\";
	System::ResolvePath(res);
	exp = "d:/";
	assertEqual(exp, res);
	res = "//d/";
	System::ResolvePath(res);
	exp = "d:/";
	assertEqual(exp, res);
	res = "d:/gugus";
	System::ResolvePath(res);
	exp = "d:/gugus";
	assertEqual(exp, res);
	res = "d:./";
	System::ResolvePath(res);
	exp = "d:";
	assertEqual(exp, res);
	res = "d:./gugus";
	System::ResolvePath(res);
	exp = "d:gugus";
	assertEqual(exp, res);
	res = "d:./gugus/..";
	System::ResolvePath(res);
	exp = "d:";
	assertEqual(exp, res);
	res = "d:./gugus/../..";
	System::ResolvePath(res);
	exp = "d:";
	assertEqual(exp, res);
	res = "d:./gugus/bla/hu";
	System::ResolvePath(res);
	exp = "d:gugus/bla/hu";
	assertEqual(exp, res);
	res = "d:./gugus\\bla/hu\\";
	System::ResolvePath(res);
	exp = "d:gugus/bla/hu/";
	assertEqual(exp, res);
	res = "d:./\\/\\";
	System::ResolvePath(res);
	exp = "d:";
	assertEqual(exp, res);
	res = "d:./\\/\\/";
	System::ResolvePath(res);
	exp = "d:";
	assertEqual(exp, res);
	res = "d:..";
	System::ResolvePath(res);
	exp = "d:";
	assertEqual(exp, res);
	res = "d:/..";
	System::ResolvePath(res);
	exp = "d:/";
	assertEqual(exp, res);
	res = "d:./gugus/../gugus";
	System::ResolvePath(res);
	exp = "d:gugus";
	assertEqual(exp, res);
	res = "d::";
	System::ResolvePath(res);
	exp = "d::";
	assertEqual(exp, res);
	res = "d:/:";
	System::ResolvePath(res);
	exp = "d:/:";
	assertEqual(exp, res);
	res = "d:........";
	System::ResolvePath(res);
	exp = "d:........";
	assertEqual(exp, res);
	res = "d:d:";
	System::ResolvePath(res);
	exp = "d:d:";
	assertEqual(exp, res);
	res = ":";
	System::ResolvePath(res);
	exp = ":";
	assertEqual(exp, res);
	res = ":.";
	System::ResolvePath(res);
	exp = ":.";
	assertEqual(exp, res);
#endif
}

void SystemFileTest::OpenStreamTest()
{
	std::iostream *Ios = System::OpenStream("Dbg.any");
	t_assert( Ios == NULL ); // should not be found!
	if ( Ios ) {
		delete Ios;
		Ios = 0;
	}

	// open file with relative path
	Ios = System::OpenStream("config/Dbg.any");
	t_assert( Ios != NULL );
	if ( Ios ) {
		delete Ios;
		Ios = 0;
	}

	// deprecated:

	// search file with path
	Ios = System::OpenStream("Dbg", "any");
	t_assert( Ios != NULL ); // should be found
	if ( Ios ) {
		delete Ios;
		Ios = 0;
	}

	// open file with relative path
	Ios = System::OpenStream("config/Dbg", "any");
	t_assert( Ios != NULL );
	if ( Ios ) {
		delete Ios;
		Ios = 0;
	}

	// open file with relative path for writing
	Ios = System::OpenStream("tmp/Test1", "tst", std::ios::out);
	t_assert( Ios != NULL );
	if ( Ios ) {
		(*Ios) << "test" << std::endl;
		t_assert(!!(*Ios));
		delete Ios;
		Ios = 0;
	}
}

void SystemFileTest::OpenStreamWithSearchTest()
{
	std::iostream *Ios = System::OpenStreamWithSearch("Dbg.any");
	t_assert( Ios != NULL );
	if ( Ios ) {
		delete Ios;
		Ios = 0;
	}

	// open file with relative path
	Ios = System::OpenStreamWithSearch("config/Dbg.any");
	t_assert( Ios != NULL );
	if ( Ios ) {
		delete Ios;
		Ios = 0;
	}

	// open file with relative path for writing
	Ios = System::OpenStreamWithSearch("tmp/Test1.tst", std::ios::out);
	t_assert( Ios != NULL );
	if ( Ios ) {
		(*Ios) << "test" << std::endl;
		t_assert(!!(*Ios));
		delete Ios;
		Ios = 0;
	}
}

void SystemFileTest::OpenIStreamTest()
{
	// open file with relative path
	std::iostream *Ios = System::OpenIStream("config/Dbg.any");

	t_assert( Ios != NULL );
	if ( Ios ) {
		Anything dbgTest;
		(*Ios) >> dbgTest;
		t_assert(!!(*Ios));
		t_assert(dbgTest.GetSize() > 0);
		delete Ios;
		Ios = 0;
	}

	// deprecated:

	// open file with relative path
	Ios = System::OpenIStream("config/Dbg", "any");

	t_assert( Ios != NULL );
	if ( Ios ) {
		Anything dbgTest;
		(*Ios) >> dbgTest;
		t_assert(!!(*Ios));
		t_assert(dbgTest.GetSize() > 0);
		delete Ios;
		Ios = 0;
	}
}

void SystemFileTest::OpenOStreamTest()
{
	// open file with relative path for writing
	std::iostream *Ios = System::OpenOStream("tmp/Test.tst");

	t_assert( Ios != NULL );
	if ( Ios ) {
		(*Ios) << "test" << std::endl;
		t_assert(!!(*Ios));
		delete Ios;
		Ios = 0;
	}

	// deprecated:

	// open file with relative path for writing
	Ios = System::OpenOStream("tmp/Test", "tst");

	t_assert( Ios != NULL );
	if ( Ios ) {
		(*Ios) << "test" << std::endl;
		t_assert(!!(*Ios));
		delete Ios;
		Ios = 0;
	}
}

void SystemFileTest::testGetFilePath(boost::function<String ()> func, const String& notFoundResult) {
	StartTrace(SystemFileTest.testGetFilePath);
	String subPath("./config/Dbg.any");

	System::Chmod(subPath, 0400); // set it read only

	String path(func());

	System::ResolvePath(subPath);
	assertEqual(subPath, path.SubString(path.Length() - subPath.Length()));

	System::Chmod(subPath, 0000); // set it to no access
#if defined(WIN32)
	t_assertm(false, " FIXME: NT lacks easy hiding of files or directories...");
#endif

	path = func();
	// should fail now.... therefore path is equal

#if !defined(WIN32)
	assertEqual(notFoundResult, path);
#endif
	System::Chmod(subPath, 0640); //clean up to make it usable again

	path = func();
	subPath = "./Dbg.any";
	System::ResolvePath(subPath);
	assertEqual(subPath, path.SubString(path.Length() - subPath.Length()));
}

void SystemFileTest::GetFilePathTest()
{
	testGetFilePath(boost::bind(&Coast::System::GetFilePath, "Dbg", "any"), ""); // deprecated
	testGetFilePath(boost::bind(&Coast::System::GetFilePath, "Dbg.any"), "");
	testGetFilePath(boost::bind(&Coast::System::GetFilePathOrInput, "Dbg.any"), "Dbg.any");
}

void SystemFileTest::dirFileListTest()
{
	StartTrace(SystemFileTest.dirFileListTest);
	Anything dir(System::DirFileList("."));

	assertEqual( 0L, dir.GetSize() );

	dir = System::DirFileList("config", "any");
	t_assert( dir.GetSize() > 0L );

	dir = System::DirFileList("..", "");
	t_assert( dir.GetSize() > 0L );
	TraceAny(dir, "entries of [..]");
}

//==============================================================================================
//        O P . I N P U T             Beginn
//==============================================================================================
void SystemFileTest::IStreamTest ()
{
	// istream& operator>>(istream& is, String& s)
	// String noch nicht belegt: Capacity gesetzt (groesser als die Dateilaenge) und Inhalt gesetzt.
	// String schon belegt:  Der Inhalt des Strings wird reset (Capacity NICHT) und neu gesetzt.
	//-------------------------------------------------------------------------------------------------

	String str0;
	std::istream *is0 = System::OpenStream("len5", "tst");
	if ( is0 ) {
		*is0 >> str0;
		//PS? t_assert ( str0.Capacity() == 66 );
		t_assert ( str0.Length() == 5 );
		t_assert ( str0.Capacity() >= str0.Length() );
		t_assert ( (long)strlen( (const char *)str0 ) == 5 );
		t_assert ( strcmp( (const char *)str0, "01234" ) == 0 );

		*is0 >> str0;
		//PS? t_assert ( str0.Capacity() == 66 );
		t_assert ( str0.Length() == 0 );
		t_assert ( str0.Capacity() >= str0.Length() );
		t_assert ( (long)strlen( (const char *)str0 ) == 0 );
		t_assert ( strcmp( (const char *)str0, "" ) == 0 );

#if !defined(WIN32)
// win32 istream hangs if read at file end is attempted! (FIXME?)
		*is0 >> str0;
		//PS? t_assert ( str0.Capacity() == 66 );
		t_assert ( str0.Length() == 0 );
		t_assert ( str0.Capacity() >= str0.Length() );
		t_assert ( (long)strlen( (const char *)str0 ) == 0 );
		t_assert ( strcmp( (const char *)str0, "" ) == 0 );

		*is0 >> str0;
		//PS? t_assert ( str0.Capacity() == 66 );
		t_assert ( str0.Length() == 0 );
		t_assert ( str0.Capacity() >= str0.Length() );
		t_assert ( (long)strlen( (const char *)str0 ) == 0 );
		t_assert ( strcmp( (const char *)str0, "" ) == 0 );
#endif
		delete is0;
	} else {
		assertEqual("'read file len5.tst'", "'could not read len5.tst'");
	}

	// Kombination von 2 ">>":  das zweite ">>" loescht das Resultat vom ersten ">>" und berechnet es neu.
	//---------------------------------------------------------------------------------------------------------
	String str1;
	std::istream *is1 = System::OpenStream("len5", "tst");
	if ( is1 ) {
		*is1 >> str1;
		//PS? t_assert (str1.Capacity() == 66 );
		t_assert (str1.Length() == 5);
		t_assert ( str1.Capacity() >= str1.Length() );
		t_assert ( strlen( (const char *)str1 ) == 5);
		t_assert( memcmp( (const char *)str1, "01234", (long)strlen((const char *)str1) ) == 0 );

		delete is1;
	} else {
		assertEqual("'read file len5.tst'", "'could not read len5.tst'");
	}
	std::istream *is2 = System::OpenStream("len5", "tst");
	if ( is2 ) {
		*is2 >> str1;
		//PS? t_assert (str1.Capacity() == 66 );
		t_assert (str1.Length() == 5);
		t_assert ( str1.Capacity() >= str1.Length() );
		t_assert ( strlen( (const char *)str1 ) == 5);
		t_assert( memcmp( (const char *)str1, "01234", strlen((const char *)str1) ) == 0 );
		// und nicht 0123401234.  Ist es gewuenscht ????
		delete is2;
	} else {
		assertEqual("'read file len5.tst'", "'could not read len5.tst'");
	}

	String str3 = "qwertzuiopasdfghjklyxcvbnm";
	std::istream *is3 = System::OpenStream("len5", "tst");
	if ( is3 ) {
		*is3 >> str3;
		//PS? t_assert (str3.Capacity() == 54 );
		t_assert (str3.Length() == 5);
		t_assert (str3.Capacity() >= str3.Length() );
		t_assert ( memcmp( (const char *)str3, "01234", strlen((const char *)str3) ) == 0 );
		t_assert ( strlen( (const char *)str3 ) == 5);
		t_assert( memcmp( (const char *)str3, "01234", strlen((const char *)str3) ) == 0 );
		// und nicht qwe...bnm91234.  Ist es gewuenscht ????
		delete is3;
	} else {
		assertEqual("'read file len5.tst'", "'could not read len5.tst'");
	}
}
//==============================================================================================
//        O P . I N P U T             Ende
//==============================================================================================

//==============================================================================================
//        O P . O U T P U T             Beginn
//==============================================================================================
void SystemFileTest::OStreamTest ()
{
	//	ostream& operator<<(ostream& os, const String &s)
	//-------------------------------------------------------------
	String str0 = "", str00 = "";
	String sname("tmp/emptyStr");
	std::ostream *os0 = System::OpenOStream(sname, "tst");
	TString msg("couldn't open ");
	msg << System::GetFilePath(sname, "tst");

	t_assertm(os0 != 0, msg);
	if ( os0 ) {
		*os0 << str0;
		delete os0;
	} else {
		assertEqual("'write to file tmp/emptyStr.tst'", "'could not write tmp/emptyStr.tst'");
	}

	std::istream *is0 = System::OpenStream("tmp/emptyStr", "tst");

	if ( is0 ) {
		*is0 >> str00;
		delete is0;
		t_assert( str0 == str00 );
		assertCompare( str0.Length(), equal_to, str00.Length() );
		// t_assert( str0.Capacity() == str00.Capacity() );
		t_assert( str0.Capacity() >= str0.Length() );
		t_assert( str00.Capacity() >= str00.Length() );
		t_assert( memcmp( (const char *)str0, (const char *)str00, str0.Length() ) == 0 );
	} else {
		assertEqual("'read file tmp/emptyStr.tst'", "'could not read tmp/emptyStr.tst'");
	}

	String str1, str11;
	str1.Append("0123456789");
	std::ostream *os1 = System::OpenOStream("tmp/zahlen", "tst");

	if ( os1 ) {
		*os1 << str1;
		delete os1;
	} else {
		assertEqual("'write to file tmp/zahlen.tst'", "'could not write tmp/zahlen.tst'");
	}
	std::istream *is1 = System::OpenStream("tmp/zahlen", "tst");

	if ( is1 ) {
		*is1 >> str11;
		delete is1;
		t_assert( str1 == str11 );
		assertCompare( str1.Length(), equal_to, str11.Length() );
		// t_assert( str1.Capacity() == str11.Capacity() );
		t_assert( str1.Capacity() >= str1.Length() );
		t_assert( str11.Capacity() >= str11.Length() );
		t_assert( memcmp( (const char *)str1, (const char *)str11, str1.Length() ) == 0 );
	} else {
		assertEqual("'read file tmp/zahlen.tst'", "'could not read tmp/zahlen.tst'");
	}

	String str2, str22;
	str2.Append("qwertzuiopasdfghjklyxcvbnm");
	std::ostream *os2 = System::OpenOStream("tmp/buchst", "tst");

	if ( os2 ) {
		*os2 << str2;
		delete os2;
	} else {
		assertEqual("'write to file tmp/buchst.tst'", "'could not write tmp/buchst.tst'");
	}
	std::istream *is2 = System::OpenStream("tmp/buchst", "tst");

	if ( is2 ) {
		*is2 >> str22;
		delete is2;
		t_assert( str2 == str22 );
		assertCompare( str2.Length(), equal_to, str22.Length() );
		// t_assert( str2.Capacity() == str22.Capacity() );
		t_assert( str2.Capacity() >= str2.Length() );
		t_assert( str22.Capacity() >= str22.Length() );
		t_assert( memcmp( (const char *)str2, (const char *)str22, str2.Length() ) == 0 );
	} else {
		assertEqual("'read file tmp/buchst.tst'", "'could not read tmp/buchst.tst'");
	}

	//	unsafe_ostream& operator<<(unsafe_ostream& os, const String &s)
	//-----------------------------------------------------------------
	String str0u = "", str00u;
	std::ostream *os0u = System::OpenOStream("tmp/emptyStrU", "tst");

	if ( os0u ) {
		*os0u << str0u;
		delete os0u;
	} else {
		assertEqual("'write to file tmp/emptyStrU.tst'", "'could not write tmp/emptyStrU.tst'");
	}

	std::istream *is0u = System::OpenStream("tmp/emptyStrU", "tst");

	if ( is0u ) {
		*is0u >> str00u;
		delete is0u;
		t_assert( str0u == str00u );
		assertCompare( str0u.Length(), equal_to, str00u.Length() );
		// t_assert( str0u.Capacity() == str00u.Capacity() );
		t_assert( str0u.Capacity() >= str0u.Length() );
		t_assert( str00u.Capacity() >= str00u.Length() );
		t_assert( memcmp( (const char *)str0u, (const char *)str00u, str0u.Length() ) == 0 );
	} else {
		assertEqual("'read file tmp/emptyStrU.tst'", "'could not read tmp/emptyStrU.tst'");
	}

	String str1u;
	str1u.Append("0123456789");
	std::ostream *os1u = System::OpenOStream("tmp/zahlenU", "tst");

	if ( os1u ) {
		*os1u << str1u;
		delete os1u;
	} else {
		assertEqual("'write to file tmp/zahlenU.tst'", "'could not write tmp/zahlenU.tst'");
	}
	std::istream *is1u = System::OpenStream("tmp/zahlenU", "tst");

	if ( is1u ) {
		String str11u;
		*is1u >> str11u;
		delete is1u;
		t_assert( str1u == str11u );
		assertCompare( str1u.Length(), equal_to, str11u.Length() );
		// t_assert( str1u.Capacity() == str11u.Capacity() );
		t_assert( str1u.Capacity() >= str1u.Length() );
		t_assert( str11u.Capacity() >= str11u.Length() );
		t_assert( memcmp( (const char *)str1u, (const char *)str11u, str1u.Length() ) == 0 );
	} else {
		assertEqual("'read file tmp/zahlenU.tst'", "'could not read tmp/zahlenU.tst'");
	}

	String str2u;
	str2u.Append("qwertzuiopasdfghjklyxcvbnm");
	std::ostream *os2u = System::OpenOStream("tmp/buchstU", "tst");

	if ( os2u ) {
		*os2u << str2u;
		delete os2u;
	} else {
		assertEqual("'write to file tmp/buchstU.tst'", "'could not write tmp/buchstU.tst'");
	}

	std::istream *is2u = System::OpenStream("tmp/buchstU", "tst");

	if ( is2u ) {
		String str22u;
		*is2u >> str22u;
		delete is2u;
		t_assert( str2u == str22u );
		assertCompare( str2u.Length(), equal_to, str22u.Length() );
		// t_assert( str2u.Capacity() == str22u.Capacity() );
		t_assert( str2u.Capacity() >= str2u.Length() );
		t_assert( str22u.Capacity() >= str22u.Length() );
		t_assert( memcmp( (const char *)str2u, (const char *)str22u, str2u.Length() ) == 0 );
	} else {
		assertEqual("'read file tmp/buchstU.tst'", "'could not read tmp/buchstU.tst'");
	}
}
//==============================================================================================
//        O P . O U T P U T             Ende
//==============================================================================================

void SystemFileTest::TimeTest ()
{
	time_t now = ::time(0);

	struct tm agmtime;
	agmtime.tm_year = 0;
	System::GmTime(&now, &agmtime);
	t_assert(agmtime.tm_year > 0);

	struct tm alocaltime;
	alocaltime.tm_year = 0;
	System::LocalTime(&now, &alocaltime);
	t_assert(alocaltime.tm_year > 0);

	assertEqual(agmtime.tm_sec, alocaltime.tm_sec);
	assertEqual(agmtime.tm_min, alocaltime.tm_min);
}

//==============================================================================================
//        O P . I N O U T P U T S             Beginn
//==============================================================================================
void SystemFileTest::IOStreamTest ()
{
	// Interaktion von >> und <<  :  Mit dem Operator "<<" wird der Inhalt gespeichert und mit ">>" wird der Inhalt gelesen.
	// Die Capacity nicht!!
	//----------------------------------------------------------------------------------------------------------------------

	String str0;
	std::istream *is0 = System::OpenStream("tmp/zahlen", "tst");

	if ( is0 ) {
		*is0 >> str0;
		t_assert (str0.Capacity() >= str0.Length() );
		t_assert ( (long)strlen( (const char *)str0 ) == str0.Length() );
		t_assert ( (long)strlen("0123456789") == str0.Length() );
		t_assert( memcmp( (const char *)str0, "0123456789", strlen((const char *)str0) ) == 0 );
		delete is0;
	} else {
		assertEqual("'read file tmp/zahlen.tst'", "'could not read tmp/zahlen.tst'");
	}

	String str1;
	std::istream *is1 = System::OpenStream("tmp/buchst", "tst");

	if ( is1 ) {
		*is1 >> str1;
		t_assert (str1.Capacity() >= str1.Length() );
		t_assert (str1.Length() == (long)strlen( (const char *)str1 ));
		t_assert (str1.Length() == (long)strlen("qwertzuiopasdfghjklyxcvbnm") );
		t_assert( memcmp( (const char *)str1, "qwertzuiopasdfghjklyxcvbnm", strlen((const char *)str1) ) == 0 );
		delete is1;
	} else {
		assertEqual("'read file tmp/buchst.tst'", "'could not read tmp/buchst.tst'");
	}

	// Konkatenation von 2 "<<"
	// Das zweite "<<" loescht das Resultat des ersten "<<" ( ???? ist das gewuenscht ???? )
	//-------------------------------------------------------------------------------------------
	String str2 = "qwertzuiopasdfghjklyxcvbnm";
	std::ostream *os2 = System::OpenOStream("tmp/dummy", "tst", std::ios::trunc);

	if ( os2 ) {
		*os2 << str2;
		t_assert (str2.Capacity() >= str2.Length() );
		t_assert ( (long)strlen( (const char *)str2 ) == str2.Length() );
		t_assert ( (long)strlen("qwertzuiopasdfghjklyxcvbnm") == str2.Length() );
		t_assert( memcmp( (const char *)str2, "qwertzuiopasdfghjklyxcvbnm", strlen((const char *)str2) ) == 0 );
		delete os2;
	} else {
		assertEqual("'write to file dummy.tst'", "'could not write dummy.tst'");
	}
	String str3 = "0123456789";
	std::ostream *os3 = System::OpenOStream("tmp/dummy", "tst", std::ios::app);

	if ( os3 ) {
		*os3 << str3;
		t_assert (str3.Capacity() >= str3.Length() );
		t_assert (str3.Length() == (long)strlen("0123456789") );
		t_assert ( (long)strlen( (const char *)str3 ) == str3.Length());
		t_assert( memcmp( (const char *)str3, "0123456789", strlen((const char *)str3) ) == 0 );
		delete os3;
	} else {
		assertEqual("'write to file dummy.tst'", "'could not write dummy.tst'");
	}
	std::istream *isHlp = System::OpenStream("tmp/dummy", "tst");

	if ( isHlp ) {
		String strHlp;
		String compare;
		compare << str2 << str3;
		*isHlp >> strHlp;
		delete isHlp;
		assertEqual(compare, strHlp);
		assertEqual(compare.Length(), strHlp.Length()  );
	} else {
		assertEqual("'read file dummy.tst'", "'could not read dummy.tst'");
	}

	// Mehrfaches "<<"-Operator
	// Bei jedem Aufruf wird die Datei tmp/strChain.tst geloescht und neu geschrieben
	String str4 = "StringA", str5 = "StringB";
	std::ostream *os4 = System::OpenOStream("tmp/strChain", "tst");

	if ( os4 ) {
		// You must check if the file is OK   ( ???? )
		*os4 << str4 << std::endl <<  str5
			 << 'a' << 'b'
			 << 123456789L << ' ' << -123456789L << ' '
			 << true << ' ' << false
			 << " qqq" << " " << 123 << " " << -123.234 << " "
			 << 0.2345e-7 << ' ' << 1.123456789e9;
		delete os4;
	} else {
		assertEqual("'write to file tmp/strChain.tst'", "'could not write tmp/strChain.tst'");
	}
	// Mehrfaches ">>"-Operator
	// Bei jedem Aufruf wird die Datei tmp/strChain.tst geloescht und neu geschrieben
	String str6;
	std::istream *is2  = System::OpenStream("tmp/buchst", "tst");
	std::istream *is3 = System::OpenStream("tmp/zahlen", "tst");

	if ( is2 && is3 ) {
		// Resultat ist wie erwartet oder falsch  ????
		// str6 << *is2;  Compilierbar aber falsches Resultat
		*is2 >> str6;
		assertEqual("qwertzuiopasdfghjklyxcvbnm", str6);
		t_assert( str6.Length() == (long)strlen("qwertzuiopasdfghjklyxcvbnm") );
		t_assert( str6.Capacity() >= str6.Length() );
		*is3 >> str6;
		assertEqual("0123456789", str6);
		t_assert( str6.Length() == (long)strlen("0123456789") );
		t_assert( str6.Capacity() >= str6.Length() );
		delete is2;
		delete is3;
	}
	if (!is2) {
		assertEqual("'read file tmp/buchst.tst'", "'could not read tmp/buchst.tst'");
	}
	if (!is3) {
		assertEqual("'read file tmp/zahlen.tst'", "'could not read tmp/zahlen.tst'");
	}

	// Einfluss von (char)0:  Length() = 5 ABER strlen = 2
	String str7;
	char bufHlp[5] = {'a', 'b', (char)0, 'c', 'd'};
	str7.Append( (void *)bufHlp, 5 );
	std::ostream *os5 = System::OpenOStream("tmp/strMit0", "tst");

	if ( os5 ) {
		*os5 << str7;
		t_assert (str7.Capacity() >=  str7.Length() );
		t_assert (str7.Length() == 5);
		t_assert ( strlen( (const char *)str7 ) == 2);
		t_assert( memcmp( (const char *)str7, "ab", strlen((const char *)str7) ) == 0 );
		t_assert( memcmp( (const char *)str7, bufHlp, str7.Length() ) == 0 );
		delete os5;
	} else {
		assertEqual("'write to file strMit0.tst'", "'could not write strMit0.tst'");
	}
	String str8;
	std::istream *is4 = System::OpenStream("tmp/strMit0", "tst");

	if ( is4 ) {
		*is4 >> str8;
		t_assert (str8.Capacity() >= str8.Length() );
		t_assert (str8.Length() == 5);
		t_assert ( strlen( (const char *)str8 ) == 2);
		t_assert( memcmp( (const char *)str8, "ab", strlen((const char *)str8) ) == 0 );
		t_assert( memcmp( (const char *)str8, bufHlp, str8.Length() ) == 0 );
		delete is4;
	} else {
		assertEqual("'read file strMit0.tst'", "'could not read strMit0.tst'");
	}

	// Test automatic creation of files for std::ios::app and std::ios::ate
	{
		StartTrace(SystemFileTest.IOStreamTest);
		// precondition: files should not exist already!!
		String strAppFile("tmp/ios_app.tst"), strAteFile("tmp/ios_ate.tst");
		if ( System::IsRegularFile(strAppFile) ) {
			System::IO::unlink(strAppFile);
		}
		if ( System::IsRegularFile(strAteFile) ) {
			System::IO::unlink(strAteFile);
		}
		String strOut("FirstEntryInAppFile"), strOutApp("-AppendedContent"), strReadIn;
		std::iostream *pStream = System::OpenStream(strAppFile, NULL, std::ios::app);
		if ( t_assertm(pStream != NULL, "expected file to be created") ) {
			*pStream << strOut;
			delete pStream;
		}
		pStream = System::OpenStream(strAppFile, NULL, std::ios::app);
		if ( t_assertm(pStream != NULL, "expected file to be opened") ) {
			*pStream << strOutApp;
			delete pStream;
		}
		pStream = System::OpenIStream(strAppFile, NULL);
		if ( t_assertm(pStream != NULL, "expected file to be opened") ) {
			*pStream >> strReadIn;
			delete pStream;
			String strExpected(strOut);
			strExpected << strOutApp;
			assertCharPtrEqual(strExpected, strReadIn);
		}
		pStream = System::OpenStream(strAppFile, NULL, std::ios::app);
		if ( t_assertm(pStream != NULL, "expected file to be opened") ) {
			// can position in file but content gets still appended at the end
			pStream->seekp(strOut.Length());
//??			assertEqual(strOut.Length(), (long)pStream->tellp()); // foo: should this one work on append-only streams?
			*pStream << strOut;
			delete pStream;
		}
		pStream = System::OpenIStream(strAppFile, NULL);
		if ( t_assertm(pStream != NULL, "expected file to be opened") ) {
			*pStream >> strReadIn;
			delete pStream;
			String strExpected(strOut);
			strExpected << strOutApp;
			strExpected << strOut;
			assertCharPtrEqual(strExpected, strReadIn);
		}
		pStream = System::OpenStream(strAteFile, NULL, std::ios::ate);
		if ( !t_assertm( pStream == NULL, "expected file not to be opened") ) {
			delete pStream;
		}
		Trace("before first app");
		pStream = System::OpenStream(strAteFile, NULL, std::ios::app);
		if ( t_assertm(pStream != NULL, "expected file to be opened") ) {
			*pStream << strOut;
			delete pStream;
		}
		Trace("testing appended content");
		pStream = System::OpenIStream(strAteFile, NULL);
		if ( t_assertm(pStream != NULL, "expected file to be opened") ) {
			*pStream >> strReadIn;
			delete pStream;
			assertCharPtrEqual(strOut, strReadIn);
		}
		Trace("before second ate");
		pStream = System::OpenStream(strAteFile, NULL, std::ios::ate);
		if ( t_assertm(pStream != NULL, "expected file to be opened") ) {
			*pStream << strOutApp;
			delete pStream;
		}
		Trace("testing ate");
		pStream = System::OpenIStream(strAteFile, NULL);
		if ( t_assertm(pStream != NULL, "expected file to be opened") ) {
			*pStream >> strReadIn;
			delete pStream;
			String strExpected(strOut);
			strExpected << strOutApp;
			assertCharPtrEqual(strExpected, strReadIn);
		}
		Trace("before third ate");
		pStream = System::OpenStream(strAteFile, NULL, std::ios::ate);
		if ( t_assertm(pStream != NULL, "expected file to be opened") ) {
			// can position in file, contents get appended beginning from this location then
			pStream->seekp(strOut.Length());
			assertEqual(strOut.Length(), (long)pStream->tellp());
			*pStream << strOut;
			delete pStream;
		}
		Trace("testing ate");
		pStream = System::OpenIStream(strAteFile, NULL);
		if ( t_assertm(pStream != NULL, "expected file to be opened") ) {
			*pStream >> strReadIn;
			delete pStream;
			String strExpected(strOut);
			strExpected << strOut;
			assertCharPtrEqual(strExpected, strReadIn);
		}
	}
}

//==============================================================================================
//        O P . I N O U T P U T S             Ende
//==============================================================================================

void SystemFileTest::LoadConfigFileTest()
{
	Anything dbgany;
	t_assert(System::LoadConfigFile(dbgany, "Dbg")); // any extension automatic
	t_assert(!dbgany.IsNull());
	t_assert(dbgany.IsDefined("LowerBound"));

	Anything dbg2;
	String realfilename;
	t_assert(System::LoadConfigFile(dbg2, "Dbg", "any", realfilename));
	assertEqual("Dbg.any", realfilename.SubString(realfilename.Length() - 7, 7));
	assertAnyEqual(dbgany, dbg2);

	Anything dbg3;
	t_assert(!System::LoadConfigFile(dbg3, "NotExisting", "any", realfilename));
	assertEqual("", realfilename);
	t_assert(dbg3.IsNull());
}

void SystemFileTest::MkRmDirTest()
{
	StartTrace(SystemFileTest.MkRmDirTest);
	String strTmpDir = GetConfig()["TmpDir"].AsString("/tmp");
	String str1LevelRel(name());
	String str2LevelRel(str1LevelRel);
	str2LevelRel.Append(System::Sep()).Append("Level2");
	String str1Level(strTmpDir), str2Level(strTmpDir);
	str1Level << System::Sep() << str1LevelRel;
	str2Level << System::Sep() << str2LevelRel;
	Trace("strTmpDir [" << strTmpDir << "]");
	Trace("str1LevelRel [" << str1LevelRel << "]");
	Trace("str2LevelRel [" << str2LevelRel << "]");
	Trace("str1Level [" << str1Level << "]");
	Trace("str2Level [" << str2Level << "]");
	// assume that we have a tmp-directory to access and to play with
	if ( t_assertm( System::IsDirectory(strTmpDir), "expected an accessible directory" ) ) {
		t_assertm( System::IO::mkdir(strTmpDir, 0755) == -1 , "expected creation of directory to fail");
		if ( t_assertm( System::IO::mkdir(str1Level, 0755) == 0 , "expected creation of directory to succeed") ) {
			t_assertm( System::IsDirectory(str1Level), "expected an accessible directory" );
			t_assertm( System::IO::mkdir(str1Level, 0755) == -1 , "expected creation of existing directory to fail");
			t_assertm( System::IO::rmdir(str1Level) == 0 , "expected deletion of directory to succeed");
			t_assertm( System::IO::rmdir(str1Level) == -1 , "expected deletion of nonexisting directory to fail");
		}
		t_assertm( System::IO::mkdir(str2Level, 0755) == -1 , "expected creation of multiple directory levels at once to fail");
		// relative tests
		String wd;
		System::GetCWD(wd);
		if ( t_assert( System::ChangeDir(strTmpDir) ) ) {
			t_assertm( System::IO::mkdir(str1LevelRel, 0755) == 0 , "expected creation of relative directory to succeed" );
			t_assertm( System::IO::mkdir(str2LevelRel, 0755) == 0 , "expected creation of two level relative directory to succeed" );
			t_assertm( System::IO::rmdir(str1LevelRel) == -1 , "expected deletion of non-empty relative directory to fail" );
			t_assertm( System::IO::rmdir(str2LevelRel) == 0 , "expected deletion of relative directory to succeed" );
			t_assertm( System::IO::rmdir(str1LevelRel) == 0 , "expected deletion of relative directory to succeed" );
		}
		System::ChangeDir(wd);
	}
}

void SystemFileTest::MakeRemoveDirectoryTest()
{
	StartTrace(SystemFileTest.MakeRemoveDirectoryTest);
	String strTmpDir = GetConfig()["TmpDir"].AsString("/tmp");
	String str1LevelRel(name());
	String str2LevelRel(str1LevelRel);
	str2LevelRel.Append(System::Sep()).Append("Level2");
	String str1Level(strTmpDir), str2Level(strTmpDir);
	str1Level << System::Sep() << str1LevelRel;
	str2Level << System::Sep() << str2LevelRel;
	Trace("strTmpDir [" << strTmpDir << "]");
	Trace("str1LevelRel [" << str1LevelRel << "]");
	Trace("str2LevelRel [" << str2LevelRel << "]");
	Trace("str1Level [" << str1Level << "]");
	Trace("str2Level [" << str2Level << "]");
	// assume that we have a tmp-directory to access and to play with
	if ( t_assertm( System::IsDirectory(strTmpDir), "expected an accessible directory" ) ) {
		String strSaveParam(strTmpDir);
		// one level tests
		assertComparem( System::eExists, equal_to, System::MakeDirectory(strTmpDir, 0755, false) , "expected creation of directory to fail");
		assertCharPtrEqual(strSaveParam, strTmpDir);
		strSaveParam = str1Level;
		if ( assertComparem( System::eSuccess, equal_to, System::MakeDirectory(str1Level, 0755, false) , "expected creation of directory to succeed") ) {
			assertCharPtrEqual(strSaveParam, str1Level);
			t_assertm( System::IsDirectory(str1Level), "expected an accessible directory" );
			assertComparem( System::eExists, equal_to, System::MakeDirectory(str1Level, 0755, false) , "expected creation of existing directory to fail");
			assertCharPtrEqual(strSaveParam, str1Level);
			assertComparem( System::eSuccess, equal_to, System::RemoveDirectory(str1Level, false), "expected deletion of directory to succeed");
			t_assertm( !System::IsDirectory(str1Level), "expected directory to be deleted" );
		}
		// multiple level tests
		strSaveParam = str2Level;
		assertComparem( System::eNotExists, equal_to, System::MakeDirectory(str2Level, 0755, false) , "expected creation of multiple directory levels at once to fail");
		assertCharPtrEqual(str1Level, str2Level);
		str2Level = strSaveParam;
		if ( assertComparem( System::eSuccess, equal_to, System::MakeDirectory(str2Level, 0755, true) , "expected creation of multiple directory levels at once to succeed") ) {
			assertCharPtrEqual(strSaveParam, str2Level);
			t_assertm( System::IsDirectory(str2Level), "expected an accessible directory tree" );
			assertComparem( System::eRecurseDeleteNotAllowed, equal_to, System::RemoveDirectory(str2Level, true), "expected deletion of multiple absolute dir levels to fail" );
			assertComparem( System::eExists, equal_to, System::RemoveDirectory(str1Level, false), TString("expected deletion of parent dir [") << str1Level << "] to fail" );
			assertComparem( System::eSuccess, equal_to, System::RemoveDirectory(str2Level, false), "expected deletion of one absolute dir level to succeed" );
			assertComparem( System::eSuccess, equal_to, System::RemoveDirectory(str1Level, false), "expected deletion of one absolute dir level to succeed" );
			t_assertm( !System::IsDirectory(str2Level), "expected directory to be deleted" );
			t_assertm( !System::IsDirectory(str1Level), "expected directory to be deleted" );
		}
		// relative tests
		String wd;
		System::GetCWD(wd);
		if ( t_assert( System::ChangeDir(strTmpDir) ) ) {
			// one level tests
			Trace("str1LevelRel [" << str1LevelRel << "] str2LevelRel [" << str2LevelRel << "]");
			assertComparem( System::eSuccess, equal_to, System::MakeDirectory(str1LevelRel, 0755, false) , "expected creation of relative directory to succeed" );
			assertComparem( System::eSuccess, equal_to, System::MakeDirectory(str2LevelRel, 0755, false) , "expected creation of second level relative directory to succeed" );
			Trace("str1LevelRel [" << str1LevelRel << "] str2LevelRel [" << str2LevelRel << "]");
			assertComparem( System::eExists, equal_to, System::RemoveDirectory(str1LevelRel, false) , "expected deletion of parent relative directory to fail" );
			assertComparem( System::eSuccess, equal_to, System::RemoveDirectory(str2LevelRel, false) , "expected deletion of relative directory to succeed" );
			assertComparem( System::eSuccess, equal_to, System::RemoveDirectory(str1LevelRel, false) , "expected deletion of relative directory to succeed" );
			// multiple level tests
			assertComparem( System::eSuccess, equal_to, System::MakeDirectory(str2LevelRel, 0755, true) , "expected creation of multiple level relative directories to succeed" );
			assertComparem( System::eSuccess, equal_to, System::RemoveDirectory(str2LevelRel, true) , "expected deletion of multiple relative directories to succeed" );
		}
		System::ChangeDir(wd);
	}
}

void SystemFileTest::MakeDirectoryTest()
{
	StartTrace(SystemFileTest.MakeDirectoryTest);
	String strStartDir = GetTestCaseConfig()["BasePath"].AsString("/tmp");
	long lNumDirsMax = GetTestCaseConfig()["MaxNumDirs"].AsLong(20L), lIdx = 0L;
	Trace("BasePath [" << strStartDir << "], MaxDirs: " << lNumDirsMax);
	String strDirToCreate;

	// assume that we have a tmp-directory to access and to play with
	if ( !System::IsDirectory(strStartDir) ) {
		assertComparem( System::eSuccess, equal_to, System::MakeDirectory(strStartDir, 0755, true) , "expected creation of directory to succeed");
	}
	if ( t_assertm( System::IsDirectory(strStartDir), "expected start directory to be valid") ) {
		System::DirStatusCode aDirStatus = System::eSuccess;
		for ( ; lIdx < lNumDirsMax; ++lIdx) {
			strDirToCreate.Trim(0L);
			strDirToCreate.Append(strStartDir).Append(System::Sep()).Append(lIdx);
			if ( ( aDirStatus = System::MakeDirectory(strDirToCreate, 0755, false) ) != System::eSuccess ) {
				SYSERROR("failed at index: " << lIdx);
				break;
			}
		}
		assertComparem(lIdx, equal_to, lNumDirsMax, "expected given number of directories to be created");
		SYSINFO("last directory created [" << strDirToCreate << "] Idx: " << lIdx);
		while ( --lIdx >= 0L ) {
			strDirToCreate.Trim(0L);
			strDirToCreate.Append(strStartDir).Append(System::Sep()).Append(lIdx);
			System::RemoveDirectory(strDirToCreate, false);
		}
	}
	assertComparem( System::eSuccess, equal_to, System::RemoveDirectory(strStartDir, false), "expected deletion of directory to succeed");
}

void SystemFileTest::MakeDirectoryExtendTest()
{
	StartTrace(SystemFileTest.MakeDirectoryExtendTest);

	String strBaseDir(GetTestCaseConfig()["BaseDir"].AsString());
	const char pcFillerPrefix[] = "dummydir_";
	bool bCreatedDirs( false );
	if ( strBaseDir.Length() > 0L ) {
		long lIdx(1L);
		System::DirStatusCode aStatusCode(System::eSuccess);
		if ( !System::IsDirectory(strBaseDir) ) {
			System::MakeDirectory(strBaseDir, 0755, true, false);
		}
		String strFillerDir(strBaseDir);
		strFillerDir.Append(System::Sep()).Append(pcFillerPrefix);
		long lTrimLen(strFillerDir.Length());
		strFillerDir.Append(lIdx);
		while ( ( aStatusCode = System::MakeDirectory(strFillerDir, 0755, true, false) ) == System::eSuccess ) {
			strFillerDir.Trim(lTrimLen);
			strFillerDir.Append(++lIdx);
			if ( ( lIdx % 1000 ) == 0 ) {
				SystemLog::WriteToStdout( String("directory [").Append(strFillerDir).Append("] created...\n") );
			}
			bCreatedDirs = true;
		}
		--lIdx;
		// add two directories to the number of created dirs: '.' and '..'
		long iNumLinks = lIdx + 2;
		Trace("number of dirs created: " << lIdx << ", num of hardlinks: " << iNumLinks);

		AnyExtensions::Iterator<ROAnything> aIterator(GetTestCaseConfig()["Tests"]);
		ROAnything roaConfig;
		while ( lIdx > 0L && aIterator(roaConfig) ) {
			String strEnsureDir(roaConfig["EnsureDirExists"].AsString());
			String strExpectBaseDir(roaConfig["ExpectedBaseDir"].AsString());
			String strCreateDirRel(roaConfig["PathToCreate"].AsString());
			String strLinkName(strCreateDirRel);
			String strCreateDir(strBaseDir), strExpectDir(strExpectBaseDir);
			strCreateDir.Append(System::Sep()).Append(strCreateDirRel);
			strExpectDir.Append(System::Sep()).Append(strCreateDirRel);
			long lSep = strLinkName.StrChr(System::Sep());
			if ( lSep > 0L ) {
				strLinkName.Trim(lSep);
			}
			Trace("Dir to create [" << strCreateDir << "] ExpectedDir [" << strExpectDir << "] first seg [" << strLinkName << "]");
			Trace("first seg [" << strLinkName << "] of rel path [" << strCreateDirRel << "]");
			if ( strCreateDirRel.Length() > 0L && strCreateDir.Length() > 0L && strExpectDir.Length() > 0L ) {
				bool bDidCreateDir(false);
				if ( strEnsureDir.Length() ) {
					System::DirStatusCode aCode(System::MakeDirectory(strEnsureDir, 0755, true, false));
					bDidCreateDir = (aCode == System::eSuccess);
				}
				if ( !System::IsDirectory(strCreateDir) ) {
					String strTmpDir(strCreateDir);
					// test should fail without extend link option
					if ( assertComparem( System::eNoMoreHardlinks, equal_to, System::MakeDirectory(strTmpDir, 0755, true, false) , TString("expected creation of directory to fail due to no more available hardlinks, is the test-directory [") << strTmpDir << "] full of subdirs?" ) ) {
						strTmpDir.Trim(strTmpDir.StrRChr(System::Sep()));
						Trace("exhausted directory [" << strTmpDir << "]");
						assertCompare( iNumLinks, equal_to, (long)System::GetNumberOfHardLinks(strTmpDir));
						strTmpDir = strCreateDir;
						if ( assertComparem( System::eSuccess, equal_to, System::MakeDirectory(strTmpDir, 0755, true, true) , "expected creation of extended directory to succeed" ) ) {
							String wd;
							System::GetCWD(wd);
							t_assertm(System::IsDirectory(strExpectDir), "expected extension directory to be created");
							if ( strLinkName.Length() && t_assert( System::ChangeDir(strBaseDir) ) ) {
								t_assertm(System::IsSymbolicLink(strLinkName), "expected directory (link) to be created");
								assertComparem( System::eSuccess, equal_to, System::RemoveDirectory(strLinkName) , "expected removal of symbolic link to succeed" );
							}
							if ( t_assert( System::ChangeDir(strExpectBaseDir) ) ) {
								assertComparem( System::eSuccess, equal_to, System::RemoveDirectory(strCreateDirRel, true) , "expected removal of directory to succeed" );
							}
							System::ChangeDir(wd);
						}
					}
				}
				if ( bDidCreateDir && System::IsDirectory(strEnsureDir) ) {
					assertComparem( System::eSuccess, equal_to, System::RemoveDirectory(strEnsureDir), "failed to remove directory we created for testing");
				}
			}
		}
		if ( bCreatedDirs ) {
			Trace("deleting created directories");
			strFillerDir.Trim(lTrimLen);
			strFillerDir.Append(lIdx);
			while ( System::IsDirectory(strFillerDir) && System::RemoveDirectory(strFillerDir) ) {
				strFillerDir.Trim(lTrimLen);
				strFillerDir.Append(--lIdx);
				if ( ( lIdx % 1000 ) == 0 ) {
					SystemLog::WriteToStdout( String("directory [").Append(strFillerDir).Append("] deleted...\n") );
				}
			}
		}
	}
}

void SystemFileTest::SymbolicLinkTest()
{
	StartTrace(SystemFileTest.SymbolicLinkTest);

	AnyExtensions::Iterator<ROAnything> aIterator(GetTestCaseConfig());
	ROAnything roaConfig;
	while ( aIterator(roaConfig) ) {
		String strTmpDir = roaConfig["TmpDir"].AsString("/tmp");
		String strRelDir = roaConfig["PathToCreate"].AsString();
		String strCreateDir(strTmpDir);
		strCreateDir.Append(System::Sep()).Append(strRelDir);
		String strLinkRel = roaConfig["Link"].AsString(), strLinkAbs(strTmpDir);
		strLinkAbs.Append(System::Sep()).Append(roaConfig["Link"].AsString());
		Trace("Dir to create [" << strCreateDir << "] Link [" << strLinkRel << "]");

		if ( strCreateDir.Length() > 0L ) {
			// assume that we have a tmp-directory to access and to play with
			if ( !System::IsDirectory(strCreateDir) ) {
				assertComparem( System::eSuccess, equal_to, System::MakeDirectory(strCreateDir, 0755, false) , "expected creation of relative directory to succeed" );
			}
			if ( t_assert(System::IsDirectory(strCreateDir)) ) {
				String wd;
				System::GetCWD(wd);
				if ( t_assert( System::ChangeDir(strTmpDir) ) ) {
					Trace("creating relative link [" << strLinkRel << "] to relative dir [" << strRelDir << "]");
					assertComparem( System::eSuccess, equal_to, System::CreateSymbolicLink(strRelDir, strLinkRel) , "expected creation of relative symbolic link to succeed" );
					t_assertm(System::IsSymbolicLink(strLinkRel), "expected link to be valid");
					assertComparem( System::eSuccess, equal_to, System::RemoveDirectory(strLinkRel) , "expected removal of relative symbolic link to succeed" );
					t_assert(System::IsDirectory(strRelDir));
					if ( t_assert(!System::IsDirectory(strLinkRel) ) ) {
						Trace("creating relative link [" << strLinkRel << "] to absolute dir [" << strCreateDir << "]");
						assertComparem( System::eSuccess, equal_to, System::CreateSymbolicLink(strCreateDir, strLinkRel) , "expected creation of relative symbolic link to succeed" );
						t_assertm(System::IsSymbolicLink(strLinkRel), "expected link to be valid");
						assertComparem( System::eSuccess, equal_to, System::RemoveDirectory(strLinkRel) , "expected removal of relative symbolic link to succeed" );
						t_assert(System::IsDirectory(strCreateDir));
					}
				}
				System::ChangeDir(wd);
				Trace("creating absolute link [" << strLinkAbs << "] to dir [" << strCreateDir << "]");
				assertComparem( System::eSuccess, equal_to, System::CreateSymbolicLink(strCreateDir, strLinkAbs) , "expected creation of absolute symbolic link to succeed" );
				t_assertm(System::IsSymbolicLink(strLinkAbs), "expected link to be valid");
				assertComparem( System::eSuccess, equal_to, System::RemoveDirectory(strLinkAbs) , "expected removal of absolute symbolic link to succeed" );
				t_assert(System::IsDirectory(strCreateDir));
			}
			assertComparem( System::eSuccess, equal_to, System::RemoveDirectory(strCreateDir) , "expected removal of directory to succeed" );
		}
	}
}

void SystemFileTest::GetFileSizeTest()
{
	StartTrace(SystemFileTest.GetFileSizeTest);
	String path(System::GetFilePath("len5", "tst"));
	if ( t_assertm(path.Length() > 0, "expected file path to be valid") ) {
		ul_long ulSize = 0;
		if ( t_assert(System::GetFileSize(path, ulSize)) ) {
			assertEqualm(5UL, ulSize, "expected same size");
		}
	}
	path = System::GetFilePath("AnythingTest", "any");
	if ( t_assertm(path.Length() > 0, "expected file path to be valid") ) {
		ul_long ulSize = 0;
		if ( t_assert(System::GetFileSize(path, ulSize)) ) {
			assertEqualm(10701UL, ulSize, "expected same size");
		}
	}
}

void SystemFileTest::BlocksLeftOnFSTest()
{
	StartTrace(SystemFileTest.BlocksLeftOnFSTest);
	// can only test that we have still some space left, nothing more for now
	ul_long ulBlocks = 0;
	unsigned long ulBlockSize = 0;
	String fsPath(GetConfig()["BlocksLeftOnFSTest"]["FS"].AsString("/"));
	if ( t_assertm(System::BlocksLeftOnFS(fsPath, ulBlocks, ulBlockSize), "expected function call to succeed") ) {
		t_assertm(ulBlocks > 0, "expected some blocks left on device");
		t_assertm(ulBlockSize > 0, "expected block size not to be 0");
		Trace("blocks: " << (l_long)ulBlocks << " blocksize: " << (long)ulBlockSize);
	}
}

Test *SystemFileTest::suite ()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, SystemFileTest, initPathTest);
	ADD_CASE(testSuite, SystemFileTest, pathListTest);
	ADD_CASE(testSuite, SystemFileTest, rooDirTest);
	ADD_CASE(testSuite, SystemFileTest, IsAbsolutePathTest);
	ADD_CASE(testSuite, SystemFileTest, ResolvePathTest);
	ADD_CASE(testSuite, SystemFileTest, OpenStreamTest);
	ADD_CASE(testSuite, SystemFileTest, OpenStreamWithSearchTest);
	ADD_CASE(testSuite, SystemFileTest, OpenOStreamTest);
	ADD_CASE(testSuite, SystemFileTest, OpenIStreamTest);
	ADD_CASE(testSuite, SystemFileTest, GetFilePathTest);
	ADD_CASE(testSuite, SystemFileTest, dirFileListTest);
	ADD_CASE(testSuite, SystemFileTest, IStreamTest);
	ADD_CASE(testSuite, SystemFileTest, OStreamTest);
	ADD_CASE(testSuite, SystemFileTest, IOStreamTest);
	ADD_CASE(testSuite, SystemFileTest, CWDTests);
	ADD_CASE(testSuite, SystemFileTest, LoadConfigFileTest);
	ADD_CASE(testSuite, SystemFileTest, TimeTest);
	ADD_CASE(testSuite, SystemFileTest, MkRmDirTest);
	ADD_CASE(testSuite, SystemFileTest, MakeRemoveDirectoryTest);
	ADD_CASE(testSuite, SystemFileTest, MakeDirectoryTest);
	ADD_CASE(testSuite, SystemFileTest, SymbolicLinkTest);
	ADD_CASE(testSuite, SystemFileTest, MakeDirectoryExtendTest);
	ADD_CASE(testSuite, SystemFileTest, GetFileSizeTest);
	ADD_CASE(testSuite, SystemFileTest, BlocksLeftOnFSTest);
	ADD_CASE(testSuite, SystemFileTest, statTests);	// needs to be last
	return testSuite;
}
