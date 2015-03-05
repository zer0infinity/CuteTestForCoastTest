/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _a2ee2a_H
#define _a2ee2a_H

#include <sys/types.h>

// void ebcdic2ascii(unsigned char *dest, const unsigned char *srce, size_t count);
void ebcdic2ascii(void *dest, const void *srce, size_t count);
void ascii2ebcdic(void *dest, const void *srce, size_t count);

#endif		//not defined _a2ee2a_H
