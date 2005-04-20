/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SybCT_H
#define _SybCT_H

//---- baseclass include -------------------------------------------------------
#include "config_sybasect.h"

//--- standard modules used ----------------------------------------------------
#include "Threads.h"

//--- c-library modules used ---------------------------------------------------
#include <ctpublic.h>

//---- SybCT ----------------------------------------------------------
//:single line description of the class
// further explanation of the purpose of the class
// this may contain <B>HTML-Tags</B>
// ...

#define	CTLIB_VERSION	CS_VERSION_125
#define	BLK_VERSION		BLK_VERSION_125

#ifndef MAX
#define MAX(X,Y)	(((X) > (Y)) ? (X) : (Y))
#endif

#ifndef MIN
#define MIN(X,Y)	(((X) < (Y)) ? (X) : (Y))
#endif
// Maximum character buffer for displaying a column
#define MAX_CHAR_BUF	2048
// Maximum size of memory block to be used during array binding
#define MAX_MEM_BLOCK_SIZE	50000
// MEM_ALIGN_BOUNDARY - memory alignment boundary
#define MEM_ALIGN_BOUNDARY	8
// MEM_ALIGN_SIZE - Align size to nearest higher multiple  of MEM_ALIGN_BOUNDARY
#define MEM_ALIGN_SIZE(blocksize)	((((blocksize)+MEM_ALIGN_BOUNDARY-1)/MEM_ALIGN_BOUNDARY)*MEM_ALIGN_BOUNDARY)
// EX_GET_COLUMNVALUE - calculate column value pointer given the row number
#define EX_GET_COLUMN_VALUE(coldata, row, col, colfmt) \
	(coldata[col].value + ((row) * (colfmt[col].maxlength)))

// Define structure where row data is bound.
// Allow for column arrays - for array binding.
typedef struct ColumnData {
	ColumnData(Allocator *a = Storage::Current());
	~ColumnData();
	CS_SMALLINT	*indicator;
	CS_CHAR		*value;
	CS_INT		*valuelen;
	static void *operator new[](size_t size, Allocator *a);
	static void operator delete[](void *ptr);
#if defined(WIN32) && (_MSC_VER >= 1200) // VC6 or greater
	static void operator delete[](void *ptr, Allocator *a);
#endif
	Allocator	*fAllocator;
} EX_COLUMN_DATA;

class EXPORTDECL_SYBASECT SybCT
{
public:
	//--- constructors
	SybCT(CS_CONTEXT *context);
	~SybCT();

	//--- public api
	//:single line description of newmethod
	// further explanation of the purpose of the method
	// this may contain <B>HTML-Tags</B>
	// ...
	//!param: aParam - explanation of aParam (important : paramname - explanation )
	//!retv: explanation of return value
	//!prec: explanation of precondition for the method call
	//!postc: explanation of postcondition for the method call
	static CS_RETCODE Init(CS_CONTEXT **context, Anything *pMessages, const String &strInterfacesPathName);
	static CS_RETCODE Finis(CS_CONTEXT *context);
	bool Open(Anything *pMessages, String user, String password, String server, String appl);
	// use either TitlesAlways or TitlesOnce as second param
	bool SqlExec(String cmd, String resultformat = "TitlesAlways", const long lMaxResultSize = 0L );
	bool GetResult( Anything &anyResult, Anything &anyTitles );
	bool Close(bool bForce = false);

	static bool GetMessageAny(CS_CONTEXT *context, CS_CONNECTION *connection, Anything **anyMessage);

protected:
	//--- subclass api
	void	Warning(String str);
	void	Error(String str);
	CS_RETCODE	DoFetchData(CS_COMMAND *cmd, const CS_INT res_type, const String &resultformat, const long &lMaxResultSize );
	CS_INT 		DisplayDlen(CS_DATAFMT *column);
	CS_RETCODE	DoFillResults(CS_INT numrows, CS_INT numcols, CS_DATAFMT *colfmt, EX_COLUMN_DATA *coldata, const CS_INT res_type, const String &resultformat);

	//--- member variables declaration
	CS_CONTEXT	*fContext;
	CS_CONNECTION *fConnection;
	Anything	*fpMessages;		// Store error and warnings-messages coming from DataBase
	Anything	fQueryResults;
	Anything 	fQueryColNames;

private:
	// use careful, you inhibit subclass use
	//--- private class api

	//--- private member variables
	static Mutex	fgSybaseLocker;

	friend class SybCTTest;
};

#endif
