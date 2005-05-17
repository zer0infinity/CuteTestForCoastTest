/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _QueueWorkingModule_H
#define _QueueWorkingModule_H

//---- WDModule include -------------------------------------------------
#include "config_Queueing.h"
#include "WDModule.h"
#include "Context.h"
#include "Threads.h"

class Queue;

//---- QueueWorkingModule ----------------------------------------------------------
//! <B>Queue based module for message passing systems</B>
/*!
<B>Configuration:</B><PRE>
{
	/QueueSize		long		mandatory, size of the queue
	...
}</PRE>
*/
class EXPORTDECL_QUEUEING QueueWorkingModule : public WDModule
{
	friend class QueueWorkingModuleTest;
	friend class ValueOutputtingModuleTest;
	friend class CalculationsModuleTest;
public:
	//--- constructors
	QueueWorkingModule(const char *name);
	~QueueWorkingModule();

	//! implementers should initialize module using config
	virtual bool Init(const Anything &config);
	//! implementers should terminate module expecting destruction
	virtual bool Finis();
	//! initializes module after termination for reinitialization; default uses Init; check if this applies
	virtual bool ResetInit(const Anything &config);
	//! terminates module for reinitialization; default uses Finis; check if this applies
	virtual bool ResetFinis(const Anything &config);

	inline bool IsAlive() {
		return fAlive == 0xf007f007;
	}

	/*! main accessor functions to work with the queue */
	bool PutElement(Anything &anyELement, bool bTryLock = false);
	bool GetElement(Anything &anyValues);
	void PutBackElement(Anything &anyValues);

	/* exclusively consume all Elements from queue, threads which are blocked on the queue to get an element will be woken up because of the released semaphore. But instead of getting an Element it will get nothing back and should be able to handle this correctly.
		\param anyELements Anything to hold the elements removed from the queue
		\return number of elements removed from the queue */
	long FlushQueue(Anything &anyElements);

	bool GetQueueStatistics(Anything &anyStat);

	void LogError(String strMessage);
	void LogWarning(String strMessage);
	void LogInfo(String strMessage);

protected:
	ROAnything GetNamedConfig(const char *name);
	ROAnything GetConfig();
	Context *GetContext() {
		return fpContext;
	};
	void IntCleanupQueue();
	void SetDead() {
		fAlive = 0x00dead00;
	};
	void Log(String strMessage, const char *channel);
	void Log(Anything &anyStatus, const char *channel);

private:
	void IntInitQueue(ROAnything roaConfig);

	Anything	fConfig;
	Queue		*fpQueue;
	Context		*fpContext;
	Mutex		fContextLock;
	Anything	fFailedPutbackMessages;
	String		fErrorLogName, fWarningLogName, fInfoLogName;
	u_long		fAlive;
};

#endif
