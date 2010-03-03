#!/bin/bash

OPENSSL_PATH=/usr/bin
openssl_bin=${OPENSSL_PATH}/openssl
if [ -z $OPENSSL_PATH ]
then
	echo $OPENSSL_PATH
	echo "Could not find OPENSSL_PATH. Exiting. Set OPENSSL_PATH variable in $0"
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
function showhelp
{
        echo "$0 -d <Directory to find CA-Structure>"
        echo "        -D<time to expire in days, default is 365>"
        echo "        -c<copy the created certs to given location>"
        echo "        -u<user name, eg. Mark Muster>"
        exit 0
}

days=365
copyto=""
while getopts "d:D:c:u:" opt
do
        case $opt in
                d)      target_dir="${OPTARG}";
                ;;
                D)      days="${OPTARG}";
                ;;
                c)      copyto="${OPTARG}";
                ;;
                u)      username="${OPTARG}";
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
target_dir="${target_dir}/ca-hierarchy"
if [ ! -d $target_dir ]
then
		echo "$target_dir not usable. Exit.";
		exit -1;
fi
CATOP=./demoCA
cd ${target_dir}

echo "Creating client certificate"
USER_FILE_NAME=ad_user_${username// /_}
mkdir $USER_FILE_NAME
cd $USER_FILE_NAME
${openssl_bin} genrsa -out client.key 1024

echo "Creating certificate request for Client"
${openssl_bin} req -new  -passout pass:gaga -key client.key -config ../../../openssl4ad.cnf \
                           -out client.csr <<EOF1
loc
ifs
testdom

$username

Hello
EOF1

echo "Signing Client certificte by subCA1"
cd ../subCA1
${openssl_bin} x509 -req -days $days -passin pass:gugus  -CA ${CATOP}/cacert.pem \
			  -CAkey ${CATOP}/private/cakey.pem \
			  -CAcreateserial -in ../$USER_FILE_NAME/client.csr -out ../$USER_FILE_NAME/clientcert.pem

echo "Creating a PKCS#12 encoded copy of client.crt for \
Netcape use. Password is client"
${openssl_bin} pkcs12  -export -passin pass:gugus -passout pass:client -clcerts \
	 -in ../$USER_FILE_NAME/clientcert.pem  \
     -inkey ../$USER_FILE_NAME/client.key      -out ../$USER_FILE_NAME/clientcert.p12

