/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _CONFIG_NTLMAUTH_H
#define _CONFIG_NTLMAUTH_H

// WIN32 settings for Windows NT
#if defined(WIN32)
#include <windows.h>
#ifdef _DLL
#ifdef NTLMAUTH_IMPL
#define EXPORTDECL_NTLMAUTH	__declspec(dllexport)
#else
#define EXPORTDECL_NTLMAUTH	__declspec(dllimport)
#endif
#else
#define EXPORTDECL_NTLMAUTH
#endif
#else
#define EXPORTDECL_NTLMAUTH
#endif

#endif
