/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- c-library modules used ---------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "Policy.h"
#include "Page.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "Registry.h"

//--- interface include --------------------------------------------------------
#include "RegistryTest.h"

//---- RegistryTest ----------------------------------------------------------------
RegistryTest::RegistryTest(TString tname) : TestCase(tname), fRegistry(0)
{
}

RegistryTest::~RegistryTest()
{
}

void RegistryTest::setUp ()
{
	fRegistry = Registry::GetRegistry(fName);
} // setUp

void RegistryTest::tearDown ()
{

} // tearDown

void RegistryTest::InstallAliases ( )
{
	// Insert regular objects into the registry
	NotCloned *a = new NotCloned(""), *b = new NotCloned("");
	// these must be on the heap, implicit assumption of Registry
	fRegistry->Register( "A", a );
	fRegistry->Register( "B", b );

	// registry with two elements
	// Find
	assertEqual( (long)a, (long)fRegistry->Find("A"));
	assertEqual( (long)b, (long)fRegistry->Find("B"));

	// Install
	AliasInstaller policy("InstallAliases"); // policy name must match category name otherwise obects are installed in the wrong registry
	Anything args;

	args["A"] = "NewA";				// regular case
	args["A"].Append("SecondA");
	args["B"]["NewB"] = "SubB";		// Aliases with structures are not defined
	args["C"] = "NewC";				// base not present
	fRegistry->Install( args, &policy );
	t_assert( fRegistry->Find( "NewA" ) == a );
	t_assert( fRegistry->Find( "SecondA" ) == a );
	t_assert( fRegistry->Find( "NewB" ) == 0 );
	// for hierarchies the current implementation does not handle
	// intermediary slotnames;
	//         *** remember: this is undocumented behavior ***
	t_assert( fRegistry->Find( "SubB" ) == b );		// but the lowest level is present as expected
	t_assert( fRegistry->Find( "C" ) == 0 );			// must not be created when not present
	t_assert( fRegistry->Find( "NewC" ) == 0 );		// neither

	// Terminate registry
	AliasTerminator terminator("Test");
	// Terminator will terminate a and b, must not do it manually

	fRegistry->Terminate(&terminator);

	// t_assert( fRegistry->Find( "A" ) == 0 );
	// after termination the registry is empty

} // InstallAliases

void RegistryTest::InstallHierarchy ( )
{
	// Insert regular objects into the registry
	Page *a = new Page("A"), *b = new Page("B"), *c = new Page("C");
	Page *d = new Page("D");

	fRegistry->Register( "A", a );
	fRegistry->Register( "B", b );
	fRegistry->Register( "C", c );
	fRegistry->Register( "D", d );

	// registry with two elements
	// Find
	assertEqual( (long)a, (long)fRegistry->Find("A"));
	assertEqual( (long)b, (long)fRegistry->Find("B"));
	assertEqual( (long)c, (long)fRegistry->Find("C"));
	assertEqual( (long)d, (long)fRegistry->Find("D"));

	// Install
	HierarchyInstaller policy("Hierarchy");
	Anything args;

	args["A"] = "NewA";				// regular case
	args["A"].Append("SecondA");
	args["B"]["NewB"] = "SubB";		// hierarchical structure
	args["B"]["NewB"].Append("SecondSubB");
	args["C"]["NewC"] = "SubC";	// only one page in substructure
	args["D"]["NewD"]["SubD"] = MetaThing();	// no page in substructure
	args["Z"] = "NewZ";				// base not present
	fRegistry->Install( args, &policy );
	t_assert( fRegistry->Find( "NewA" ) != 0 );
	t_assert( fRegistry->Find( "SecondA" ) != 0 );
	t_assert( fRegistry->Find( "NewB" ) != 0 );
	// for hierarchies the SlotNames are present also
	t_assert( fRegistry->Find( "SubB" ) != 0 );
	t_assert( fRegistry->Find( "SecondSubB" ) != 0 );
	t_assert( fRegistry->Find( "NewC" ) != 0 );
	t_assert( fRegistry->Find( "SubC" ) != 0 );
	t_assert( fRegistry->Find( "NewD" ) != 0 );
	t_assert( fRegistry->Find( "SubD" ) != 0 );
	t_assert( fRegistry->Find( "AnyArrayImpl" ) == 0 );
	t_assert( fRegistry->Find( "Z" ) == 0 );			// must not be created when not present
	t_assert( fRegistry->Find( "NewZ" ) == 0 );		// neither

	// Terminate registry
	AliasTerminator terminator("Test");
	// Terminator will terminate a, b, c and d, must not do it manually

	fRegistry->Terminate(&terminator);

	// objects are no longer there
	t_assert( fRegistry->Find( "A" ) == 0 );
} // InstallHierarchy

void RegistryTest::GetRegistry ()
{
	Registry *myRegistry = Registry::GetRegistry("NewRegistry");
	NotCloned test("Test");

	t_assert(myRegistry->Find("Key") == 0);

	myRegistry->Register("Key", &test);
	t_assert(myRegistry->Find("Key") == &test);

	Registry *myRegistry2 = Registry::GetRegistry("NewRegistry");
	// must return the same registry
	t_assert(myRegistry2->Find("Key") == &test);
	// therefore the same key must return the same value

	myRegistry->Unregister("Key");
}

void RegistryTest::Constructor ()
{
	t_assert(fRegistry->Find("Something") == 0);
	// there must not be anything in the registry
}

void RegistryTest::TerminateTest()
{
	NotCloned a("terminate1");
	a.MarkStatic();
	NotCloned *b = new NotCloned("terminate2");

	Registry *r = Registry::GetRegistry("TerminateTest");
	t_assert(r != 0);
	if ( r ) {
		r->Register("terminate1", &a);
		r->Register("terminate2", b);

		AliasTerminator at("TerminateTest");
		t_assert(r->Terminate(&at));

		// registry is still there
		t_assert(Registry::GetRegistry("TerminateTest") != 0);
		// static objects are still there
		t_assert(r->Find("terminate1") != 0);

		t_assert(r->Find("terminate2") == 0);
		r->Unregister("terminate1");
	}

}

class TestPage: public Page
{
public:
	TestPage(const char *name) : Page(name) {}
	~TestPage() {}
	IFAObject *Clone() {
		return new TestPage("TestPage");
	}
};

void RegistryTest::InstallHierarchyConfig()
{
	// generate test registry in the central registry
	Registry *registry = Registry::GetRegistry( "InstallerResetTest" );

	// generate test pages which are initially there
	Page *a = new Page("Page");
	Page *b = new TestPage("TestPage");

	// initially registered pages
	registry->Register("Page", a);
	registry->Register("TestPage", b);

	assertEqual( (long)a, (long)registry->Find("Page"));
	assertEqual( (long)b, (long)registry->Find("TestPage"));

	HierarchyInstaller policy("Hierarchy");
	Anything config;
	config["Page"].Append("TestPage");

	registry->Install( config, &policy );

	t_assert(registry->Find("Page") != 0);
	t_assert(registry->Find("TestPage") != 0);

	assertEqual("is it there (Page)?", a->Lookup("TestItem", "Not found"));
	assertEqual("is it there(TestPage)?", b->Lookup("TestItem", "Not found"));

	assertEqual("is it there (PageOnly)?", a->Lookup("TestItem1", "Not found"));
	assertEqual("is it there (PageOnly)?", b->Lookup("TestItem1", "Not found"));

	assertEqual("Not found", a->Lookup("TestItem2", "Not found"));
	assertEqual("is it there (TestPageOnly)?", b->Lookup("TestItem2", "Not found"));

	// Terminate registry
	AliasTerminator terminator("Test");
	// Terminator will terminate a, b, c and d, must not do it manually
	fRegistry->Terminate(&terminator);

}

Test *RegistryTest::suite ()
// collect all test cases for the RegistryStream
{
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(RegistryTest, Constructor));
	testSuite->addTest (NEW_CASE(RegistryTest, GetRegistry));
	testSuite->addTest (NEW_CASE(RegistryTest, InstallAliases));
	testSuite->addTest (NEW_CASE(RegistryTest, InstallHierarchy));
	testSuite->addTest (NEW_CASE(RegistryTest, InstallHierarchyConfig));
	testSuite->addTest (NEW_CASE(RegistryTest, TerminateTest));

	return testSuite;

} // suite
