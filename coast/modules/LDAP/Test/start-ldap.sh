#!/bin/ksh

mypath=`pwd`
NETSITE_ROOT=/db/wdtest
export NETSITE_ROOT
dbreldir=slapd-DailyBuildTests
importbinpath=bin/slapd/server
importbin=ns-slapd

function WaitOnStart
{
	# first wait some secs to let it start
	local loop_counter=1;
	local max_count=20;
	local PID=0;
	printf "waiting on slapd process to startup ";
	while [ $loop_counter -le $max_count ]; do
		loop_counter=`expr $loop_counter + 1`;
		if [ -f "$NETSITE_ROOT/$dbreldir/logs/pid" ]; then
			PID=`cat $NETSITE_ROOT/$dbreldir/logs/pid`
		fi
		if [ $PID -ne 0 ]; then
			ps -ef | grep -c "${PID}.*${importbin}.*${dbreldir}" >/dev/null
			if [ $? -eq 0 ]; then
				printf " %s\n" "done"
				return 0
			fi
		fi
		printf "."
		sleep 2
	done
	return 1;
}

startretcode=44;

# start server
cd $NETSITE_ROOT/$importbinpath; ./$importbin -f $NETSITE_ROOT/$dbreldir/config/slapd.conf -i $NETSITE_ROOT/$dbreldir/logs/pid "$@" >/dev/null 2>&1 </dev/null &

# wait until process is running
WaitOnStart

if [ $? -eq 0 ]; then
	startretcode=0;
	echo 'OK';
else
	startretcode=$?;
	echo 'ERROR: starting of ['$dbreldir'] failed!';
fi

exit $startretcode;
