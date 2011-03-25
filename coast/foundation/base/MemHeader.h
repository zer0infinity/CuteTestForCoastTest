/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _MemHeader_H
#define _MemHeader_H

#include <sys/types.h>//lint !e537
#if defined(WIN32)
#include <windows.h>
#endif

//!handling headers for memory management
//!supports custom memory management by easing handling of header information
class MemoryHeader
{
public:
	//!the states a chunk of memory can be in
	enum EMemState {
		eFree = 1L,
		eUsed = 2L,
		eNotPooled = 4L,
		eFreeNotPooled = ( eFree | eNotPooled ),
		eUsedNotPooled = ( eUsed | eNotPooled ),
	};

	//!magic cookie to determine memory boundaries
	static const u_long gcMagic;

	//!constructor simplifying settings of values
	MemoryHeader(u_long size, EMemState ems)
		: fMagic(MemoryHeader::gcMagic)
		, fUsableSize(size)
		, fState(ems)
		, fNextFree(0) {
	}
	//!the cookie defining the boundary of the memory
	u_long fMagic;
	//!size of the memory chunk
	u_long fUsableSize;
	//!state a chunk of memory is in
	EMemState fState;
	//!link to next free chunk in pool
	MemoryHeader *fNextFree;
};

#endif
