#!/bin/ksh
clienthost="dylan.hsr.ch"
cp /tmp/mkca/ca-hierarchy/ucerts/clientcrt_${clienthost}.pem clientcrt.pem
cp /tmp/mkca/ca-hierarchy/ucerts/clientkey_${clienthost}.pem clientkey.pem
cp /tmp/mkca/ca-hierarchy/ucerts/rootCA1crt.pem .

