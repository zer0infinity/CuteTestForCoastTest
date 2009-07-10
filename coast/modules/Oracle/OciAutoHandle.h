/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef OCIAUTOHANDLE_H_
#define OCIAUTOHANDLE_H_

#include "Dbg.h"
#include "occi.h"

//---- occi_auto_handle -----------------------------------------------------------
//! <B>wrapper template class around OCCI object requiring special destruction</B>
/*!
 Simplifies memory handling on allocated OCCI objects like Connection, Statement and so on.
 Frees the internal handle if allocated using the specified destruction function. It almost works like
 a std::auto_ptr in behavior of assignment, construction etc.
 \note Ownership of the pointer is not shared (no extended reference counting), it gets always passed when assigning.
 */
template<typename objectType, typename parentType>
struct occi_auto_handle {
	typedef objectType *obj_ptr_type;
	typedef parentType *parent_ptr_type;
	typedef void (parentType::*terminateFunc)( obj_ptr_type );

	obj_ptr_type fpObj;
	parent_ptr_type fpParent;
	terminateFunc fpTermFunc;

	occi_auto_handle() :
		fpObj( 0 ), fpParent( 0 ), fpTermFunc( 0 ) {
	}

	occi_auto_handle( obj_ptr_type pObj, parent_ptr_type pParent, terminateFunc pFunc ) :
		fpObj( pObj ), fpParent( pParent ), fpTermFunc( pFunc ) {
		StatTrace(occi_auto_handle.occi_auto_handle, "constructing, fpObj: " << (long)fpObj << " parent: " << (long)fpParent << " deleteFunc: " << (long)&fpTermFunc, Storage::Current());
	}
	~occi_auto_handle() {
		StatTrace(occi_auto_handle.~occi_auto_handle, "destructing, fpObj: " << (long)fpObj << " parent: " << (long)fpParent << " deleteFunc: " << (long)&fpTermFunc, Storage::Current());
		doDelete();
	}
	occi_auto_handle &operator=( occi_auto_handle &rhs ) throw () {
		reset( rhs.release() );
		fpParent = rhs.fpParent;
		fpTermFunc = rhs.fpTermFunc;
		StatTrace(occi_auto_handle.operator = , "assignment, fpObj: " << (long)fpObj << " parent: " << (long)fpParent << " deleteFunc: " << (long)&fpTermFunc, Storage::Current());
		return *this;
	}
	obj_ptr_type release() {
		obj_ptr_type p( fpObj );
		fpObj = 0;
		return p;
	}
	void reset( obj_ptr_type ptr = 0 ) throw () {
		if ( fpObj && fpObj != ptr ) {
			doDelete();
		}
		fpObj = ptr;
	}
	void doDelete() throw () {
		StatTrace(occi_auto_handle.doDelete, "releasing object fpObj: " << (long)fpObj << " parent: " << (long)fpParent << " deleteFunc: " << (long)&fpTermFunc, Storage::Current());
		if ( fpObj ) {
			if ( fpParent ) {
				( fpParent->*fpTermFunc )( release() );
				StatTrace(occi_auto_handle.doDelete, "object released", Storage::Current());
			}
		}
	}
	obj_ptr_type get() const {
		return fpObj;
	}
	obj_ptr_type operator->() const throw () {
		return fpObj;
	}
	operator const bool() const {
		return fpObj != 0;
	}
private:
	occi_auto_handle( occi_auto_handle &rhs );
};

typedef occi_auto_handle<oracle::occi::Connection, oracle::occi::Environment> ConnectionPtrType;
typedef occi_auto_handle<oracle::occi::Statement, oracle::occi::Connection> StatementPtrType;
typedef occi_auto_handle<oracle::occi::ResultSet, oracle::occi::Statement> ResultSetPtrType;

#endif /* OCIAUTOHANDLE_H_ */
