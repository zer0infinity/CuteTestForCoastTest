/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _CONFIG_LDAP_H
#define _CONFIG_LDAP_H

//! DLL specific settings for Windows NT
#if defined(WIN32)
#ifdef _DLL
#include <windows.h>
#ifdef LDAP_IMPL
#define EXPORTDECL_LDAP	__declspec(dllexport)
#else
#define EXPORTDECL_LDAP	__declspec(dllimport)
#endif
#else
#define EXPORTDECL_LDAP
#endif
#else
#define EXPORTDECL_LDAP
#endif

#endif
