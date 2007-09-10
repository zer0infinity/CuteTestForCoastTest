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
#if !defined(WD_DISABLE_TRACE)
		StatTrace(BoostPoolUserAllocatorGlobal.malloc, "size:" << (long)bytes << " @" << (long)pRet, Storage::Global());
#endif
		return pRet;
	}

	void BoostPoolUserAllocatorGlobal::free(char *const block)
	{
#if !defined(WD_DISABLE_TRACE)
		StatTrace(BoostPoolUserAllocatorGlobal.free, "@" << (long)block, Storage::Global());
#endif
		Storage::Global()->Free(block);
	}

	char *BoostPoolUserAllocatorCurrent::malloc(const size_type bytes)
	{
		char *pRet = reinterpret_cast<char *>(Storage::Current()->Malloc(bytes));
#if !defined(WD_DISABLE_TRACE)
		StatTrace(BoostPoolUserAllocatorCurrent.malloc, "size:" << (long)bytes << " @" << (long)pRet, Storage::Current());
#endif
		return pRet;
	}

	void BoostPoolUserAllocatorCurrent::free(char *const block)
	{
#if !defined(WD_DISABLE_TRACE)
		StatTrace(BoostPoolUserAllocatorCurrent.free, "@" << (long)block, Storage::Current());
#endif
		Storage::Current()->Free(block);
	}
};
