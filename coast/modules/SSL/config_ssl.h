/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _CONFIG_SSL_H
#define _CONFIG_SSL_H

//! DLL specific settings for Windows NT
#if defined(WIN32)
#ifdef _DLL
#include <windows.h>
#ifdef SSL_IMPL
#define EXPORTDECL_SSL	__declspec(dllexport)
#else
#define EXPORTDECL_SSL	__declspec(dllimport)
#endif
#else
#define EXPORTDECL_SSL
#endif
#else
#define EXPORTDECL_SSL
#endif

typedef struct ssl_ctx_st SSL_CTX;
typedef struct ssl_st SSL;
typedef struct x509_st X509;

#endif
