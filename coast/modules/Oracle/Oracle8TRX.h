/*
 * Oracle8TRX : DataAccessImpl using the OCI version Oracle8
 */

#ifndef _ORACLE8TRX_H
#define _ORACLE8TRX_H

//--- modules used in the interface
#include "config_coastoracle.h"
#include "DataAccessImpl.h"

#include "oci.h"

class O8Connection;
class O8ConnectionManager;

//---- Oracle8TRX -----------------------------------------------------------
// arbitrary input fields may be used to dynamically generate an SQL statment
// the following fields have a predefined semantic:
// {
//		/StartPos	0		# first row of fetched data to be returned
//		/MaxOut		10		# total number of fetched rows that are returned
// }

class EXPORTDECL_COASTORACLE Oracle8TRX: public DataAccessImpl
{
public:
	//--- constructors
	Oracle8TRX(const char *name);
	~Oracle8TRX();

	// returns a new TRX object
	IFAObject *Clone() const;

	//: executes the transaction
	//!param: c - The context of the transaction
	virtual bool Exec(Context &ctx, ParameterMapper *in, ResultMapper *out);

protected:

	bool ConnectOracleUser(const char *name, const char *pw);
	void FetchRowData(Anything &descs, ParameterMapper *pmapIn, ResultMapper *pmapOut, O8Connection *pConnection, Context &ctx);
	void GetRecordData(Anything &descs, Anything &record, bool bTitlesOnce);

	// original sample methods
	bool GetOutputDescription(Anything &desc, ResultMapper *pmapOut, ub2 &fncode, O8Connection *pConnection, Context &ctx);
	bool DefineOutputArea(Anything &desc, ResultMapper *pmapOut, O8Connection *pConnection, Context &ctx);

private:
	static O8ConnectionManager fgConnMgr;

	enum EColumnDesc {
		// dont use 0 since it may be considered to be a char*
		eColumnName = 1,
		eColumnType = 2,
		eDataLength = 3,
		eRawBuf = 4,
		eDefHandle = 5,
		eIndicator = 6,
		eScale = 7,
		eEffectiveLength = 8
	};

	bool DoPrepareSQL( String &command, Context &ctx, ParameterMapper *in);

	void Warning(Context &ctx, ResultMapper *pResultMapper, String str);
	void Error(Context &ctx, ResultMapper *pResultMapper, String str);

	Oracle8TRX();
	Oracle8TRX(const Oracle8TRX &);

	//assignement
	Oracle8TRX &operator=(const Oracle8TRX &);
};

#endif
