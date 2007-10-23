#!/bin/ksh
#
# BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO WARRANTY
# FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW.  EXCEPT WHEN
# OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES
# PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED
# OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS
# TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE
# PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING,
# REPAIR OR CORRECTION.
#
# IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING
# WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR
# REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES,
# INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING
# OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED
# TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY
# YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER
# PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGES.
#
# This script creates a RootCA1 root certification authority.
# Within RootCA1 exists the subCA1 sub certification authority.
# subCA1 finally issues a certificate for a server.
# This script is mimiks CA.pl based upon the underlying ${openssl_bin}
# cmd line tool.
# Besides the generation of a RootCA->SubCA->ServerCert hierarchy
# it can be used to create signed server certifiates with arbitrary duration.
# Just change the common name in the script and use the resulting server*.pem
# files.
# Since the sub-CA creates the cert, you must specify the
# SSLCACertificateChainFile and give the intermediate cert as value
# (Apache).
# The files you want to use are in target_dir/ca-hierarchy/ucerts
#
# Change History
# 09.Nov 02  Fixed recursively delete of directory given with -d
#            within this dir ca-hierarchy will be created and deleted
#	     on every run
#
# May    03 Added client cert generation, made script independent of openssl.cnf
# Adjust path to openssl.cnf file (OPENSSL_PATH) and to openssl executable (openssl_bin)
OPENSSL_CFG_PATH=$DEV_HOME/coast/modules/SSL/Test
conf=$OPENSSL_CFG_PATH/openssl.cnf
country="CH"
state="ZH"
locality="ZH"
org="Test"
orgunit="CertIssueing"
emailsuffix="wildplanet.com"
commonnameclient=""
commonnameserver=""
pathtoexistingcacert=""
typeset -l clientemail="${commonnameclient}@${emailsuffix}"
ret=0

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
if [ ! -f $conf ]
then
    echo "Could not find $conf. Exiting. Place in $OPENSSL_CFG_PATH in $0"
    exit
fi

function showhelp
{
    echo "mkca.sh -d <Directory to place CA-Structure>"
    echo "        creates ca-hierarchy within this dir"
    echo "        creates directories if not present."
    echo "        -D<default is 3650>"
    echo "        -c<copy the created certs to given location>"
    echo "        -E<location where previously built rootCA1 and subCA1 certs are kept>"
    echo "           This allows you to add additional client/server certs without re-generating previously"
    echo "           generated certificates."
    echo "        -C common name (subject) of client, entity which is checked in cert verify (-C may be given repeatedly)"
    echo "        -S common name (subject) of server, entity which is checked in cert verify (-S may be given repeatedly)"
    echo ""
    echo "The files you want to use are in the directory"
    echo "given with -d in ca-hierarchy/ucerts"
    exit 0
}

function errorexit
{
	echo "!!!!!!!!!!!!!!!!!! CERTIFICATE VERIFICATION FAILURE!!!!!!!!!!!!!!!"
	exit 999
}

days=3650
copyto=""
while getopts "d:D:c:C::e:E:S:" opt
do
        case $opt in
                d)      target_dir="${OPTARG}";
                ;;
                D)      days="${OPTARG}";
                ;;
                c)      copyto="${OPTARG}";
                ;;
                C)      commonnameclient="${commonnameclient} ${OPTARG}";
                ;;
                S)      commonnameserver="${commonnameserver} ${OPTARG}";
                ;;
                e)      emailclient="${OPTARG}";
                ;;
                E)      pathtoexistingcacert="${OPTARG}";
                ;;
                \?)
                        showhelp;
                ;;
        esac
done

shift $(($OPTIND - 1))

if [ -z ${commonnameclient} ]
then
	commonnameclient="IAMACLIENT";
fi
if [ -z ${commonnameserver} ]
then
	commonnameserver="IAMASERVER";
fi

if [  -z ${target_dir} ]
then
        showhelp;
fi

if [ ! -d ${target_dir} ]
then
   mkdir ${target_dir}
   if [ $? -ne 0 ]
   then
        echo "Can't create ${target_dir}"
        exit 1
   fi
fi

target_dir=$target_dir/ca-hierarchy

if [  -d ${target_dir} ]
then
        rm -Rf ${target_dir}/*
else
	mkdir ${target_dir}
	if [ $? -ne 0 ]
	then
           echo "Can't create ${target_dir}"
           exit 1
	fi
fi

mkdir ${target_dir}/ucerts
cd ${target_dir}

DAYS="-days $days"
echo "Creating RootCA1"

CATOP=./demoCA
CAKEY=./cakey.pem
CACERT=./cacert.pem

mkdir ${CATOP}
mkdir ${CATOP}/certs
mkdir ${CATOP}/crl
mkdir ${CATOP}/newcerts
mkdir ${CATOP}/private
echo "01" > ${CATOP}/serial
touch ${CATOP}/index.txt

if [ -z ${pathtoexistingcacert} ]
then
${openssl_bin} req -config $conf -new -x509 -passout pass:hank -keyout ${CATOP}/private/$CAKEY \
                           -out ${CATOP}/$CACERT $DAYS <<EOF
${country}
${state}
${locality}
RootCA1
${orgunit}
RootCA1
rootca1@${emailsuffix}

EOF
if [ -z ${pathtoexistingcacert} ]
then
echo "Creating a PKCS#12 encoded copy of rootCA1.pem for \
Netcape use. Password is rootca1"
${openssl_bin} pkcs12 -passin pass:hank  -passout pass:rootca1 -export \
	 -in ${CATOP}/$CACERT   \
         -inkey ${CATOP}/private/$CAKEY -out ${CATOP}/cacert.p12
fi
echo "Stripping passphrase from rootCA1key.pem"
${openssl_bin} rsa -passin pass:hank -in ${CATOP}/private/$CAKEY -out ${target_dir}/ucerts/rootCA1key.pem

echo "Certificate Request for SubCA1"
${openssl_bin} req -config $conf -new  -passout pass:gugus -keyout newreq.pem \
                           -out newreq.pem <<EOF1
${country}
${state}
${locality}
SubCA1
${orgunit}
SubCA1
subca1@${emailsuffix}
.
.
EOF1

echo "Signing CA request for SubCA1"
${openssl_bin} ca -config $conf -policy policy_anything -passin pass:hank -out newcert.pem \
          -extensions v3_ca $DAYS -infiles newreq.pem  <<EOF2
y
y
EOF2
else
	cp ${pathtoexistingcacert}/rootCA1key.pem ${CATOP}/private/$CAKEY
	cp ${pathtoexistingcacert}/rootCA1crt.pem ${CATOP}/$CACERT
	if [ ! -f ${CATOP}/private/$CAKEY ]
	then
		echo "Unable to retrieve rootCA1key.pem"
		exit 1
	fi
	if [ ! -f ${CATOP}/$CACERT ]
	then
		echo "Unable to retrieve rootCA1crt.pem"
		exit 1
	fi
	chmod 744 ${CATOP}/private/$CAKEY
	chmod 744 ${CATOP}/$CACERT

fi
echo "Creating SubCA1"

mkdir ./subCA1
cd ./subCA1

mkdir ${CATOP}
mkdir ${CATOP}/certs
mkdir ${CATOP}/crl
mkdir ${CATOP}/newcerts
mkdir ${CATOP}/private
echo "01" > ${CATOP}/serial
touch ${CATOP}/index.txt

if [ -z ${pathtoexistingcacert} ]
then
	echo "Stripping Certificate and private Key from Certificate for SubCA1"
	csplit -f work -s -n 2  ../newreq.pem '/-----BEGIN CERTIFICATE REQUEST-----/'
	mv work00 ${CATOP}/private/cakey.pem
	rm work01
	echo "Stripping passphrase from subCA1key.pem"
	${openssl_bin} rsa -passin pass:gugus -in ${CATOP}/private/$CAKEY -out ${target_dir}/ucerts/subCA1key.pem
	cp ${CATOP}/private/cakey.pem ${target_dir}/ucerts/subCA1key.pem
	echo "Stripping plaintext from SubCA1 Certificate"
	${openssl_bin} x509  -in ../newcert.pem -out ${CATOP}/cacert.pem
else
	cp ${pathtoexistingcacert}/subCA1key.pem ${CATOP}/private/$CAKEY
	cp ${pathtoexistingcacert}/subCA1crt.pem ${CATOP}/cacert.pem
	if [ ! -f ${CATOP}/private/$CAKEY ]
	then
		echo "Unable to retrieve subCA1key.pem"
		exit 1
	fi
	if [ ! -f ${CATOP}/$CACERT ]
	then
		echo "Unable to retrieve subCA1crt.pem"
		exit 1
	fi
	chmod 744 ${CATOP}/private/$CAKEY
	chmod 744 ${CATOP}/cacert.pem
fi

for commonnameserver_var in ${commonnameserver}
do
	echo "Creating certificate request for Server ${commonnameserver_var}"
	${openssl_bin} req -config $conf -new  -passout pass:gaga -keyout newreq_${commonnameserver_var}.pem \
                          -out newreq_${commonnameserver_var}.pem <<EOF1
${country}
${state}
${locality}
ServerSite
${orgunit}
${commonnameserver_var}
${commonnameserver_var}@${emailsuffix}
.
.
EOF1

	echo "Signing CA request for Server ${commonnameserver_var}"
	${openssl_bin} ca -config $conf -policy policy_anything \
	-passin pass:gugus $DAYS -out newcert_${commonnameserver_var}.pem \
	-infiles newreq_${commonnameserver_var}.pem <<EOF2
y
y
EOF2

	echo "Stripping signed Certificate for Server ${commonnameserver_var}"
	${openssl_bin} x509  -in newcert_${commonnameserver_var}.pem -out xx_${commonnameserver_var}.pem
	mv xx_${commonnameserver_var}.pem newcert_${commonnameserver_var}.pem
	${openssl_bin} rsa  -passin pass:gaga -in newreq_${commonnameserver_var}.pem -out newreq_${commonnameserver_var}.plain.pem
done

cd ${target_dir}
mkdir client

for commonnameclient_var in ${commonnameclient}
do
	echo "Creating client certificate for ${commonnameclient_var}"
	cd ${target_dir}
	cd client
	${openssl_bin} genrsa -out client_${commonnameclient_var}.key 1024

	echo "Creating certificate request for Client ${commonnameclient_var}"
	${openssl_bin} req -config $conf -new  -passout pass:gaga -key client_${commonnameclient_var}.key \
                           -out client_${commonnameclient_var}.csr <<EOF1
${country}
${state}
${locality}
ClientSite
${orgunit}
${commonnameclient_var}
${commonnameclient_var}@${emailsuffix}
.
.
EOF1

	echo "Signing Client certificte for ${commonnameclient_var} by subCA1"
	cd ${target_dir}/subCA1
	${openssl_bin} x509 -req $DAYS -passin pass:gugus  -CA ${CATOP}/cacert.pem \
			  -CAkey ${CATOP}/private/cakey.pem \
			  -CAcreateserial -in ${target_dir}/client/client_${commonnameclient_var}.csr -out ${target_dir}/client/clientcrt_${commonnameclient_var}.pem

	echo "Creating a PKCS#12 encoded copy of client.crt for Netcape use. Password is client"
	${openssl_bin} pkcs12 -export -passin pass:gugus -passout pass:client -clcerts \
	-in ${target_dir}/client/clientcrt_${commonnameclient_var}.pem  \
     	-inkey ${target_dir}/client/client_${commonnameclient_var}.key      -out ${target_dir}/client/clientcrt_${commonnameclient_var}.p12
done

echo "Building result directory  ${target_dir}/ucerts"
cp  ${CATOP}/cacert.pem ${target_dir}/ucerts/subCA1crt.pem
for commonnameserver_var in ${commonnameserver}
do
	cp  ./newcert_${commonnameserver_var}.pem       ${target_dir}/ucerts/servercrt_${commonnameserver_var}.pem
	cp  ./newreq_${commonnameserver_var}.plain.pem  ${target_dir}/ucerts/serverkey_${commonnameserver_var}.pem
done
cd ..
cp ${CATOP}/cacert.pem ${target_dir}/ucerts/rootCA1crt.pem
if [ -z ${pathtoexistingcacert} ]
then
cp ${CATOP}/cacert.p12 ${target_dir}/ucerts/rootCA1.p12
fi
cd client

for commonnameclient_var in ${commonnameclient}
do
	cp  clientcrt_${commonnameclient_var}.p12      ${target_dir}/ucerts/clientcrt_${commonnameclient_var}.p12
	cp  clientcrt_${commonnameclient_var}.pem	   ${target_dir}/ucerts/clientcrt_${commonnameclient_var}.pem
	cp  client_${commonnameclient_var}.key		   ${target_dir}/ucerts/clientkey_${commonnameclient_var}.pem
done

cd ${target_dir}/ucerts
echo "Creating Chain without Server Cert)"
cp  subCA1crt.pem     chainwoservercrt.pem
cat rootCA1crt.pem >> chainwoservercrt.pem
for commonnameserver_var in ${commonnameserver}
do
	echo "Creating Full Cain file (Server/subCA1/rootCA1) for ${commonnameserver_var}"
	cp  servercrt_${commonnameserver_var}.pem     fullchain_${commonnameserver_var}.pem
	cat subCA1crt.pem     >> fullchain_${commonnameserver_var}.pem
	cat rootCA1crt.pem    >> fullchain_${commonnameserver_var}.pem
done

for commonnameserver_var in ${commonnameserver}
do
	echo "Creating Chain without the CA the client trusts (without rootCA1) for ${commonnameserver_var}"
	echo "You might use this in Frontdoor"
	cp  servercrt_${commonnameserver_var}.pem     serverchain_${commonnameserver_var}.pem
	cat subCA1crt.pem     >> serverchain_${commonnameserver_var}.pem
done
echo "Your certificates are in ${target_dir}/ucerts"
if [ ! -z ${copyto} ]
then
	if [ ! -d ${copyto} ]
	then
		mkdir ${copyto}
	fi
	echo "Copying SERVER related files: serverkey servercrt intermediate.crt  to ${copyto}:"
	for commonnameserver_var in ${commonnameserver}
	do
		cp servercrt_${commonnameserver_var}.pem          ${copyto}
		cp serverkey_${commonnameserver_var}.pem          ${copyto}
		cp chainwoservercrt.pem   ${copyto}
		cp fullchain_${commonnameserver_var}.pem          ${copyto}
		cp serverchain_${commonnameserver_var}.pem    	  ${copyto}
	done
	echo "Copying CLIENT related files: clientcrt.pem client.key clientcrt.p12 to ${copyto}:"
	for commonnameclient_var in ${commonnameclient}
	do
	cd ${target_dir}/client
		cp clientcrt_${commonnameclient_var}.pem	 ${copyto}
		cp clientkey_${commonnameclient_var}.pem	 ${copyto}
		cp clientcrt_${commonnameclient_var}.p12	 ${copyto}
	cd ${target_dir}/ucerts
	done
fi
for commonnameserver_var in ${commonnameserver}
do
	echo "Verifying cert chain with -untrusted option for ${commonnameserver_var}"
	${openssl_bin} verify -verbose  -purpose sslserver \
	-CAfile rootCA1crt.pem -untrusted subCA1crt.pem servercrt_${commonnameserver_var}.pem
	if [ $? -ne 0 ]
	then
		errorexit;
	fi;
	echo "Verifying cert chain without server cert (chainwoservercrt.pem) for ${commonnameserver_var}"
	${openssl_bin} verify -verbose  -purpose sslserver \
	-CAfile chainwoservercrt.pem servercrt_${commonnameserver_var}.pem
	if [ $? -ne 0 ]
	then
		errorexit;
	fi;
	echo "Verifying cert chain with full chain  (fullchain.pem)"
	${openssl_bin} verify -verbose  -purpose sslserver \
	-CAfile fullchain_${commonnameserver_var}.pem servercrt_${commonnameserver_var}.pem
	if [ $? -ne 0 ]
	then
		errorexit;
	fi;
	for commonnameclient_var in ${commonnameclient}
	do
		echo "Verifying cert chain with full chain and clientcrt.pem for ${commonnameclient_var}"
		${openssl_bin} verify -verbose  -purpose sslclient \
		-CAfile fullchain_${commonnameserver_var}.pem clientcrt_${commonnameclient_var}.pem
		if [ $? -ne 0 ]
		then
			errorexit;
		fi;
	done
done
echo "RootCA1 Certificate is:"
${openssl_bin}  x509  -text -in  rootCA1crt.pem   -noout
echo "subCA1  Certificate is:"
${openssl_bin}  x509  -text -in  subCA1crt.pem    -noout
for commonnameserver_var in ${commonnameserver}
do
	echo "Server Certificate ${commonnameserver_var} is:"
	${openssl_bin}  x509  -text -in  servercrt_${commonnameserver_var}.pem -noout
done
for commonnameclient_var in ${commonnameclient}
do
	echo "Client Certificate ${commonnameclient_var} is:"
	${openssl_bin}  x509  -text -in  clientcrt_${commonnameclient_var}.pem   -noout
done
echo ""
echo ""
echo "What to do with all these certs.....?"
echo ""
echo "For Frontdoor usage install serverchain_XXX as servercrt.pem"
echo "                            serverkey_XXX.pem as serverkey.pem (private key)"
echo ""
echo "To import RootCA1 cert in Netscape you must rename the cert"
echo "to .crt and put it in a place where it can be loaded through a"
echo "webserver (with mime-type application/x-x509-ca-cert .crt enabled.)"
echo ""
echo "IE will just do the job with .pem - certs loaded from a directory"
echo ""
echo "For Apache Usage install servercrt_XXX.pem as SSLCertificateFile"
echo "                         serverkey_XXX.pem as SSLCertificateKeyFile (private key)"
echo "                         fullchain_XXX.pem as SSLCertificateChainFile"
echo "HINT for SSL module tests: the CN used in the certificate (XXX) must"
echo "correspond with the tests done in SSLCertificateTest.any! (peer verification)"
echo "Possible setting would be mkca.sh -S bernina.hsr.ch -S dylan.hsr.ch"
echo ""
echo "NOTE: Apache retrieves the server certificate through a hashed link."
echo "This is done with make rehash. Because only .crt files are recognized"
echo "rename servercrt.pem to servercrt.crt"
echo ""
echo "For Client usage install clientcrt_XXX.pem as CertFileClient"
echo "                         clientkey_XXX.pem as CertKeyClient (private key)"
echo "                         rootCA1.pem       as SSLPeerCAFile"
echo "                                              SSLVerifyDepth must be >=2"
echo "alternately use ***c_rehash*** and put certs contained in serverchain_XXX into SSLVerifyPath"
echo ""
echo "For SSLModule and StdDataAccess tests do the following:"
echo "                         cp clientcrt_XXX.pem clientcrt.pem"
echo "                         cp clientkey_XXX.pem clientkey.pem"
echo "                         cp servercrt_XXX.pem servercrt.pem"
echo "                         cp serverkey_XXX.pem serverkey.pem"
echo "                         cp serverchain_XXX.pem serverchain.pem"
echo "                         cp fullchain_XXX.pem fullchain.pem"
echo ""
echo "clientcrt_XXX.p12 may be used by your browser after you import it."
if [ -z ${pathtoexistingcacert} ]
then
	echo "Remember the PKCS#12 encrypted rootCA1.p12    cert password is rootca1"
fi
echo "Remember the PKCS#12 encrypted clientcrt.p12 cert password is client"
echo "Remember the password for the root CA is hank"
echo "Remember the password for the sub CA is gugus"

