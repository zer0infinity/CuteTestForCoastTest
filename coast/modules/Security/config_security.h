/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _CONFIG_SECURITY_H
#define _CONFIG_SECURITY_H

//! DLL specific settings for Windows NT
#if defined(WIN32)
#ifdef _DLL
#include <windows.h>
#ifdef SECURITY_IMPL
#define EXPORTDECL_SECURITY	__declspec(dllexport)
#else
#define EXPORTDECL_SECURITY	__declspec(dllimport)
#endif
#else
#define EXPORTDECL_SECURITY
#endif
#else
#define EXPORTDECL_SECURITY
#endif

#endif
