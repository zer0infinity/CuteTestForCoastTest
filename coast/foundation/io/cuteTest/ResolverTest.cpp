/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "Resolver.h"
#include "ResolverTest.h"

ROAnything ResolverTest::GetTestCaseConfig(String strClassName, String strTestName) {
	Anything fConfig;
	if(!coast::system::LoadConfigFile(fConfig, "ResolverTest", "any")) {
		ASSERT_EQUAL("'read ResolverTest.any'", "'could not read ResolverTest.any'");
	}
	return fConfig[strClassName][strTestName];
}

void ResolverTest::simpleDNS2IPTest()
{
	StartTrace(ResolverTest.simpleDNS2IPTest);
	ROAnything roaConfig;
	AnyExtensions::Iterator<ROAnything, ROAnything, std::string> aEntryIterator(
		GetTestCaseConfig());
	while (aEntryIterator.Next(roaConfig)) {
		std::string strCase;
		if (!aEntryIterator.SlotName(strCase)) {
			strCase.append("idx:").append((const char *)aEntryIterator.Index());
		}
		String expIP, resIP;
		expIP = roaConfig["ip"].AsString();
		resIP = Resolver::DNS2IPAddress(roaConfig["name"].AsString());
		ASSERT_EQUALM(std::string("Failed at: ").append(strCase), expIP, resIP);
	}
}

void ResolverTest::simpleIP2DNSTest()
{
	StartTrace(ResolverTest.simpleIP2DNSTest);
	ROAnything roaConfig;
	AnyExtensions::Iterator<ROAnything, ROAnything, std::string> aEntryIterator(
		GetTestCaseConfig());
	while (aEntryIterator.Next(roaConfig)) {
		std::string strCase;
		if (!aEntryIterator.SlotName(strCase)) {
			strCase.append("idx:").append((const char *)aEntryIterator.Index());
		}
		String expDN, expFQDN, resDN, ip;
		expDN = roaConfig["name"].AsString("localhost");
		expFQDN = roaConfig["fqdn"].AsString();
		ip = roaConfig["ip"].AsString();
		resDN = Resolver::IPAddress2DNS(ip);
		if ( !expDN.IsEqual(resDN) && !expFQDN.IsEqual(resDN) ) {
			ASSERT_EQUALM(std::string("Failed at ").append(strCase), expDN, resDN);
		}
	}
}

void ResolverTest::runAllTests(cute::suite &s) {
	s.push_back(CUTE_SMEMFUN(ResolverTest, simpleDNS2IPTest));
	s.push_back(CUTE_SMEMFUN(ResolverTest, simpleIP2DNSTest));

}
