/*
 * OracleDAImpl : DataAccessImpl using the OCI version Oracle8
 */

//--- interface include --------------------------------------------------------
#include "OracleDAImpl.h"

//--- standard modules used ----------------------------------------------------
//#include "OracleConnectionManager.h"
#include "OracleConnection.h"
#include "OracleModule.h"
#include "SysLog.h"
#include "Timers.h"
#include "StringStream.h"
#include "TimeStamp.h"
#include "Dbg.h"
#include "AnyIterators.h"

#include <limits>
#include <algorithm>

//--- c-library modules used ---------------------------------------------------
#include <cstring>

static const long glStringBufferSize( 4096L );

//---- OracleDAImpl ---------------------------------------------------------
RegisterDataAccessImpl( OracleDAImpl);

OracleDAImpl::OracleDAImpl(const char *name) :
	DataAccessImpl(name)
{
	StartTrace(OracleDAImpl.OracleDAImpl);
}

OracleDAImpl::~OracleDAImpl()
{
	StartTrace(OracleDAImpl.~OracleDAImpl);
}

IFAObject *OracleDAImpl::Clone() const
{
	StartTrace(OracleDAImpl.Clone);
	return new OracleDAImpl(fName);
}

bool OracleDAImpl::Exec(Context &ctx, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(OracleDAImpl.Exec);
	bool bRet( false );
	DAAccessTimer(OracleDAImpl.Exec, fName, ctx);
	OracleModule *pModule = SafeCast(WDModule::FindWDModule("OracleModule"), OracleModule);
	ConnectionPool *pConnectionPool(0);
	if ( pModule && ( pConnectionPool = pModule->GetConnectionPool()) ) {
		// we need the server and user to see if there is an existing and Open OracleConnection
		String user, server, passwd;
		in->Get("DBUser", user, ctx);
		in->Get("DBPW", passwd, ctx);
		in->Get("DBConnectString", server, ctx);
		Trace("USER IS:" << user);
		Trace("Connect string is [" << server << "]");
		out->Put("QuerySource", server, ctx);

		bool bIsOpen = false, bDoRetry = true;
		long lTryCount(1L);
		in->Get("DBTries", lTryCount, ctx);
		if (lTryCount < 1) {
			lTryCount = 1L;
		}
		// we slipped through and are ready to get a connection and execute a query
		// find a free OracleConnection, we should always get a valid OracleConnection here!
		while (bDoRetry && --lTryCount >= 0) {
			OracleConnection *pConnection = NULL;
			String command;

			// --- establish db connection
			if ( !pConnectionPool->BorrowConnection(pConnection, bIsOpen, server, user) ) {
				Error(ctx, out, "unable to get OracleConnection");
				bDoRetry = false;
			} else {
				if ( bIsOpen || pConnection->Open(server, user, passwd) ) {
					bIsOpen = true;
					OCIError *eh = pConnection->ErrorHandle();
					if (DoPrepareSQL(command, ctx, in)) {
						bool error(false);
						String strErr;
						error = pConnection->checkError(pConnection->StmtPrepare((text *) (const char *) command), strErr);
						if (error) {
							Error(ctx, out, strErr);
						}
						out->Put("Query", command, ctx);
						ub2 fncode = 0;
						MetaThing desc;
						if (!error) {
							// --- determine format of retrieved data
							error = GetOutputDescription(desc, out, fncode, pConnection, ctx); // determine format of output
							TraceAny(desc, "row description");
						}
						if (fncode == OCI_STMT_SELECT) {
							if (!error) {
								error = DefineOutputArea(desc, out, pConnection, ctx);
							}

							if (!error) {
								// --- map header information
								String resultformat, resultsize;
								in->Get("DBResultFormat", resultformat, ctx);

								bool bTitlesOnce = resultformat.IsEqual("TitlesOnce");
								//FIXME: TitlesAlways should be possible too
								if (bTitlesOnce) {
									// put column name information
									Anything temp;
									for (sword col = 0; col < desc.GetSize(); ++col) {
										String strColName(desc[col][eColumnName].AsString());
										Trace("colname@" << col << " [" << strColName << "]");
										temp[strColName] = (long) col;
									}
									out->Put("QueryTitles", temp, ctx);
								}

								//FIXME: implement result size limit by kB
								// in->Get("DBMaxResultSize", resultsize, ctx);
								// --- get data rows
								FetchRowData(desc, in, out, pConnection, ctx);
								bRet = true;
								bDoRetry = false;
							}
						} else {
							error = pConnection->checkError(pConnection->ExecuteStmt(), strErr);
							if (error) {
								Error(ctx, out, strErr);
							}

							// cancel cursor
							if (!error) {
								error = pConnection->checkError(OCIStmtFetch(pConnection->StmtHandle(), eh, (ub4) 0, (ub4) OCI_FETCH_NEXT, (ub4) OCI_DEFAULT), strErr);
								if (error) {
									Error(ctx, out, strErr);
								}
							}
						}
						// according to documentation, def handles will get cleaned too
						pConnection->StmtCleanup();
					} else if (DoPrepareSP(command, ctx, in)) {
						Trace("STORED PROCEDURE IS [" << command << "]");
						bool error(false);
						String strErr;

						MetaThing desc;
						bool bIsFunction(false);
						error = GetSPDescription(command, bIsFunction, desc, in, out, pConnection, ctx);

						String strSP( ConstructSPStr(command, bIsFunction, desc) );

						Trace(String("prepare stored procedure: ") << strSP);
						error = pConnection->checkError(pConnection->StmtPrepare((text *) (const char *) strSP), strErr);
						if (error) {
							Error(ctx, out, strErr);
						}

						if (!error) {
							out->Put("Query", strSP, ctx);
							error = BindSPVariables(desc, in, out, pConnection, ctx);
						}
						if (!error) {
							error = pConnection->checkError(OCIStmtExecute(pConnection->SvcHandle(), pConnection->StmtHandle(), pConnection->ErrorHandle(), 1, 0, (const OCISnapshot *) 0, (OCISnapshot *) 0, OCI_DEFAULT), strErr);
							if (error) {
								Error(ctx, out, strErr);
							}
							if (!error) {
								Anything record;
								GetRecordData(desc, record, false); //TitlesOnce
								TraceAny(record, "fetched result");
								AnyExtensions::Iterator<Anything> recordIter(record);
								Anything anyEntry;
								String slotname;
								while (recordIter.Next(anyEntry)) {
									recordIter.SlotName(slotname);
									out->Put(slotname, anyEntry, ctx);
								}

//								bool bShowRowCount(true);
//								in->Get("ShowQueryCount", bShowRowCount, ctx);
//								if (bShowRowCount) {
//									// append summary if extract of available data is returned
//									out->Put("QueryCount", 1L, ctx);
//								}
								bRet = true;
								bDoRetry = false;

								// FIXME: can we detect stored procedure return codes?
								// out->Put("SP_Retcode", strRetcode, ctx);
							}
						}
					} else {
						out->Put("Messages", "Rendered slot SQL resulted in an empty string", ctx);
						bDoRetry = false;
					}
				}
			}
			if (pConnection) {
				pConnectionPool->ReleaseConnection(pConnection, bIsOpen, server, user);
			}
			if (bDoRetry && lTryCount > 0) {
				SYSWARNING("Internally retrying to execute command [" << command << "]");
			}
		}
	} else {
		if ( !pModule ) {
			SYSERROR("unable to get OracleModule, aborting!");
		} else {
			SYSERROR("unable to get ConnectionPool, aborting!");
		}
	}
	return bRet;
}

bool OracleDAImpl::DoPrepareSQL(String &command, Context &ctx, ParameterMapper *in)
{
	StartTrace(OracleDAImpl.DoPrepareSQL);
	DAAccessTimer(OracleDAImpl.DoPrepareSQL, fName, ctx);
	OStringStream os(command);
	in->Get("SQL", os, ctx);
	os.flush();
	SubTrace(Query, "QUERY IS [" << command << "]");
	return (command.Length() > 0L);
}

bool OracleDAImpl::DoPrepareSP(String &command, Context &ctx, ParameterMapper *in)
{
	StartTrace(OracleDAImpl.DoPrepareSP);
	DAAccessTimer(OracleDAImpl.DoPrepareSP, fName, ctx);
	return in->Get("SP", command, ctx);
}

void OracleDAImpl::Warning(Context &ctx, ResultMapper *pResultMapper, String str)
{
	StartTrace(OracleDAImpl.Warning);
	String strErr("OracleDAImpl::");
	strErr.Append(str);
	SysLog::Warning(TimeStamp::Now().AsStringWithZ().Append(' ').Append(strErr));
	if (pResultMapper) {
		pResultMapper->Put("Messages", strErr, ctx);
	}
}

void OracleDAImpl::Error(Context &ctx, ResultMapper *pResultMapper, String str)
{
	StartTrace(OracleDAImpl.Error);
	String strErr("OracleDAImpl::");
	strErr.Append(str);
	SysLog::Error(TimeStamp::Now().AsStringWithZ().Append(' ').Append(strErr));
	if (pResultMapper) {
		pResultMapper->Put("Messages", strErr, ctx);
	}
}

bool OracleDAImpl::GetOutputDescription(Anything &desc, ResultMapper *pmapOut, ub2 &fncode, OracleConnection *pConnection, Context &ctx)
{
	StartTrace(OracleDAImpl.GetOutputDescription);
	// returns array of element descriptions: each description is
	// an Anything array with 4 entries:
	// name of the collumn, type of the data, length of the data in bytes, scale

	OCIParam *mypard;
	ub2 dtype;
	//	sb2					scale;
	ub4 data_len;
	text *col_name;
	ub4 col_name_len;

	bool error = false;
	OCIError *eh = pConnection->ErrorHandle();
	String strErr;

	OCIStmt *pStmthp(pConnection->StmtHandle()); // OCI statement handle

	error = pConnection->checkError(OCIAttrGet((dvoid *) pStmthp, OCI_HTYPE_STMT, (dvoid *) &fncode, (ub4 *) 0, OCI_ATTR_STMT_TYPE, eh), strErr);
	if (error) {
		Error(ctx, pmapOut, strErr);
		return error;
	}

	if (fncode == OCI_STMT_SELECT) {

		error = pConnection->checkError(pConnection->GetReplyDescription(), strErr);
		if (error) {
			Error(ctx, pmapOut, strErr);
			return error;
		}

		// Request a parameter descriptor for position 1 in the select-list
		ub4 counter = 1;
		sb4 parm_status = OCIParamGet(pStmthp, OCI_HTYPE_STMT, eh, (void **) &mypard, (ub4) counter);

		error = pConnection->checkError(parm_status, strErr);
		if (error) {
			Error(ctx, pmapOut, strErr);
			return error;
		}

		// Loop only if a descriptor was successfully retrieved for
		// current  position, starting at 1

		while (parm_status == OCI_SUCCESS) {
			data_len = 0;
			col_name = 0;
			col_name_len = 0;

			// Retrieve the data type attribute
			error = pConnection->checkError(OCIAttrGet((dvoid *) mypard, (ub4) OCI_DTYPE_PARAM, (dvoid *) &dtype, (ub4 *) 0, (ub4) OCI_ATTR_DATA_TYPE,
											(OCIError *) eh), strErr);
			if (error) {
				Error(ctx, pmapOut, strErr);
				return error;
			}

			error = pConnection->checkError(OCIAttrGet((dvoid *) mypard, (ub4) OCI_DTYPE_PARAM, (dvoid *) &data_len, (ub4 *) 0, (ub4) OCI_ATTR_DISP_SIZE,
											(OCIError *) eh), strErr);
			if (error) {
				Error(ctx, pmapOut, strErr);
				return error;
			}

			//			error = pConnection->checkError(OCIAttrGet((dvoid*) mypard, (ub4) OCI_DTYPE_PARAM,
			//							 (dvoid*) &scale,(ub4 *) 0, (ub4) OCI_ATTR_SCALE,
			//							(OCIError *) eh), strErr);

			// Retrieve the column name attribute
			error = pConnection->checkError(OCIAttrGet((dvoid *) mypard, (ub4) OCI_DTYPE_PARAM, (dvoid **) &col_name, (ub4 *) &col_name_len,
											(ub4) OCI_ATTR_NAME, (OCIError *) eh), strErr);
			if (error) {
				Error(ctx, pmapOut, strErr);
				return error;
			}

			Anything param;
			param[0L] = Anything(); // dummy
			param[eColumnName] = String((char *) col_name, col_name_len);
			param[eColumnType] = dtype;
			param[eDataLength] = (int) data_len;
			//			if (dtype == SQLT_NUM)
			//				param[eScale]= (sb2)scale;
			desc.Append(param);

			// increment counter and get next descriptor, if there is one
			++counter;
			parm_status = OCIParamGet(pStmthp, OCI_HTYPE_STMT, eh, (void **) &mypard, (ub4) counter);
		}
	}
	TraceAny(desc, "descriptions");
	return error;
}

bool OracleDAImpl::DefineOutputArea(Anything &desc, ResultMapper *pmapOut, OracleConnection *pConnection, Context &ctx)
{
	StartTrace(OracleDAImpl.DefineOutputArea);
	// use 'desc' to allocate output area used by oracle library
	// to store fetched data (binary Anything buffers are allocated and
	// stored within the 'desc' structure... for automatic storage
	// management)

	bool error = false;
	OCIError *eh = pConnection->ErrorHandle();
	OCIStmt *pStmthp(pConnection->StmtHandle()); // OCI statement handle
	String strErr;

	for (ub4 i = 0; i < desc.GetSize(); i++) {
		Anything &col = desc[static_cast<long> (i)];

		sb4 dummy;
		OCIDefine *defHandle = 0;
		long len;
		if (col[eColumnType].AsLong() == SQLT_DAT) {
			// --- date field

			col[eDataLength] = 9;
			col[eColumnType] = SQLT_STR;

			len = col[eDataLength].AsLong() + 1;

		} else if (col[eColumnType].AsLong() == SQLT_NUM) {
			// --- number field

			//			if (col[eScale].AsLong() != 0) {					// dont need floats now
			//				col[eDataLength]= (sword) sizeof(float);
			//				col[eColumnType]= SQLT_FLT;
			//            } else {
			col[eDataLength] = (sword) sizeof(sword);
			col[eColumnType] = SQLT_INT;
			//            }
			len = col[eDataLength].AsLong();

		} else {
			len = col[eDataLength].AsLong() + 1;
		}

		// allocate space for the returned data
		Anything buf = Anything((void *) 0, len);
		col[eRawBuf] = buf;

		// accocate space for NULL indicator
		Anything indicator = Anything((void *) 0, sizeof(OCIInd));
		col[eIndicator] = indicator;

		// allocate space to store effective result size
		Anything effectiveSize = Anything((void *) 0, sizeof(ub2));
		col[eEffectiveLength] = effectiveSize;

		sword status = OCIDefineByPos(pStmthp, &defHandle, eh, i + 1, (void *) buf.AsCharPtr(), len, col[eColumnType].AsLong(),
									  (dvoid *) indicator.AsCharPtr(), (ub2 *) effectiveSize.AsCharPtr(), 0, OCI_DEFAULT);
		error = pConnection->checkError(status);
		if (error) {
			Error(ctx, pmapOut, pConnection->errorMessage(status));
			return error;
		}
	}
	return error;
}

void OracleDAImpl::GetRecordData(Anything &descs, Anything &record, bool bTitlesOnce)
{
	StartTrace(OracleDAImpl.GetRecordData);
	// extract the values from a fetched row of data

	for (sword col = 0; col < descs.GetSize(); col++) {
		Anything &desc = descs[col], value;
		SubTraceAny(TraceDesc, desc, "desc at col:" << (long) col);
		SubTrace(TraceColType, "column type is: " << desc[eColumnType].AsLong() << " indicator: " << desc[eIndicator].AsLong());
		SubTrace(TraceBuf, "buf ptr " << (long) (desc[eRawBuf].AsCharPtr()) << " length: " << (long) * ((ub2 *) desc[eEffectiveLength].AsCharPtr()));
		switch (desc[eColumnType].AsLong()) {
			case SQLT_INT:
				Trace("SQLT_INT");
				if (desc[eIndicator].AsLong() == OCI_IND_NULL) {
					value = 0L;
				} else {
					value = (*((sword *) desc[eRawBuf].AsCharPtr()));
				}
				break;
			case SQLT_FLT:
				Trace("SQLT_FLT");
				if (desc[eIndicator].AsLong() == OCI_IND_NULL) {
					value = 0.0f;
				} else {
					value = (*((float *) desc[eRawBuf].AsCharPtr()));
				}
				break;
			case SQLT_STR:
				Trace("SQLT_STR");
				if (desc[eIndicator].AsLong() == OCI_IND_NULL) {
					value = "";
				} else {
					value = String(desc[eRawBuf].AsCharPtr());
				}
				break;
			default:
				SubTraceBuf(TraceBuf, desc[eRawBuf].AsCharPtr(), *((ub2 *) desc[eEffectiveLength].AsCharPtr()));
				if (desc[eIndicator].AsLong() == OCI_IND_NULL) {
					value = "";
				} else {
					value = String(static_cast<void *> (const_cast<char *> (desc[eRawBuf].AsCharPtr())),
								   (long) * (reinterpret_cast<ub2 *> (const_cast<char *> (desc[eEffectiveLength].AsCharPtr()))));
				}
				break;
		}
		if (bTitlesOnce) {
			record.Append(value);
		} else {
			record[desc[eColumnName].AsCharPtr()] = value;
		}
	}
	TraceAny(record, "row record");
}

void OracleDAImpl::FetchRowData(Anything &descs, ParameterMapper *pmapIn, ResultMapper *pmapOut, OracleConnection *pConnection, Context &ctx)
{
	StartTrace(OracleDAImpl.FetchRowData);

	// return subset of the fetched data rows (if 'start' or 'count'
	// is <0, then all the data rows are returned)

	TraceAny(descs, "descriptions");

	bool bRet(true);
	OCIError *eh = pConnection->ErrorHandle();

	// --- successful execute returns the 1st row

	sword rc = pConnection->ExecuteStmt();
	OCIStmt *pStmthp(pConnection->StmtHandle()); // OCI statement handle

	String strMaxRows;
	pmapIn->Get("DBMaxRows", strMaxRows, ctx);

	String resultformat;
	pmapIn->Get("DBResultFormat", resultformat, ctx);

	bool bTitlesOnce(resultformat.IsEqual("TitlesOnce"));

	long rowCount(0L), lMaxRows(strMaxRows.AsLong(std::numeric_limits<long>::max()));

	while (bRet && (rc == OCI_SUCCESS || rc == OCI_SUCCESS_WITH_INFO) && rowCount < lMaxRows) {
		// fetch data into preallocated areas within 'descs'
		// (all the data is fetched - and possibly discarded - to
		// determine the total number auf results)

		// --- return only selected rows (brute force implementation)
		Anything record;
		GetRecordData(descs, record, bTitlesOnce);
		SubTraceAny(TraceRow, record, "putting into QueryResult for row " << (long) (rowCount));
		bRet = pmapOut->Put("QueryResult", record, ctx);

		rc = OCIStmtFetch(pStmthp, eh, (ub4) 1, (ub4) OCI_FETCH_NEXT, (ub4) OCI_DEFAULT);
		++rowCount;
	};

	if (pConnection->checkError(rc)) {
		Error(ctx, pmapOut, pConnection->errorMessage(rc));
	}

	bool bShowRowCount(true);
	pmapIn->Get("ShowQueryCount", bShowRowCount, ctx);
	if (bShowRowCount) {
		// append summary if extract of available data is returned
		pmapOut->Put("QueryCount", rowCount, ctx);
	}
}

bool OracleDAImpl::GetSPDescription(String const &spname, bool &pIsFunction, Anything &desc, ParameterMapper *pmapIn, ResultMapper *pmapOut, OracleConnection *pConnection, Context &ctx)
{
	StartTrace(OracleDAImpl.GetSPDescription);
	// returns array of element descriptions: each description is
	// an Anything array with 4 entries:
	// name of the column, type of the data, length of the data in bytes, scale

	text const *objptr = reinterpret_cast<const text *> ((const char *)spname);
	bool error(false);
	String strErr;

	pIsFunction = false;
	Trace("get the describe handle for the procedure");
	error = pConnection->checkError(OCIDescribeAny(pConnection->SvcHandle(), pConnection->ErrorHandle(), (dvoid *)objptr, (ub4)strlen((char *)objptr), OCI_OTYPE_NAME, 0, OCI_PTYPE_PROC, pConnection->DscHandle()), strErr);
	if (error) {
		pIsFunction = true;
		error = pConnection->checkError(OCIDescribeAny(pConnection->SvcHandle(), pConnection->ErrorHandle(), (dvoid *)objptr, (ub4)strlen((char *)objptr), OCI_OTYPE_NAME, 0, OCI_PTYPE_FUNC, pConnection->DscHandle()), strErr);
		if (error) {
			Error(ctx, pmapOut, String("DB-Object is neither procedure nor function (") << strErr << ")");
			return error;
		}
	}

	Trace("get the parameter handle");
	OCIParam *parmh( 0 );
	error = pConnection->checkError(OCIAttrGet(pConnection->DscHandle(), OCI_HTYPE_DESCRIBE, (dvoid *)&parmh, (ub4 *)0, OCI_ATTR_PARAM, pConnection->ErrorHandle()), strErr);
	if (error) {
		Error(ctx, pmapOut, strErr);
		return error;
	}

	Trace("get the number of arguments and the arg list");
	OCIParam *arglst( 0 );
	ub2 numargs = 0;
	error = pConnection->checkError(OCIAttrGet((dvoid *)parmh, OCI_DTYPE_PARAM, (dvoid *)&arglst, (ub4 *)0, OCI_ATTR_LIST_ARGUMENTS, pConnection->ErrorHandle()), strErr);
	if (error) {
		Error(ctx, pmapOut, strErr);
		return error;
	}

	error = pConnection->checkError(OCIAttrGet((dvoid *)arglst, OCI_DTYPE_PARAM, (dvoid *)&numargs, (ub4 *)0, OCI_ATTR_NUM_PARAMS, pConnection->ErrorHandle()), strErr);
	if (error) {
		Error(ctx, pmapOut, strErr);
		return error;
	}

	Trace(String("number of arguments: ") << numargs);

	OCIParam *arg( 0 );
	text *name;
	ub4 namelen;
	ub2 dtype;
	OCITypeParamMode iomode;
	ub4 data_len;

	// For a procedure, we begin with i = 1; for a function, we begin with i = 0.
	int start = 0;
	int end = numargs;
	if (!pIsFunction) {
		++start;
		++end;
	}

	for (int i = start; i < end; ++i) {
		error = pConnection->checkError(OCIParamGet((dvoid *) arglst, OCI_DTYPE_PARAM, pConnection->ErrorHandle(), (dvoid **) &arg, (ub4) i), strErr);
		if (error) {
			Error(ctx, pmapOut, strErr);
			return error;
		}
		namelen = 0;
		name = 0;
		data_len = 0;

		error = pConnection->checkError(OCIAttrGet((dvoid *) arg, OCI_DTYPE_PARAM, (dvoid *) &dtype, (ub4 *) 0, OCI_ATTR_DATA_TYPE, pConnection->ErrorHandle()), strErr);
		if (error) {
			Error(ctx, pmapOut, strErr);
			return error;
		}
		Trace("Data type: " << dtype);

		error = pConnection->checkError(OCIAttrGet((dvoid *) arg, OCI_DTYPE_PARAM, (dvoid *) &name, (ub4 *) &namelen, OCI_ATTR_NAME, pConnection->ErrorHandle()), strErr);
		if (error) {
			Error(ctx, pmapOut, strErr);
			return error;
		}
		String strName((char *) name, namelen);
		// the first param of a function is the return param
		if (pIsFunction && i == start && strName.Length() == 0) {
			Trace("Overriding return param name");
			strName = spname;
			pmapIn->Get( "Return", strName, ctx );
		}
		Trace("Name: " << strName);

		// 0 = IN (OCI_TYPEPARAM_IN), 1 = OUT (OCI_TYPEPARAM_OUT), 2 = IN/OUT (OCI_TYPEPARAM_INOUT)
		error = pConnection->checkError(OCIAttrGet((dvoid *) arg, OCI_DTYPE_PARAM, (dvoid *) &iomode, (ub4 *) 0, OCI_ATTR_IOMODE, pConnection->ErrorHandle()), strErr);
		if (error) {
			Error(ctx, pmapOut, strErr);
			return error;
		}
		Trace("IO type: " << iomode);

		error = pConnection->checkError(OCIAttrGet((dvoid *) arg, OCI_DTYPE_PARAM, (dvoid *) &data_len, (ub4 *) 0, OCI_ATTR_DATA_SIZE, pConnection->ErrorHandle()), strErr);
		if (error) {
			Error(ctx, pmapOut, strErr);
			return error;
		}
		Trace("Size: " << (int)data_len);

		Anything param;
		param[0L] = Anything(); // dummy
		param[eColumnName] = strName;
		param[eColumnType] = dtype;
		param[eDataLength] = (int) data_len;
		param[eInOutType] = iomode;
		desc.Append(param);
	}

	TraceAny(desc, "stored procedure description");
	return error;
}

struct ConstructPlSql {
	String &fStr;
	ConstructPlSql(String &str): fStr(str) {}
	void operator()(const Anything &anyParam) {
		if ( fStr.Length() ) {
			fStr.Append(',');
		}
		fStr.Append(':').Append(anyParam[OracleDAImpl::eColumnName].AsString());
	}
};

String OracleDAImpl::ConstructSPStr( String &command, bool pIsFunction, Anything const &desc )
{
	String plsql, strParams;
	plsql << "BEGIN ";
	Anything_const_iterator begin( desc.begin() );
	if (pIsFunction) {
		plsql << ":" << (*begin).At(OracleDAImpl::eColumnName).AsString() << " := ";
		++begin;
	}
	std::for_each(begin, desc.end(), ConstructPlSql(strParams));
	plsql << command << "(" << strParams << "); END;";
	StatTrace(OracleDAImpl.ConstructSPStr, "SP string [" << plsql << "]", Storage::Current());
	return plsql;
}

bool OracleDAImpl::BindSPVariables(Anything &desc, ParameterMapper *pmapIn, ResultMapper *pmapOut, OracleConnection *pConnection, Context &ctx)
{
	StartTrace(OracleDAImpl.BindSPVariables);
	// use 'desc' to allocate output area used by oracle library
	// to bind variables (binary Anything buffers are allocated and
	// stored within the 'desc' structure... for automatic storage
	// management)

	bool error(false);
	String strErr;
	sword status;

	AnyExtensions::Iterator<Anything> descIter(desc);
	Anything col;
	while (descIter.Next(col)) {
		long bindPos( descIter.Index() + 1 ); // first bind variable position is 1

		long len;
		Anything buf;
		String strValue;
		String strParamname( col[eColumnName].AsString() );

		if (col[eInOutType] == OCI_TYPEPARAM_IN || col[eInOutType] == OCI_TYPEPARAM_INOUT) {
			if ( !pmapIn->Get( String("Params.").Append(strParamname), strValue, ctx ) ) {
				Error(ctx, pmapOut, String("In(out) parameter [") << strParamname << "] not found in config, is it defined in upper case letters?");
				return true;
			}
			col[eDataLength] = strValue.Length();
			col[eColumnType] = SQLT_STR;
			len = col[eDataLength].AsLong() + 1;
			buf = Anything((void *)(const char *)strValue, len);
		} else {
			switch ( col[eColumnType].AsLong() ) {
				case SQLT_DAT:
					col[eDataLength] = 9;
					col[eColumnType] = SQLT_STR;
					len = col[eDataLength].AsLong() + 1;
					break;
				case SQLT_CHR:
				case SQLT_STR: {
					long lBufSize( glStringBufferSize );
					pmapIn->Get( "StringBufferSize", lBufSize, ctx );
					col[eDataLength] = lBufSize;
					col[eColumnType] = SQLT_STR;
					len = col[eDataLength].AsLong() + 1;
					break;
				}
				case SQLT_NUM:
					col[eDataLength] = 38;
					col[eColumnType] = SQLT_STR;
					len = col[eDataLength].AsLong() + 1;
					break;
				case SQLT_CUR:
				case SQLT_RSET: {
					//TODO
					col[eColumnType] = SQLT_RSET;
					break;
				}
				default:
					len = col[eDataLength].AsLong() + 1;
					break;
			}
			buf = Anything((void *) 0, len);
		}

		col[eRawBuf] = buf;
		Trace("binding variable: " << strParamname << ", length: " << len);

		// allocate space for NULL indicator
		Anything indicator = Anything((void *) 0, sizeof(OCIInd));
		col[eIndicator] = indicator;

		// allocate space to store effective result size
		ub2 ub2Len( (ub2)col[eDataLength].AsLong(0L) );
		Anything effectiveSize = Anything((void *)(ub2 *)&ub2Len, sizeof(ub2));
		col[eEffectiveLength] = effectiveSize;

		OCIBind *bndp = 0;
		status = OCIBindByPos(pConnection->StmtHandle(), &bndp, pConnection->ErrorHandle(),
							  (ub4) bindPos, (ub1 *) col[eRawBuf].AsCharPtr(), (sword) len, col[eColumnType].AsLong(),
							  (dvoid *) indicator.AsCharPtr(), (ub2 *) 0, (ub2) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT);
		error = pConnection->checkError(status, strErr);
		if (error) {
			Error(ctx, pmapOut, strErr);
			return error;
		}
	}

	TraceAny(desc, "bind description");
	return error;
}
