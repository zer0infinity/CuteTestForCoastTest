/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _MemHeader_H
#define _MemHeader_H

#include "config_foundation.h" // for definition of u_long EXPORTDECL_FOUNDATION
#include <sys/types.h>
//!handling headers for memory management
//!supports custom memory management by easing handling of header information
class EXPORTDECL_FOUNDATION MemoryHeader
{
public:
	//!the states a chunk of memory can be in
	enum EMemState { eFree = 1L, eUsed = 2L, eNotPooled = 4L, eFreeNotPooled = 5L, eUsedNotPooled = 6L };

	//!magic cookie to determine memory boundaries
	static const u_long gcMagic;

	//!constructor simplifying settings of values
	MemoryHeader(u_long size, EMemState ems) : fMagic(MemoryHeader::gcMagic), fSize(size), fState(ems), fNext(0) { }
	//!the cookie defining the boundary of the memory
	u_long fMagic;
	//!size of the memory chunk
	u_long fSize;
	//!state a chunk of memory is in
	EMemState fState;
	//!link to next chunk
	MemoryHeader *fNext;
	//!align memory suitably
	static inline size_t MemoryHeader::AlignedSize() {
		const size_t rest =  sizeof(MemoryHeader) % sizeof(long double);
		const size_t alignedsize = sizeof(MemoryHeader) + ( rest ? sizeof(long double) - rest : 0);
		return alignedsize;
	}
};

#endif
