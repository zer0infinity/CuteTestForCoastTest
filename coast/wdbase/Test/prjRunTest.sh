#!/bin/ksh

export WD_PATH=config:.

## add test specific things before the call to callTest
function prepareTest
{
	export WD_DOLOG=1
	export WDTESTPIDFILE=config/wdbasetest.pid
}

## call to wdtest or whatever you want to call
function callTest
{
	${TEST_EXE} $cfg_testparams
	# remember return code of wdtest, signals number of failures
	return $?;
}

## add test specific things after the call to callTest
# remove generated files ...
function cleanupTest
{
	if  [ $ret_code -eq 0 ]; then
		echo $cfg_testparams |egrep "all|AppBooterTest" >/dev/null;
		if [ $? -eq 0 -a -f "$WDTESTPIDFILE" ]; then
			WDTESTPID=$(cat $WDTESTPIDFILE);
			case "${CURSYSTEM}" in
				SunOS)
					SYSLOGFILE=/var/adm/messages;
					SYSLOGMSG='((Coast)|(wdbasetest))'"\[$WDTESTPID*\]:.*libfake: ld\.so\.1: ${TEST_EXE}: fatal: libfake\.so: open failed: No such file or directory"
					egrep "$SYSLOGMSG" $SYSLOGFILE >/dev/null;
					RESULT=$?
					rm $WDTESTPIDFILE;
					if [ $RESULT -eq 0 ]; then
						return $RESULT;
					fi
					echo "prjRunTest.sh:38 Failure <" $SYSLOGMSG "> not found in" $SYSLOGFILE
					ret_code=1;
				;;
				Linux)
					SYSLOGFILE=/var/log/messages;
					SYSLOGMSG="libfake: libfake\.so: cannot open shared object file: No such file or directory"
					# is not testable on Linux right now - needs root permissions
				;;
			esac
		fi
	fi
}
