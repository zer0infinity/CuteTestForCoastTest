#!/bin/ksh
clienthost="dylan.hank.com"
cp /tmp/mkca/ca-hierarchy/ucerts/clientcrt_${clienthost}.pem clientcrt.pem
cp /tmp/mkca/ca-hierarchy/ucerts/clientkey_${clienthost}.pem clientkey.pem
cp /tmp/mkca/ca-hierarchy/ucerts/rootCA1crt.pem .
/* empty file */
