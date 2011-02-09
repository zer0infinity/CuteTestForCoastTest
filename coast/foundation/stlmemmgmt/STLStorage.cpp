/*
 * Copyright (c) 2007, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------

//--- interface include --------------------------------------------------------
#include "STLStorage.h"

//--- standard modules used ----------------------------------------------------

//--- c-library modules used ---------------------------------------------------

namespace STLStorage
{

	char *BoostPoolUserAllocatorGlobal::malloc(const size_type bytes)
	{
		char *pRet = reinterpret_cast<char *>(Coast::Storage::Global()->Malloc(bytes));
		_StatTrace(BoostPoolUserAllocatorGlobal.malloc, "@" << (long)pRet << " sz:" << (long)bytes, Coast::Storage::Global());
		return pRet;
	}

	void BoostPoolUserAllocatorGlobal::free(char *const block)
	{
		size_t sz(Coast::Storage::Global()->Free(block));
		_StatTrace(BoostPoolUserAllocatorGlobal.free, "@" << (long)block << " sz:" << (long)sz, Coast::Storage::Global());
		(void) sz; // avoid unused variable warning
	}

	char *BoostPoolUserAllocatorCurrent::malloc(const size_type bytes)
	{
		char *pRet = reinterpret_cast<char *>(Coast::Storage::Current()->Malloc(bytes));
		_StatTrace(BoostPoolUserAllocatorCurrent.malloc, "@" << (long)pRet << " sz:" << (long)bytes, Coast::Storage::Current());
		return pRet;
	}

	void BoostPoolUserAllocatorCurrent::free(char *const block)
	{
		size_t sz(Coast::Storage::Current()->Free(block));
		_StatTrace(BoostPoolUserAllocatorCurrent.free, "@" << (long)block << " sz:" << (long)sz, Coast::Storage::Current());
		(void) sz; // avoid unused variable warning
	}
};
