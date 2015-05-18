/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "AppBooter.h"
#include <locale>

int main(int argc, const char *argv[]) {
	// initialize locale aspects to default locale -> 'C'
	std::locale::global(std::locale::classic());
	return AppBooter().Run(argc, argv);
}
