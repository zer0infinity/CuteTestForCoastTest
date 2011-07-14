/*
 * Copyright (c) 2007, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _Threads_IPP
#define _Threads_IPP

template<class WorkerParamType>
bool Thread::SetWorking(WorkerParamType workerArgs) {
	StatTrace(Thread.SetWorking, "ThrdId: " << GetId() << " CallId: " << MyId(), Coast::Storage::Current());
	return SetRunningState(eWorking, workerArgs);
}

template<class WorkerParamType>
bool Thread::SetRunningState(ERunningState state, WorkerParamType args) {
	LockUnlockEntry me(fStateMutex);

	// allocate things used before and after call to CallRunningStateHooks() on Coast::Storage::Global() because Allocator could be refreshed during call

	StatTrace(Thread.SetRunningState, "-- entering -- CallId: " << MyId(), Coast::Storage::Current());

	if (fState != eRunning || fRunningState == state) {
		return false;
	}

	ERunningState oldState = fRunningState;
	fRunningState = state;
	// after this call we potentially have refreshed Allocator
	CallRunningStateHooks(state, args);
	{
		// scoping to force compiler not to move around automatic variables which could make strange things happen
		Anything anyEvt;
		anyEvt["ThreadId"] = GetId();
		anyEvt["RunningState"]["Old"] = (long) oldState;
		anyEvt["RunningState"]["New"] = (long) state;
		if (!args.IsNull()) {
			anyEvt["Args"] = args.DeepClone();
		}
		BroadCastEvent(anyEvt);
	}
	StatTrace(Thread.SetRunningState, "-- leaving --", Coast::Storage::Current());
	return true;
}

#endif
