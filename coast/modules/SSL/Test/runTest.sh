#!/bin/sh

TESTPATH=`pwd`
CONFIGPATH=$TESTPATH/config
CERTDIR=$CONFIGPATH/coast_cert

echo "Setting up certificate cache...."
# Setting up certificate directory for tests
mkdir $CONFIGPATH/cert_files  > /dev/null 2>&1
rm -f $CONFIGPATH/cert_files/*
cp $CERTDIR/*.pem $CONFIGPATH/cert_files
# Setting up hashed certificate directory for tests
mkdir $CONFIGPATH/cert_hashes > /dev/null 2>&1
rm -f $CONFIGPATH/cert_hashes/*
cp $CERTDIR/*.pem $CONFIGPATH/cert_hashes
c_rehash $CONFIGPATH/cert_hashes
# Linking hashed certificate directory to one common place
rm /tmp/SSLSocketTestHashedCerts > /dev/null 2>&1
ln -s $CONFIGPATH/cert_hashes /tmp/SSLSocketTestHashedCerts
