#!/bin/sh
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
DEV_HOME=/home/builduser/build/coast
OPENSSL_CFG_PATH=$DEV_HOME/tests/modules/SSL
conf=$OPENSSL_CFG_PATH/openssl.cnf
country="CH"
state="ZH"
locality="ZH"
org="Test"
orgunit="CertIssueing"
emailsuffix="wildplanet.com"
commonnameclient="IAMACLIENT"
commonnameserver="IAMASERVER"
typeset -l clientemail="${commonnameclient}@${emailsuffix}"

openssl_bin=`which openssl`

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
    echo "        -D<default is 365>"
    echo "        -c<copy the created certs to given location>"
    echo "        -C common name (subject) of client, entity which is checked in cert verify"
    echo "        -S common name (subject) of server, entity which is checked in cert verify"
	echo ""
	echo "The files you want to use are in the directory"
	echo "given with -d in ca-hierarchy/ucerts"
    exit 0
}

days=365
copyto=""
while getopts "d:D:c:C::e:S:" opt
do
        case $opt in
                d)      target_dir="${OPTARG}";
                ;;
                D)      days="${OPTARG}";
                ;;
                c)      copyto="${OPTARG}";
                ;;
                C)      commonnameclient="${OPTARG}";
                ;;
                S)      commonnameserver="${OPTARG}";
                ;;
                e)      emailclient="${OPTARG}";
                ;;
                \?)
                        showhelp;
                ;;
        esac
done

shift $(($OPTIND - 1))

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

echo "Creating a PKCS#12 encoded copy of rootCA1.pem for \
Netcape use. Password is rootca1"
${openssl_bin} pkcs12 -passin pass:hank  -passout pass:rootca1 -export \
	 -in ${CATOP}/$CACERT   \
         -inkey ${CATOP}/private/$CAKEY -out ${CATOP}/cacert.p12

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

echo "Stripping Certificate and private Key from Certificate for SubCA1"
csplit -f work -s -n 2  ../newreq.pem '/-----BEGIN CERTIFICATE REQUEST-----/'
mv work00 ${CATOP}/private/cakey.pem
rm work01

echo "Stripping plaintext from SubCA1 Certificate"
${openssl_bin} x509  -in ../newcert.pem -out ${CATOP}/cacert.pem

echo "Creating certificate request for Server"
${openssl_bin} req -config $conf -new  -passout pass:gaga -keyout newreq.pem \
                           -out newreq.pem <<EOF1
${country}
${state}
${locality}
ServerSite
${orgunit}
${commonnameserver}
${commonnameserver}@${emailsuffix}
.
.
EOF1

echo "Signing CA request for Server"
${openssl_bin} ca -config $conf -policy policy_anything \
	-passin pass:gugus $DAYS -out newcert.pem \
	-infiles newreq.pem <<EOF2
y
y
EOF2

echo "Stripping signed Certificate for Server"
${openssl_bin} x509  -in newcert.pem -out xx.pem
mv xx.pem newcert.pem
${openssl_bin} rsa  -passin pass:gaga -in newreq.pem -out newreq.plain.pem

echo "Creating client certificate"
cd ${target_dir}
mkdir client
cd client
${openssl_bin} genrsa -out client.key 1024

echo "Creating certificate request for Client"
${openssl_bin} req -config $conf -new  -passout pass:gaga -key client.key \
                           -out client.csr <<EOF1
${country}
${state}
${locality}
ClientSite
${orgunit}
${commonnameclient}
${commonnameclient}@${emailsuffix}
.
.
EOF1

echo "Signing Client certificte by subCA1"
cd ${target_dir}/subCA1
${openssl_bin} x509 -req $DAYS -passin pass:gugus  -CA ${CATOP}/cacert.pem \
			  -CAkey ${CATOP}/private/cakey.pem \
			  -CAcreateserial -in ${target_dir}/client/client.csr -out ${target_dir}/client/clientcrt.pem

echo "Creating a PKCS#12 encoded copy of client.crt for \
Netcape use. Password is client"
${openssl_bin} pkcs12 -export -passin pass:gugus -passout pass:client -clcerts \
	 -in ${target_dir}/client/clientcrt.pem  \
     -inkey ${target_dir}/client/client.key      -out ${target_dir}/client/clientcrt.p12

echo "Building result directory  ${target_dir}/ucerts"
mkdir ${target_dir}/ucerts
cp  ${CATOP}/cacert.pem ${target_dir}/ucerts/subCA1.pem
cp  ./newcert.pem       ${target_dir}/ucerts/servercrt.pem
cp  ./newreq.plain.pem  ${target_dir}/ucerts/serverkey.pem
cd ..
cp ${CATOP}/cacert.pem ${target_dir}/ucerts/rootCA1.pem
cp ${CATOP}/cacert.p12 ${target_dir}/ucerts/rootCA1.p12
cd client
cp  clientcrt.p12     ${target_dir}/ucerts/clientcrt.p12
cp  clientcrt.pem	   ${target_dir}/ucerts/clientcrt.pem
cp  client.key		   ${target_dir}/ucerts/clientkey.pem

cd ${target_dir}/ucerts
echo "Creating Chain without Server Cert)"
cp  subCA1.pem     chainwoservercrt.pem
cat rootCA1.pem >> chainwoservercrt.pem
echo "Creating Full Cain file (Server/subCA1/rootCA1)"
cp  servercrt.pem     fullchain.pem
cat subCA1.pem     >> fullchain.pem
cat rootCA1.pem    >> fullchain.pem
echo "Creating Chain without the CA the client trusts (without
rootCA1)"
echo "Use this in Frontdoor"
cp  servercrt.pem     frontdoorchain.pem
cat subCA1.pem     >> frontdoorchain.pem

echo "Your certificates are in ${target_dir}/ucerts"
if [ ! -z ${copyto} ]
then
	if [ ! -d ${copyto} ]
	then
		mkdir ${copyto}
	fi
	echo "Copying SERVER related files: serverkey servercrt intermediate.crt  to ${copyto}:"
	cp servercrt.pem          ${copyto}
	cp serverkey.pem          ${copyto}
	cp chainwoservercrt.pem   ${copyto}
	cp fullchain.pem          ${copyto}
	cp frontdoorchain.pem     ${copyto}
	echo "Copying CLIENT related files: clientcrt.pem client.key clientcrt.p12 to ${copyto}:"
	cd ${target_dir}/client
	cp clientcrt.pem	 ${copyto}
	cp clientkey.pem	 ${copyto}
	cp clientcrt.p12	 ${copyto}
	cd ${target_dir}/ucerts
fi
echo "Verifying cert chain with -untrusted option"
${openssl_bin} verify -verbose  -purpose sslserver \
-CAfile rootCA1.pem -untrusted subCA1.pem servercrt.pem
echo "Verifying cert chain without server cert (chainwoservercrt.pem)"
${openssl_bin} verify -verbose  -purpose sslserver \
-CAfile chainwoservercrt.pem servercrt.pem
echo "Verifying cert chain with full chain  (fullchain.pem)"
${openssl_bin} verify -verbose  -purpose sslserver \
-CAfile fullchain.pem servercrt.pem
echo "Verifying cert chain with full chain and clientcrt.pem"
${openssl_bin} verify -verbose  -purpose sslclient \
-CAfile fullchain.pem clientcrt.pem
echo "RootCA1 Certificate is:"
${openssl_bin}  x509  -text -in  rootCA1.pem   -noout
echo "subCA1  Certificate is:"
${openssl_bin}  x509  -text -in  subCA1.pem    -noout
echo "Server  Certificate is:"
${openssl_bin}  x509  -text -in  servercrt.pem -noout
echo "Client Certificate is:"
${openssl_bin}  x509  -text -in  clientcrt.pem   -noout
echo ""
echo ""
echo "What to do with all these certs.....?"
echo ""
echo "For Frontdoor usage install frondoorchain as servercrt.pem"
echo "                            serverkey.pem as serverkey.pem"
echo ""
echo "To import RootCA1 cert in Netscape you must rename the cert"
echo "to .crt and put it in a place where it can be loaded through a"
echo "webserver (with mime-type application/x-x509-ca-cert .crt enabled.)"
echo ""
echo "IE will just do the job with .pem - certs loaded from a directory"
echo ""
echo "For Apache Usage install servercrt.pem as SSLCertificateFile"
echo "                         serverkey.pem as SSLCertificateKeyFile"
echo "                         fullchain.pem as
SSLCertificateChainFile"
echo "Note: Apache retrieves the server certificate through a hashed link."
echo "This is done with make rehash. Because only .crt files are recognized"
echo "rename servercrt.pem to servercrt.crt"
echo ""
echo "clientcrt.p12 may be used by your browser after you import it."
echo "Remember the PKCS#12 encrypted rootCA1.p12    cert password is rootca1"
echo "Remember the PKCS#12 encrypted clientcrt.p12 cert password is client"
echo "Remember the password for the root CA is hank"
echo "Remember the password for the sub CA is gugus"

