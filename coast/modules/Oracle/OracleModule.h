#ifndef _OracleModule_H
#define _OracleModule_H

//---- WDModule include -------------------------------------------------
#include "config_coastoracle.h"
#include "WDModule.h"

//---- OracleModule ----------------------------------------------------------
//: comment action
//	Structure of config:
//<PRE>	{
//		/SybaseServerName1 {
//			/ParallelQueries	long	defines number of parallel sql queries which can be issued, default 5
//		}
//      ...
//	}</PRE>
class EXPORTDECL_COASTORACLE OracleModule : public WDModule
{
public:
	//--- constructors
	OracleModule(const char *name);
	~OracleModule();

	//:implementers should initialize module using config
	virtual bool Init(const ROAnything config);
	//:implementers should terminate module expecting destruction
	virtual bool Finis();

//    //:initializes module after termination for reinitialization; default uses Init; check if this applies
//    virtual bool ResetInit(const ROAnything config);
//    //:terminates module for reinitialization; default uses Finis; check if this applies
//    virtual bool ResetFinis(const ROAnything config);

};

#endif
