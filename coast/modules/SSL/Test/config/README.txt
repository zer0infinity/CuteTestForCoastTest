* Every year testcases will fail because the certificate expires.
  Adjust the organization, the organization unit and the dn 
  the certificate certifies (normally your hostname) by searching for
  below lines in mkca.sh and replacing them:
	Kim Dojo
	Test
	atlantis.hsr.loc

  Run mkca.sh -D 365 -d /tmp/myca

  The dot in below lines points  to the 
  modules/SSL/Test/config directory, (where this README.txt
    is located)

  cp /tmp/myca/ca-hierarchy/ucerts/* . 
  cp /tmp/myca/ca-hierarchy/client/client.key . 

  Since the tests connect to a backend server which is an SSL-enabled
  apache you will have to place the following files in the apaches
  ./conf directory: 

  Into ./conf/ssl.crt:
  fullchain.pem, rename to fullchain.crt
  servercrt.pem, rename to servercrt.crt
  Because make rehash only recognices .crt extensions, you must rename the files
  from .pem to .crt.
  Run make rehash  to rehash the links to the .crt files.
  
  Into ./conf/ssl.key
  serverkey.pem


The server's configuration should be something like:

SSLCertificateFile /usr/local/apache-2.0.48/conf/ssl.crt/servercrt.pem
SSLCertificateKeyFile /usr/local/apache-2.0.48/conf/ssl.key/serverkey.pem
SSLCertificateChainFile /usr/local/apache-2.0.48/conf/ssl.crt/fullchain.pem

You will find more info in the mkca.sh script.

If you omit the SSLCertificateChainFile you should know what you are doing,
because you MUST establish a fully trusted path between the rootCA1.pem
and the servercert, as it says in mkca.sh:

echo "Creating Full Cain file (Server/subCA1/rootCA1)"
cp  servercrt.pem     fullchain.pem
cat subCA2.pem     >> fullchain.pem
cat rootCA1.pem    >> fullchain.pem

