#!/bin/ksh
# Use this script to install new generated certificates.
cp /tmp/mkca/ca-hierarchy/ucerts/* .
clienthost=dylan.hsr.ch
serverhost=dylan.hsr.ch
cp clientcrt_${clienthost}.pem clientcrt.pem
cp clientkey_${clienthost}.pem clientkey.pem
cp servercrt_${serverhost}.pem servercrt.pem
cp serverkey_${serverhost}.pem serverkey.pem
cp fullchain_${serverhost}.pem fullchain.pem
cp serverchain_${serverhost}.pem serverchain.pem

