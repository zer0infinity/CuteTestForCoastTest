/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _CONFIG_MTFOUNDATION_H
#define _CONFIG_MTFOUNDATION_H

//! DLL specific settings for Windows NT
#if defined(WIN32)
#ifdef _DLL
#include "Anything.h"
extern Anything fgThreads;
extern DWORD fgThreadPtrKey;
class SimpleMutex;
extern SimpleMutex fgThreadsMutex;
#ifdef MTFOUNDATION_IMPL
#define EXPORTDECL_MTFOUNDATION	__declspec(dllexport)
#else
#define EXPORTDECL_MTFOUNDATION	__declspec(dllimport)
#endif
void EXPORTDECL_MTFOUNDATION TerminateKilledThreads();
#else
#define EXPORTDECL_MTFOUNDATION
#endif
#else
#define EXPORTDECL_MTFOUNDATION
#endif

#endif
