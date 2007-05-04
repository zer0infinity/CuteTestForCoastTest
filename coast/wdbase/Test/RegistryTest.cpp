/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "RegistryTest.h"
//--- module under test --------------------------------------------------------
#include "Registry.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "Page.h"
//--- c-library modules used ---------------------------------------------------

//---- RegistryTest ----------------------------------------------------------------
RegistryTest::RegistryTest(TString tname)
	: TestCaseType(tname)
	, fRegistry(0)
{
}

RegistryTest::~RegistryTest()
{
}

void RegistryTest::setUp ()
{
	fRegistry = Registry::GetRegistry(fName);
}

void RegistryTest::tearDown ()
{
	fRegistry = Registry::RemoveRegistry(fName);
}
void RegistryTest::InstallAliases ( )
{
	StartTrace(RegistryTest.InstallAliases);
	{
		// Insert regular objects into the registry
		NotCloned *a = new NotCloned(""), *b = new NotCloned("");
		// these must be on the heap, implicit assumption of Registry
		fRegistry->RegisterRegisterableObject( "A", a );
		fRegistry->RegisterRegisterableObject( "B", b );

		// registry with two elements
		assertEqual( (long)a, (long)fRegistry->Find("A"));
		assertEqual( (long)b, (long)fRegistry->Find("B"));

		// Install
		AliasInstaller policy("InstallAliases"); // policy name must match category name otherwise obects are installed in the wrong registry

		Anything args;
		args["A"] = "NewA";				// regular case
		args["B"] = MetaThing();		// create empty installer spec { }
		TraceAny(args, "spec to alias-install");
		t_assertm( fRegistry->Install( args, &policy ), "installation of aliases should succeed");
		t_assert( fRegistry->Find( "NewA" ) == a );

		// Terminate registry
		AliasTerminator terminator("Test");
		// Terminator will terminate a and b, must not do it manually

		t_assertm(fRegistry->Terminate(&terminator), "expected successful termination");
		// after termination the registry is empty
		t_assert( fRegistry->Find( "A" ) == 0 );
	}
	{
		// Insert regular objects into the registry
		NotCloned *a = new NotCloned(""), *b = new NotCloned("");
		// these must be on the heap, implicit assumption of Registry
		fRegistry->RegisterRegisterableObject( "A", a );
		fRegistry->RegisterRegisterableObject( "B", b );

		// registry with two elements
		assertEqual( (long)a, (long)fRegistry->Find("A"));
		assertEqual( (long)b, (long)fRegistry->Find("B"));

		// Install
		AliasInstaller policy("InstallAliases"); // policy name must match category name otherwise obects are installed in the wrong registry

		Anything args;
		args["A"].Append("NewA");		// multiple aliases to same RegisterableObject
		args["A"].Append("SecondA");
		args["B"] = MetaThing();		// create empty installer spec { }
		TraceAny(args, "spec to alias-install");
		t_assertm( fRegistry->Install( args, &policy ), "installation of aliases should succeed");
		t_assert( fRegistry->Find( "NewA" ) == a );
		t_assert( fRegistry->Find( "SecondA" ) == a );

		// Terminate registry
		AliasTerminator terminator("Test");
		// Terminator will terminate a and b, must not do it manually

		t_assertm(fRegistry->Terminate(&terminator), "expected successful termination");
		// after termination the registry is empty
		t_assert( fRegistry->Find( "A" ) == 0 );
	}
}

void RegistryTest::InstallErroneousAliases ( )
{
	StartTrace(RegistryTest.InstallErroneousAliases);
	// Insert regular objects into the registry
	NotCloned *a = new NotCloned(""), *b = new NotCloned("");
	// these must be on the heap, implicit assumption of Registry
	fRegistry->RegisterRegisterableObject( "A", a );
	fRegistry->RegisterRegisterableObject( "B", b );

	// registry with two elements
	assertEqual( (long)a, (long)fRegistry->Find("A"));
	assertEqual( (long)b, (long)fRegistry->Find("B"));

	// Install
	AliasInstaller policy("InstallAliases"); // policy name must match category name otherwise obects are installed in the wrong registry
	Anything args;
	args["A"] = "";				// create unnamed alias which should not get installed...
	TraceAny(args, "spec to unname alias");
	t_assertm( !fRegistry->Install( args, &policy ), "installation of aliases should fail");

	args = Anything();
	args["A"][0L] = MetaThing();	// create empty installer spec at index 0
	TraceAny(args, "spec to empty sub-any");
	t_assertm( !fRegistry->Install( args, &policy ), "installation of non-string aliases should fail");

	// for hierarchies the current implementation of AliasInstaller does not handle intermediary slotnames;
	// -> use HierarchyInstaller instead
	args = Anything();
	args["A"]["bad"] = "NewA";	// Aliases with structures are not defined
	TraceAny(args, "spec to hierarchic alias spec");
	t_assertm( !fRegistry->Install( args, &policy ), "installation of hierarchic aliases should fail");

	args = Anything();
	args["C"] = "NewC";				// base not present
	TraceAny(args, "spec of alias to non-existing RegisterableObject");
	t_assertm( !fRegistry->Install( args, &policy ), "installation of non existing base object should fail");
}

void RegistryTest::InstallHierarchy ( )
{
	StartTrace(RegistryTest.InstallHierarchy);
	// Insert regular objects into the registry
	Page *a = new Page("A"), *b = new Page("B"), *c = new Page("C");
	Page *d = new Page("D");

	fRegistry->RegisterRegisterableObject( "A", a );
	fRegistry->RegisterRegisterableObject( "B", b );
	fRegistry->RegisterRegisterableObject( "C", c );
	fRegistry->RegisterRegisterableObject( "D", d );

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
}

void RegistryTest::GetRegistry ()
{
	StartTrace(RegistryTest.GetRegistry);
	Registry *myRegistry = Registry::GetRegistry("NewRegistry");
	NotCloned test("Test");

	t_assert(myRegistry->Find("Key") == 0);

	myRegistry->RegisterRegisterableObject("Key", &test);
	t_assert(myRegistry->Find("Key") == &test);

	Registry *myRegistry2 = Registry::GetRegistry("NewRegistry");
	// must return the same registry
	t_assert(myRegistry2->Find("Key") == &test);
	// therefore the same key must return the same value

	myRegistry->UnregisterRegisterableObject("Key");
}

void RegistryTest::Constructor ()
{
	StartTrace(RegistryTest.Constructor);
	t_assert(fRegistry->Find("Something") == 0);
	// there must not be anything in the registry
}

void RegistryTest::TerminateTest()
{
	StartTrace(RegistryTest.TerminateTest);
	NotCloned a("terminate1");
	a.MarkStatic();
	NotCloned *b = new NotCloned("terminate2");

	Registry *r = Registry::GetRegistry("TerminateTest");
	t_assert(r != 0);
	if ( r ) {
		r->RegisterRegisterableObject("terminate1", &a);
		r->RegisterRegisterableObject("terminate2", b);

		AliasTerminator at("TerminateTest");
		t_assert(r->Terminate(&at));

		// registry is still there
		t_assert(Registry::GetRegistry("TerminateTest") != 0);
		// static objects are still there
		t_assert(r->Find("terminate1") != 0);

		t_assert(r->Find("terminate2") == 0);
		r->UnregisterRegisterableObject("terminate1");
	}
}

class TestPage: public Page
{
public:
	TestPage(const char *name) : Page(name) {}
	~TestPage() {}
	IFAObject *Clone() const {
		return new TestPage("TestPage");
	}
};

void RegistryTest::InstallHierarchyConfig()
{
	StartTrace(RegistryTest.InstallHierarchyConfig);
	// generate test registry in the central registry
	Registry *registry = Registry::GetRegistry( "InstallerResetTest" );

	// generate test pages which are initially there
	Page *a = new Page("Page");
	Page *b = new TestPage("TestPage");

	// initially registered pages
	registry->RegisterRegisterableObject("Page", a);
	registry->RegisterRegisterableObject("TestPage", b);

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
{
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, RegistryTest, Constructor);
	ADD_CASE(testSuite, RegistryTest, GetRegistry);
	ADD_CASE(testSuite, RegistryTest, InstallAliases);
	ADD_CASE(testSuite, RegistryTest, InstallErroneousAliases);
	ADD_CASE(testSuite, RegistryTest, InstallHierarchy);
	ADD_CASE(testSuite, RegistryTest, InstallHierarchyConfig);
	ADD_CASE(testSuite, RegistryTest, TerminateTest);

	return testSuite;

}
