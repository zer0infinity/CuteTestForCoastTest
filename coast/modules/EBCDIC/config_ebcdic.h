/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _CONFIG_EBCDIC_H
#define _CONFIG_EBCDIC_H

//! DLL specific settings for Windows NT
#if defined(WIN32)
#ifdef _DLL
#include <windows.h>
#ifdef EBCDIC_IMPL
#define EXPORTDECL_EBCDIC	__declspec(dllexport)
#else
#define EXPORTDECL_EBCDIC	__declspec(dllimport)
#endif
#else
#define EXPORTDECL_EBCDIC
#endif
#else
#define EXPORTDECL_EBCDIC
#endif

#endif
