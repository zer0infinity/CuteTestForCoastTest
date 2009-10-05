/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef ORACLESTATEMENT_H_
#define ORACLESTATEMENT_H_

#include "config_coastoracle.h"
#include "OracleConnection.h"
#include "IFAObject.h"
#include "AllocatorNewDelete.h"
#include <algorithm>	// std::max

class OracleResultset;

//! <b>Abstraction of an Oracle statement</b>
/*!
 * A statement is either a simple SQL query or a stored procedure call. The OracleStatement class is the main
 * interface when executing queries/SP and to retrieve result data. Several methods exist to
 * obtain either simple column/parameter values or a complete OracleResultset object to process row results.
 *
 * Internally the state of the statement is tracked using an enumerator OracleStatement::Status. The primary state just
 * after creating a new OracleStatement is OracleStatement::UNPREPARED. After executing Prepare(), the state changes to
 * OracleStatement::PREPARED to signal that the underlying OCI call was successful. From now on, we know the type of
 * statement which is going to be executed.
 *
 * Two categories of statements exist in general, simple statements and PL/SQL statements. The two types are a bit
 * different when dealing with parameters. Simple queries do not need parameter binding unless explicitly requested.
 * PL/SQL statements need binding as soon as we use parameters to transport information. Additionally, parameter
 * description must be collected in a different way than for simple statements.
 *
 * If the statement type is OracleStatement::STMT_BEGIN, we have to bind the input parameters before continuing
 * to execute() the statement.
 *
 * If execution of the statement was successful, the state either changes to OracleStatement::RESULT_SET_AVAILABLE or
 * to OracleStatement::UPDATE_COUNT_AVAILABLE depending on the type of the statement.
 *
 * @section DescribeSimpleSQL Simple Statements
 * - \c OracleStatement::RESULT_SET_AVAILABLE \n
 * 		Tells us that we can call getResultset() to get a valid OracleResultset object and work on it to fetch all
 * 		rows of the query.\n
 * 		Please use OracleResultsetPtr to automatically track destruction of the OracleResultset after use.
 * - \c OracleStatement::UPDATE_COUNT_AVAILABLE \n
 * 		Tells us how many rows were affected on the database by the query.
 *
 * @section DescribePLSQL PL/SQL stored procedure/function
 * - \c OracleStatement::RESULT_SET_AVAILABLE \n
 * 		Not implemented
 * - \c OracleStatement::UPDATE_COUNT_AVAILABLE \n
 * 		Tells us that we can now walk through the procedure parameters and retrieve either simple parameter values
 * 		using getString() or using getCursor() to process the results of a cursor type parameter\n
 * 		Please use OracleResultsetPtr to automatically track destruction of the OracleResultset after use.
 */
class EXPORTDECL_COASTORACLE OracleStatement: public IFAObject, public Coast::AllocatorNewDelete
{
public:
	//! Tracking internal state of statement processing
	enum Status {
		UNPREPARED, 			//!< UNPREPARED
		PREPARED, 				//!< PREPARED
		RESULT_SET_AVAILABLE, 	//!< RESULT_SET_AVAILABLE
		UPDATE_COUNT_AVAILABLE,	//!< UPDATE_COUNT_AVAILABLE
		NEEDS_STREAM_DATA, 		//!< NEEDS_STREAM_DATA
		STREAM_DATA_AVAILABLE,	//!< STREAM_DATA_AVAILABLE
	};

	//! after statement preparation, the type of statement is known
	enum StmtType {
#ifndef OCI_STMT_UNKNOWN
#define OCI_STMT_UNKNOWN 0	//!< define value for oracle ic-libs <11.x
#endif
#ifndef OCI_STMT_CALL
#define OCI_STMT_CALL 10	//!< define value for oracle ic-libs <11.x
#endif
		STMT_UNKNOWN = OCI_STMT_UNKNOWN,//!< unknown statement
		STMT_SELECT = OCI_STMT_SELECT,  //!< select statement
		STMT_UPDATE = OCI_STMT_UPDATE,  //!< update statement
		STMT_DELETE = OCI_STMT_DELETE,  //!< delete statement
		STMT_INSERT = OCI_STMT_INSERT,  //!< insert Statement
		STMT_CREATE = OCI_STMT_CREATE,  //!< create statement
		STMT_DROP = OCI_STMT_DROP,      //!< drop statement
		STMT_ALTER = OCI_STMT_ALTER,    //!< alter statement
		STMT_BEGIN = OCI_STMT_BEGIN,    //!< begin (pl/sql statement)
		STMT_DECLARE = OCI_STMT_DECLARE,//!< declare (pl/sql statement )
		STMT_CALL = OCI_STMT_CALL,      //!< corresponds to kpu call
	};

	//! Type to bind parameter
	enum BindType {
		INTERNAL,//!< Use type of parameter description as bind type
		CURSOR,  //!< Explicitly change parameter type to CURSOR
		STRING   //!< Explicitly change parameter type to STRING
	};
	//! Defines valid statement execution modes
	enum ExecMode {
		EXEC_DEFAULT = OCI_DEFAULT,	//!< Use default execution mode
		EXEC_COMMIT = OCI_COMMIT_ON_SUCCESS, //!< Commit statement after successful completion
		EXEC_SCROLLABLE_RO = OCI_STMT_SCROLLABLE_READONLY, //!< Special cursor processing mode where absolute or relative navigation is possible
	};

	class Description
	{
		friend class StatementDescriptionTest;
		ROAnything fRBuf;
		MetaThing fWBuf;
		Description(const Description &);
		Description &operator=(const Description &);
	public:
		class Element
		{
			friend class StatementDescriptionTest;
			ROAnything fRBuf;
			Anything fWBuf;
			//! shorthand for At(const char *slotname) const
			ROAnything operator[](const char *slotname) const {
				ROAnything roaValue( fRBuf[slotname] );
				if ( fWBuf.IsDefined(slotname) ) {
					roaValue = fWBuf[slotname];
				}
				return roaValue;
			}
		public:
			Element() {}
			Element(const Element &elt) : fRBuf(elt.fRBuf), fWBuf(elt.fWBuf) {}
			Element &operator=(const Element &elt) {
				fRBuf = elt.fRBuf;
				fWBuf = elt.fWBuf;
				return *this;
			}
			Element(ROAnything &roaEntry, Anything &anyEntry) : fRBuf(roaEntry), fWBuf(anyEntry) {}
			~Element() {}
			/*! returns a String representation of the implementation if any is set else the default, the string copies memory
				beware of temporaries */
			String AsString(const char *slotname, const char *dflt = 0) const {
				String strValue = operator[](slotname).AsString(dflt);
				return strValue;
			}

			//! returns a long representation of the implementation if possible else the default
			long AsLong(const char *slotname, long dflt = 0) const {
				long lValue = operator[](slotname).AsLong(dflt);
				return lValue;
			}

			/*! returns a const char * representation of the implementation if any is set else the default
				this method doesn't copy memory */
			const char *AsCharPtr(const char *slotname, const char *dflt = 0) const {
				return operator[](slotname).AsCharPtr(dflt);
			}

			//!shorthand for At(const char *slotname)
			Anything &operator[](const char *slotname) {
				return fWBuf[slotname];
			}
		};
		Description() {}
		~Description() {}
		/*! Retrieve the number of slots in this Anything
			\return The number of slots in this Anything */
		long GetSize() const {
			return std::max(fWBuf.GetSize(), fRBuf.GetSize());
		}

		//!assignment operator creates Anything of type a.GetType() returns
		Description &operator= (const ROAnything &a) {
			fRBuf = a;
			return *this;
		}

		/*! Appends a new slot and stores <I>a</I> in it.
			\param a the Anything added at the end of this Anything.
			\return the index of the slot where <I>a</I> is now stored. */
		long Append(const Anything &a) {
			return fWBuf.Append(a);
		}
		//!returns name of slot (if any)
		const char *SlotName(long slot) const {
			static const char *gSlotname = "";
			return gSlotname;
		}

		//! shorthand for At(long slot)
		Element operator[](long slot) {
			ROAnything roaR( fRBuf[slot] );
			Anything anyW;
			if ( !fWBuf.IsDefined(slot) ) {
				fWBuf[slot] = MetaThing(fWBuf.GetAllocator());
			}
			anyW = fWBuf[slot];
			Element aElt(roaR, anyW);
			return aElt;
		}
	};
private:
	OracleStatement();
	OracleConnection *fpConnection;
	String fStmt;
	StmtHandleType fStmthp;
	Anything fErrorMessages;
	Status fStatus;
	StmtType fStmtType;
	Anything fSubStatements;
	Description fDescriptions;
	bool AllocHandle();
	void Cleanup();

	OracleStatement( OracleConnection *pConn, OCIStmt *phStmt );

	sword bindColumn( long lBindPos, OracleStatement::Description::Element &aDescEl, long len, bool bIsNull = false );
public:
	/*! Create a new statement using the given connection and statement String
	 * @param pConn Underlying OracleConnection object to use for operation
	 * @param strStmt String representation of the query, either simple or PL/SQL
	 */
	OracleStatement( OracleConnection *pConn, String const &strStmt );
	~OracleStatement();

	/*! Apply syntax checking on statement
	 * @return true in case the current statement is valid
	 */
	bool Prepare();
	/*! Execute the prepared statement
	 * @param mode ExecMode of statement execution
	 * @return Usually the Status after execution will be either RESULT_SET_AVAILABLE or UPDATE_COUNT_AVAILABLE to
	 * signal that Fetch can be called
	 */
	Status execute( ExecMode mode );
	/*! Fetch the given number of rows into the internally allocated column buffers
	 * @param numRows Number of rows to fetch at once
	 * @return OCI status of executing the OCI fetch command
	 */
	sword Fetch( ub4 numRows = 1 );

	/*! Obtain the current statement Status
	 * @return Current Status of the statement
	 */
	Status status() const {
		return fStatus;
	}
	StmtType getStatementType() const {
		return fStmtType;
	}
	String getStatement() const {
		return fStmt;
	}

	void setPrefetchRows( long lPrefetchRows );
	unsigned long getUpdateCount() const;

	OracleResultsetPtr getResultset();
	OracleResultsetPtr getCursor( long lColumnIndex );
	String getString( long lColumnIndex );

	void registerOutParam( long lBindPos, BindType bindType = INTERNAL, long lBufferSize = -1 );
	void setString( long lBindPos, String const &strValue );

	OCIStmt *getHandle() const {
		return fStmthp.getHandle();
	}
	OracleConnection *getConnection() const {
		return fpConnection;
	}

	OracleStatement::Description &GetOutputDescription();
	bool DefineOutputArea();
	void setSPDescription( ROAnything roaSPDescription, const String &strReturnName );

	const Anything &GetErrorMessages() const {
		return fErrorMessages;
	}
	String GetLastErrorMessage() const {
		return ROAnything( fErrorMessages )[fErrorMessages.GetSize() - 1L].AsString( "" );
	}

protected:
	virtual IFAObject *Clone() const {
		return NULL;
	}
};

#endif /* ORACLESTATEMENT_H_ */
