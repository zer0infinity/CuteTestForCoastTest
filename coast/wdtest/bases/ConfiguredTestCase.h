/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ConfiguredTestCase_H
#define _ConfiguredTestCase_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"
#include "Anything.h"

//---- ConfiguredTestCase ----------------------------------------------------------
//!(abstract) TestCase that loads a config file
class ConfiguredTestCase : public TestCase
{
public:
	//!TestCase constructor
	//! \param name name of the test
	//! \param configFileName The name of the Config file to load (Without extension)
	//! the Config file has to have the extension .any
	ConfiguredTestCase(TString name, TString configFileName);

	//!destroys the test case
	~ConfiguredTestCase();

	//--- public api

	//!sets the environment for this test
	void setUp ();

	//!deletes the environment for this test
	void tearDown ();

protected:
	//! load an Anything from an any file (probably this should be a static method of Anything)
	Anything LoadConfigFile(const TString &configFileName);

	//! copies slots from source to dest
	//! mainly used to initialize context stores ( PutInStore(anAny,ctx.GetTmpStore()) )
	void PutInStore(const Anything &source, Anything &dest);

	//! Name of the Config file to load
	TString fConfigFileName;

	//! The whole configuration file
	Anything fConfig;
	//! The part of the configuration file with the slotname of the TestCase
	Anything fTestCaseConfig;
};

// Iterates over a list of configs, executing body for each config
/* Example:
	RUN_ENTRY("TestPut",cConfig)
	{
		String streamInp(cConfig["StreamInput"].AsString());
		StringStreamSocket sss(streamInp);
		Context ctx(&sss);
		StringStream os;
		ROAnything exp(cConfig["Expected"]);
		ROAnything dummy;
		t_assert(exp["Result"].AsBool() == dbm->Get("",os,ctx,dummy));
		assertEquals(exp["StreamOutput"].AsString(),os.str());
	}
*/
#define RUN_ENTRY(TEST_CONFIG_SLOT,CASE_CONFIG) \
	ROAnything tConfig(fConfig[TEST_CONFIG_SLOT]);\
	t_assert(!tConfig.IsNull());\
	ROAnything CASE_CONFIG(tConfig[0L]);\
	for(long i=0;i<tConfig.GetSize();i++, CASE_CONFIG = tConfig[i])

#define FOREACH_ENTRY(TEST_CONFIG_SLOT,CASE_CONFIG,SLOTNAME) \
	ROAnything tConfig(fConfig[TEST_CONFIG_SLOT]);\
	t_assert(!tConfig.IsNull());\
	ROAnything CASE_CONFIG(tConfig[0L]);\
	TString SLOTNAME(tConfig.SlotName(0L));\
	for(long i=0;i<tConfig.GetSize();i++, CASE_CONFIG = tConfig[i], SLOTNAME=tConfig.SlotName(i))

#endif
