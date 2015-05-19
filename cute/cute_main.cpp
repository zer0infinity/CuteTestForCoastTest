/*
 * Copyright (c) 2015, David Tran, Faculty of Computer Science,
 * University of Applied Sciences Rapperswil (HSR),
 * 8640 Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file lgpl.txt.
 */

#include "ide_listener.h"
#include "xml_listener.h"
#include "cute_counting_listener.h"
#include "cute_runner.h"
#include "cute.h"
#include "cute_case.h"
#include <iostream>

const char * setupSuite(cute::suite &s) {
	return "AllTests";
}

int main(int argc, char const *argv[]) {
	cute::suite s;
	const char *test = setupSuite(s);
	cute::xml_file_opener xmlfile(argc, argv);
	cute::xml_listener<cute::counting_listener<cute::ide_listener<> > > lis(xmlfile.out);
	cute::makeRunner(lis)(s, test);
	std::cerr << lis.numberOfTests << " Tests - expect " << s.size() << std::endl;
	std::cerr << lis.successfulTests << " successful - expect " << s.size() << std::endl;
	std::cerr << lis.failedTests << " failed - expect 0 failures" << std::endl;
	std::cerr << lis.errors << " errors - expect 0 errors" << std::endl;
	return lis.failedTests;
}
