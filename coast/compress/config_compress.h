/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _CONFIG_COMPRESS_H
#define _CONFIG_COMPRESS_H

// WIN32 settings for Windows NT
#if defined(WIN32)
#ifdef _DLL
#include <windows.h>
#ifdef COMPRESS_IMPL
#define EXPORTDECL_COMPRESS	__declspec(dllexport)
#else
#define EXPORTDECL_COMPRESS	__declspec(dllimport)
#endif
#else
#define EXPORTDECL_COMPRESS
#endif
#else
#define EXPORTDECL_COMPRESS
#endif

#endif
