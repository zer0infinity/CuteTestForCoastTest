/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SybCTnewDA_H
#define _SybCTnewDA_H

//---- baseclass include -------------------------------------------------------
#include "config_sybasect.h"

//--- standard modules used ----------------------------------------------------
#include "Threads.h"
#include "Mapper.h"

//--- c-library modules used ---------------------------------------------------
#include <ctpublic.h>

//---- SybCTnewDA ----------------------------------------------------------
//:single line description of the class
// further explanation of the purpose of the class
// this may contain <B>HTML-Tags</B>
// ...

#define	CTLIB_VERSION	CS_VERSION_125
#define	BLK_VERSION		BLK_VERSION_125

// Maximum character buffer for displaying a column
#define MAX_CHAR_BUF	4096
// Maximum size of memory block to be used during array binding
#define MAX_MEM_BLOCK_SIZE	50000
// MEM_ALIGN_BOUNDARY - memory alignment boundary
#define MEM_ALIGN_BOUNDARY	8
// MEM_ALIGN_SIZE - Align size to nearest higher multiple  of MEM_ALIGN_BOUNDARY
#define MEM_ALIGN_SIZE(blocksize)	((((blocksize)+MEM_ALIGN_BOUNDARY-1)/MEM_ALIGN_BOUNDARY)*MEM_ALIGN_BOUNDARY)
// EX_GET_COLUMNVALUE - calculate column value pointer given the row number
#define EX_GET_COLUMN_VALUE(coldata, row, col, colfmt) \
	(coldata[col].value + ((row) * (colfmt[col].maxlength)))

class EXPORTDECL_SYBASECT SybCTnewDA : public IFAObject
{
	// Define structure where row data is bound.
	// Allow for column arrays - for array binding.
	typedef struct ColumnData {
		ColumnData(Allocator *a = Storage::Current());
		~ColumnData();
		CS_SMALLINT	*indicator;
		CS_CHAR		*value;
		CS_INT		*valuelen;
		static void *operator new[](size_t size, Allocator *a) throw();
		static void operator delete[](void *ptr);
#if defined(WIN32) && (_MSC_VER >= 1200) // VC6 or greater
		static void operator delete[](void *ptr, Allocator *a);
#endif
		CS_RETCODE AllocateValue(size_t lSize);
		CS_RETCODE AllocateValuelen(CS_INT num_rows);
		CS_RETCODE AllocateIndicator(CS_INT num_rows);

		Allocator	*fAllocator;
	} EX_COLUMN_DATA;

	IFAObject *Clone() const {
		return NULL;
	};

public:
	//--- constructors
	SybCTnewDA(CS_CONTEXT *context);
	~SybCTnewDA();

	//--- public api
	//:single line description of newmethod
	// further explanation of the purpose of the method
	// this may contain <B>HTML-Tags</B>
	// ...
	//!param: aParam - explanation of aParam (important : paramname - explanation )
	//!retv: explanation of return value
	//!prec: explanation of precondition for the method call
	//!postc: explanation of postcondition for the method call
	static CS_RETCODE Init(CS_CONTEXT **context, Anything *pMessages, const String &strInterfacesPathName, CS_INT iNumberOfConns);
	static CS_RETCODE Finis(CS_CONTEXT *context);

	struct DaParams {
		DaParams() : fpContext(NULL), fpIn(NULL), fpOut(NULL), fpDAName(NULL) {};
		DaParams(Context *context, ParameterMapper *in, ResultMapper *out, String *pDAName) : fpContext(context), fpIn(in), fpOut(out), fpDAName(pDAName) {};
		Context *fpContext;
		ParameterMapper *fpIn;
		ResultMapper *fpOut;
		String *fpDAName;
	};
	static bool GetDaParams(DaParams &params, CS_CONNECTION *connection);
	static bool PutMessages(DaParams &daParams, Anything &anyMessages);

	bool Open(DaParams &params, String user, String password, String server, String appl);
	// use either TitlesAlways or TitlesOnce as second param
	bool SqlExec(DaParams &params, String cmd, String resultformat = "TitlesAlways", const long lMaxResultSize = 0L, const long lMaxRows = -1L);
	bool Close(bool bForce = false);

	static bool GetMessageAny(CS_CONTEXT *context, Anything **anyMessage);
	CS_RETCODE SetConProps(CS_INT property, CS_VOID *buffer, CS_INT buflen);
	bool GetConProps(CS_INT property, CS_VOID **propvalue, CS_INT propsize);

	static String GetStringFromRetCode(CS_RETCODE retcode);

protected:
	//--- subclass api
	void	Warning(DaParams &params, String str);
	void	Error(DaParams &params, String str);
	CS_RETCODE	DoFetchData(DaParams &params, CS_COMMAND *cmd, const CS_INT res_type, const String &resultformat, const long lMaxResultSize, const long lParMaxRows, long &lRowsReceived);
	CS_INT 		DisplayDlen(CS_DATAFMT *column);
	bool	DoFillResults(DaParams &params, CS_INT totalrows, CS_INT numrows, CS_INT numcols, CS_DATAFMT *colfmt, EX_COLUMN_DATA *coldata, bool bTitlesOnce );
	static bool IntGetConProps(CS_CONNECTION *connection, CS_INT property, CS_VOID **propvalue, CS_INT propsize);

	//--- member variables declaration
	CS_CONTEXT	*fContext;
	CS_CONNECTION *fConnection;

private:
	// use careful, you inhibit subclass use
	//--- private class api

	//--- private member variables
	static SimpleMutex fgSybaseLocker;

	friend class SybCTnewDATest;
	friend CS_RETCODE SybCTnewDA_csmsg_handler(CS_CONTEXT *context, CS_CLIENTMSG *errmsg);
	friend CS_RETCODE SybCTnewDA_clientmsg_handler(CS_CONTEXT *context, CS_CONNECTION *connection, CS_CLIENTMSG *errmsg);
	friend CS_RETCODE SybCTnewDA_servermsg_handler(CS_CONTEXT *context, CS_CONNECTION *connection, CS_SERVERMSG *srvmsg);
};

#endif
