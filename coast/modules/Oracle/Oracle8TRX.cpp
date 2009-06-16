/*
 * Oracle8TRX : DataAccessImpl using the OCI version Oracle8
 */

//--- interface include --------------------------------------------------------
#include "Oracle8TRX.h"

//--- standard modules used ----------------------------------------------------
#include "O8ConnectionManager.h"
#include "O8Connection.h"
#include "SysLog.h"
#include "Timers.h"
#include "StringStream.h"
#include "TimeStamp.h"
#include "Action.h"
#include "PeriodicAction.h"
#include "Dbg.h"

#include <limits>

//--- c-library modules used ---------------------------------------------------
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>

using namespace std;

//---- Oracle8TRX ---------------------------------------------------------
O8ConnectionManager Oracle8TRX::fgConnMgr;

RegisterDataAccessImpl( Oracle8TRX);

Oracle8TRX::Oracle8TRX(const char *name) :
	DataAccessImpl(name)
{
	StartTrace(Oracle8TRX.Oracle8TRX);
}

Oracle8TRX::~Oracle8TRX()
{
	StartTrace(Oracle8TRX.~Oracle8TRX);
}

IFAObject *Oracle8TRX::Clone() const
{
	StartTrace(Oracle8TRX.Clone);
	return new Oracle8TRX(fName);
}

bool Oracle8TRX::Exec(Context &ctx, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(Oracle8TRX.Exec);
	bool bRet = false;
	DAAccessTimer(Oracle8TRX.Exec, fName, ctx);
	// check if we are initialized
	bool bInitialized = true;
	if (bInitialized) {
		// we need the server and user to see if there is an existing and Open O8Connection
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
		// find a free O8Connection, we should always get a valid O8Connection here!
		while (bDoRetry && --lTryCount >= 0) {
			O8Connection *pConnection = NULL;
			String command;

			// --- establish db connection
			//FIXME: add server to get connection
			if (!(pConnection = fgConnMgr.BorrowConnection((text *) (const char *) user, (text *) (const char *) passwd))) {
				Error(ctx, out, "unable to get O8Connection");
				bDoRetry = false;
			} else {
				bool error(false);

				// --- determine format of retrieved data
				MetaThing desc;
				String strErr;
				OCIError *eh = pConnection->ErrorHandle();

				if (!error) {
					if (DoPrepareSQL(command, ctx, in)) {
						strErr = pConnection->checkerr(eh, pConnection->StmtPrepare((text *) (const char *) command), error);
						if (error) {
							Error(ctx, out, strErr);
						}
						out->Put("Query", command, ctx);
						ub2 fncode = 0;
						if (!error) {
							error = GetOutputDescription(desc, out, fncode, pConnection, ctx);    // determine format of output
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
//								FIXME: can we detect stored procedure return codes?
//								out->Put("SP_Retcode", strRetcode, ctx);
							}
						} else {
							strErr = pConnection->checkerr(eh, pConnection->ExecuteStmt(), error);
							if (error) {
								Error(ctx, out, strErr);
							}

							// cancel cursor
							if (!error) {
								OCIStmt *pStmthp(pConnection->StmtHandle()); // OCI statement handle
								strErr = pConnection->checkerr(eh, OCIStmtFetch(pStmthp, eh, (ub4) 0, (ub4) OCI_FETCH_NEXT, (ub4) OCI_DEFAULT), error);
								if (error) {
									Error(ctx, out, strErr);
								}
							}
						}

					} else {
						out->Put("Messages", "Rendered slot SQL resulted in an empty string", ctx);
						bDoRetry = false;
					}
				}
				// of all the dynamically allocated data only the define handles
				// are not cleaned up automatically when destroying 'desc'

				for (long i = 0; i < desc.GetSize(); ++i) {
					// --- cleanup define handles
					strErr = pConnection->checkerr(eh, OCIHandleFree((dvoid *) desc[i][eDefHandle].AsIFAObject(), (ub4) OCI_HTYPE_DEFINE), error);
					if (error) {
						Error(ctx, out, strErr);
					}

				}
				pConnection->StmtCleanup();
			}
			if (pConnection) {
				fgConnMgr.ReleaseConnection(pConnection);
			}
			if (bDoRetry && lTryCount > 0) {
				SYSWARNING("Internally retrying to execute command [" << command << "]");
			}
		}
	}
	return bRet;
}

bool Oracle8TRX::DoPrepareSQL(String &command, Context &ctx, ParameterMapper *in)
{
	StartTrace(Oracle8TRX.DoPrepareSQL);
	DAAccessTimer(Oracle8TRX.DoPrepareSQL, fName, ctx);
	OStringStream os(command);
	in->Get("SQL", os, ctx);
	os.flush();
	SubTrace(Query, "QUERY IS [" << command << "]");
	return (command.Length() > 0L);
}

void Oracle8TRX::Warning(Context &ctx, ResultMapper *pResultMapper, String str)
{
	StartTrace(Oracle8TRX.Warning);
	String strErr("Oracle8TRX::");
	strErr.Append(str);
	SysLog::Warning(TimeStamp::Now().AsStringWithZ().Append(' ').Append(strErr));
	if (pResultMapper) {
		pResultMapper->Put("Messages", strErr, ctx);
	}
}

void Oracle8TRX::Error(Context &ctx, ResultMapper *pResultMapper, String str)
{
	StartTrace(Oracle8TRX.Error);
	String strErr("Oracle8TRX::");
	strErr.Append(str);
	SysLog::Error(TimeStamp::Now().AsStringWithZ().Append(' ').Append(strErr));
	if (pResultMapper) {
		pResultMapper->Put("Messages", strErr, ctx);
	}
}

bool Oracle8TRX::GetOutputDescription(Anything &desc, ResultMapper *pmapOut, ub2 &fncode, O8Connection *pConnection, Context &ctx)
{
	// returns array of element descriptions: each description is
	// an Anything array with 4 entries:
	// name of the collumn, type of the data, length of the data in bytes, scale

	OCIParam *mypard;
	ub2 dtype;
	//	sb2					scale;
	ub4 data_len;
	text *col_name;
	ub4 col_name_len;

	bool error = FALSE;
	OCIError *eh = pConnection->ErrorHandle();

	OCIStmt *pStmthp(pConnection->StmtHandle()); // OCI statement handle
	String strErr;
	strErr = pConnection->checkerr(eh, OCIAttrGet((dvoid *) pStmthp, OCI_HTYPE_STMT, (dvoid *) &fncode, (ub4 *) 0, OCI_ATTR_STMT_TYPE, eh), error);
	if (error) {
		Error(ctx, pmapOut, strErr);
	}

	if (fncode == OCI_STMT_SELECT) {

		strErr = pConnection->checkerr(eh, pConnection->GetReplyDescription(), error);
		if (error) {
			Error(ctx, pmapOut, strErr);
		}

		// Request a parameter descriptor for position 1 in the select-list
		ub4 counter = 1;
		sb4 parm_status = OCIParamGet(pStmthp, OCI_HTYPE_STMT, eh, (void **) &mypard, (ub4) counter);

		strErr = pConnection->checkerr(eh, parm_status, error);
		if (error) {
			Error(ctx, pmapOut, strErr);
		}

		// Loop only if a descriptor was successfully retrieved for
		// current  position, starting at 1

		while (parm_status == OCI_SUCCESS) {

			data_len = 0;
			col_name = 0;
			col_name_len = 0;

			// Retrieve the data type attribute
			strErr = pConnection->checkerr(eh, OCIAttrGet((dvoid *) mypard, (ub4) OCI_DTYPE_PARAM, (dvoid *) &dtype, (ub4 *) 0, (ub4) OCI_ATTR_DATA_TYPE,
										   (OCIError *) eh), error);
			if (error) {
				Error(ctx, pmapOut, strErr);
			}

			strErr = pConnection->checkerr(eh, OCIAttrGet((dvoid *) mypard, (ub4) OCI_DTYPE_PARAM, (dvoid *) &data_len, (ub4 *) 0, (ub4) OCI_ATTR_DISP_SIZE,
										   (OCIError *) eh), error);
			if (error) {
				Error(ctx, pmapOut, strErr);
			}

			//			checkerr(eh, OCIAttrGet((dvoid*) mypard, (ub4) OCI_DTYPE_PARAM,
			//							 (dvoid*) &scale,(ub4 *) 0, (ub4) OCI_ATTR_SCALE,
			//							(OCIError *) eh), error, pmapOut);

			// Retrieve the column name attribute
			strErr = pConnection->checkerr(eh, OCIAttrGet((dvoid *) mypard, (ub4) OCI_DTYPE_PARAM, (dvoid **) &col_name, (ub4 *) &col_name_len,
										   (ub4) OCI_ATTR_NAME, (OCIError *) eh), error);

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
	return error;
}

bool Oracle8TRX::DefineOutputArea(Anything &desc, ResultMapper *pmapOut, O8Connection *pConnection, Context &ctx)
{
	// use 'desc' to allocate output area used by oracle library
	// to store fetched data (binary Anything buffers are allocated and
	// stored within the 'desc' structure... for automatic storage
	// management)

	bool error = FALSE;
	OCIError *eh = pConnection->ErrorHandle();
	OCIStmt *pStmthp(pConnection->StmtHandle()); // OCI statement handle
	String strErr;

	for (ub4 i = 0; i < desc.GetSize(); i++) {
		Anything &col = desc[ static_cast<long>(i)];

		sb4 dummy;
		OCIDefine *defHandle = 0; // caution: handle needs to be cleaned up manually
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

		strErr = pConnection->checkerr(eh, OCIDefineByPos(pStmthp, &defHandle, eh, i + 1, (void *) buf.AsCharPtr(), len, col[eColumnType].AsLong(),
									   (dvoid *) indicator.AsCharPtr(), (ub2 *) effectiveSize.AsCharPtr(), 0, OCI_DEFAULT), error);
		if (error) {
			Error(ctx, pmapOut, strErr);
		}
		// CAUTION: unlike the other data the define handle is *NOT* automatically
		//          garbage collected!
		col[eDefHandle] = Anything((IFAObject *) defHandle);
	}
	return error;
}

void Oracle8TRX::GetRecordData(Anything &descs, Anything &record, bool bTitlesOnce)
{
	// extract the values from a fetched row of data

	for (sword col = 0; col < descs.GetSize(); col++) {
		Anything &desc = descs[col], value;

		switch (desc[eColumnType].AsLong()) {
			case SQLT_INT:
				if (desc[eIndicator].AsLong() == OCI_IND_NULL) {
					value = 0L;
				} else {
					value = (*((sword *) desc[eRawBuf].AsCharPtr()));
				}
				break;
			case SQLT_FLT:
				if (desc[eIndicator].AsLong() == OCI_IND_NULL) {
					value = 0.0f;
				} else {
					value = (*((float *) desc[eRawBuf].AsCharPtr()));
				}
				break;
			default:
				if (desc[eIndicator].AsLong() == OCI_IND_NULL) {
					value = "";
				} else {
					value = ((char *) desc[eRawBuf].AsCharPtr(), *((ub2 *) desc[eEffectiveLength].AsCharPtr()));
				}
				break;
		}
		if ( bTitlesOnce ) {
			record.Append( value );
		} else {
			record[ desc[col][eColumnName].AsCharPtr() ] = value;
		}
	}
}

void Oracle8TRX::FetchRowData(Anything &descs, ParameterMapper *pmapIn, ResultMapper *pmapOut, O8Connection *pConnection, Context &ctx)
{
	// return subset of the fetched data rows (if 'start' or 'count'
	// is <0, then all the data rows are returned)

	bool error = FALSE, bRet(true);
	OCIError *eh = pConnection->ErrorHandle();

	// --- successful execute returns the 1st row

	sword rc = pConnection->ExecuteStmt();
	OCIStmt *pStmthp(pConnection->StmtHandle()); // OCI statement handle

	String strMaxRows;
	pmapIn->Get("DBMaxRows", strMaxRows, ctx);

	String resultformat;
	pmapIn->Get("DBResultFormat", resultformat, ctx);

	bool bTitlesOnce( resultformat.IsEqual("TitlesOnce") );

	long rowCount(0L), lMaxRows( strMaxRows.AsLong(std::numeric_limits<long>::max()) );

	while ( bRet && ( rc == OCI_SUCCESS || rc == OCI_SUCCESS_WITH_INFO ) && rowCount < lMaxRows ) {
		// fetch data into preallocated areas within 'descs'
		// (all the data is fetched - and possibly discarded - to
		// determine the total number auf results)

		// --- return only selected rows (brute force implementation)
		Anything record;
		GetRecordData(descs, record, bTitlesOnce);
		TraceAny(temp, "putting into QueryResult for row " << (long)(rowCount));
		bRet = pmapOut->Put("QueryResult", record, ctx);

		rc = OCIStmtFetch(pStmthp, eh, (ub4) 1, (ub4) OCI_FETCH_NEXT, (ub4) OCI_DEFAULT);
		++rowCount;
	};

	String strErr( pConnection->checkerr(eh, rc, error) );
	if ( error ) {
		Error(ctx, pmapOut, strErr);
	}

	bool bShowRowCount( true );
	pmapIn->Get("ShowQueryCount", bShowRowCount, ctx);
	if ( bShowRowCount ) {
		// append summary if extract of available data is returned
		pmapOut->Put("QueryCount", rowCount, ctx);
	}
}
