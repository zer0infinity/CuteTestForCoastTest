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
		char *pRet = reinterpret_cast<char *>(Storage::Global()->Malloc(bytes));
		_StatTrace(BoostPoolUserAllocatorGlobal.malloc, "size:" << (long)bytes << " @" << (long)pRet, Storage::Global());
		return pRet;
	}

	void BoostPoolUserAllocatorGlobal::free(char *const block)
	{
		_StatTrace(BoostPoolUserAllocatorGlobal.free, "@" << (long)block, Storage::Global());
		Storage::Global()->Free(block);
	}

	char *BoostPoolUserAllocatorCurrent::malloc(const size_type bytes)
	{
		char *pRet = reinterpret_cast<char *>(Storage::Current()->Malloc(bytes));
		_StatTrace(BoostPoolUserAllocatorCurrent.malloc, "size:" << (long)bytes << " @" << (long)pRet, Storage::Current());
		return pRet;
	}

	void BoostPoolUserAllocatorCurrent::free(char *const block)
	{
		_StatTrace(BoostPoolUserAllocatorCurrent.free, "@" << (long)block, Storage::Current());
		Storage::Current()->Free(block);
	}
};
