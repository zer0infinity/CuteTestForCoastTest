* Every year testcases will fail because the certificate expires.
  run mkca.sh -d /tmp/mkca -D 365 -S zuoz.hsr.ch -S bondo -C bondo
 
  This will create the certificates you need for  SSL and StdDataAccess tests
  as well as Frontdoor Tests.
 
  The -S option gives the common names used for SERVER certificates.
  The -C option gives the common names used for CLIENT certificates.
 
  You must follow the guidelines given in mkca.sh (at the end of the script)
  and place the generated certificates into your SSL enabled Apache which
  is used as backend server for the tests.
 
  mkca.sh gives you details which certificates you have to copy to your_cert
  directory.
