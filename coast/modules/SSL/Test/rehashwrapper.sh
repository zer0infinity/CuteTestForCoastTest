#!/bin/ksh
# Wrapper around c_rehash.
if [ $# -ne 1 ]
then
	echo "$0 <directory to rehash>"
	exit 1
fi
OPENSSL_CFG_PATH=$DEV_HOME/coast/modules/SSL/Test
conf=$OPENSSL_CFG_PATH/openssl.cnf
if [ ! -d "$DEV_HOME/3rdparty/installed/openssl/bin/${OSREL}" ]
then
      	curOS=${OSREL%_*};
        lastREL="";
        for name in $(ls $DEV_HOME/3rdparty/installed/openssl/bin); do
                if [ -d "$DEV_HOME/3rdparty/installed/openssl/bin/$name" ]; then
                        if [ "${name%_*}" == "${curOS}" ]; then
                                if [[ "${OSREL}" < "${name}" ]]; then
                                        break;
                                else
                                        lastREL=$name;
                                fi;
                        fi;
                fi;
        done;
	if [ "${lastREL}" != "" ]
	then
		echo "No openssl binary for $OSREL found. Using $lastREL instead.";
	else
		echo "Could not downgrade to another installed openssl binary!";
		exit 1
	fi
	OSREL=$lastREL;		# safe, since we don't export the variable!
fi

OPENSSL_PATH=$DEV_HOME/3rdparty/installed/openssl/bin/$OSREL
openssl_bin=${OPENSSL_PATH}/openssl

if [ -z $OPENSSL_PATH ]
then
	echo $OPENSSL_PATH
	echo "Could not find $OPENSSL_PATH. Exiting. Set OPENSSL_PATH variable in $0"
	exit 1
else
   	echo "Using openssl path $OPENSSL_PATH...."
fi

if [ -x $openssl_bin -a ! -d $openssl_bin ]
then
	echo "Using openssl binary $openssl_bin"
else
	echo "Could not find openssl binary. Exiting. Set openssl_bin variable in $0)"
	exit 1
fi
export OPENSSL=$openssl_bin
./c_rehash $1
