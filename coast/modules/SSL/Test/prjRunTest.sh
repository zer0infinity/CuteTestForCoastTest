#!/bin/ksh

## add test specific things before the call to callTest
function prepareTest
{
	echo "Setting up certificate cache...."
	certdir=ifs_cert
	# Setting up certificate directory for tests
	mkdir $PRJCONFIGPATH/cert_files  > /dev/null 2>&1
	rm -f $PRJCONFIGPATH/cert_files/*
	cp $PRJCONFIGPATH/$certdir/*.pem $PRJCONFIGPATH/cert_files
	# Setting up hashed certificate directory for tests
	mkdir $PRJCONFIGPATH/cert_hashes > /dev/null 2>&1
	rm -f $PRJCONFIGPATH/cert_hashes/*
	cp $PRJCONFIGPATH/$certdir/*.pem $PRJCONFIGPATH/cert_hashes
	$PROJECTDIR/rehashwrapper.sh $PRJCONFIGPATH/cert_hashes
	# Linking hashed certificate directory to one common place
	rm /tmp/SSLSocketTestHashedCerts > /dev/null 2>&1
	ln -s $PRJCONFIGPATH/cert_hashes /tmp/SSLSocketTestHashedCerts
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
	echo
}
