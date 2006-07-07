/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _TraceLocks_H
#define _TraceLocks_H

//! Helper macros to trace aquiring and release of locks
//!  Since the normal trace options alter the behaviour of locking conditions
//!  these macros allow you to write output to syslog while running the optimized
//!  code. Before the usage of the macros in a method you have to invoke TRACE_LOCK_START(methodname)
//!  set #define TRACE_LOCKS to enable the macros.
//!  Sample may be found in SessionListManager.cpp
//!  To examine the lock parity, enable syslogging with WD_DOLOG=2 and use the following script
//!  (Done in K-shell, of course)

//  ---- snipp
/*
#!/bin/ksh
cut -b 44- /var/adm/frontdoor > /tmp/$$tmp1
grep "^  locked"   /tmp/$$tmp1   | sort -u  > /tmp/locked
grep "^unlocked"   /tmp/$$tmp1   | sort -u  > /tmp/unlocked
rm /tmp/locked1
rm /tmp/unlocked1
cat /tmp/locked   | awk '{print $2}'  > /tmp/locked1
cat /tmp/unlocked | awk '{print $2}'  > /tmp/unlocked1
diff  /tmp/locked1 /tmp/unlocked1
if [ $? -eq 0 ]
then
        echo "Aquiring/releasing lock parity"
else
  echo "Problem detected. Further analysis is:"
  cat /tmp/locked   | awk '{print $2 " " $3 " " $4}'  > /tmp/locked1
  cat /tmp/unlocked | awk '{print $2 " " $3 " " $4}'  > /tmp/unlocked1
  diff /tmp/locked1 /tmp/unlocked1
  rm /tmp/$$tmp1
  #cat /tmp/locked1
  #cat /tmp/unlocked1
fi
*/
//  ---- end snipp

// Enable syslogging for WD:

// --- snipp /etc/syslog.conf
/*
*.err;kern.notice;auth.notice			/dev/console
*.err;kern.debug;daemon.notice;mail.crit	/var/adm/messages

*.alert;kern.err;daemon.err			operator
*.alert						root

*.emerg						*

# enable wd logging
user.err					/var/adm/frontdoor
user.info					/var/adm/frontdoor
user.debug					/var/adm/frontdoor
user.notice					/var/adm/frontdoor
user.warning				/var/adm/frontdoor
user.alert					/var/adm/frontdoor

# if a non-loghost machine chooses to have authentication messages
# sent to the loghost machine, un-comment out the following line:
#auth.notice			ifdef(`LOGHOST', /var/log/authlog, @loghost)

mail.debug			ifdef(`LOGHOST', /var/log/syslog, @loghost)

#
# non-loghost machines will use the following lines to cause "user"
# log messages to be logged locally.
#
ifdef(`LOGHOST', ,
user.err					/dev/console
user.err					/var/adm/messages
user.alert					`root, operator'
user.emerg					*
)
*/
// --- end snipp
#if defined TRACE_LOCKS

#define TRACE_LOCK_START(methodname) 	String lmsgf; lmsgf << time(0) << " " << Thread::MyId() << " " << (methodname); \
								  String lname; String lmsgw;	String lmsg
#define TRACE_LOCK_ACQUIRE(lockname)   lname = " "; lname << (lockname);  lmsgw = "  locked "; lmsg = ""; \
								  lmsg << lmsgw << lmsgf << lname; SysLog::Info(lmsg)
#define TRACE_LOCK_RELEASE(lockname)   lname = " "; lname << (lockname);  lmsgw = "unlocked "; lmsg = ""; \
								  lmsg << lmsgw << lmsgf << lname; SysLog::Info(lmsg)
#else
#define TRACE_LOCK_START(methodname)
#define TRACE_LOCK_ACQUIRE(lockname)
#define TRACE_LOCK_RELEASE(lockname)
#endif

#endif
