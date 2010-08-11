#!/bin/sh
#--------------------------------------------------------------------
# Copyright (c) 2010, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
# All rights reserved.
#
# This library/application is free software; you can redistribute and/or modify it under the terms of
# the license that is included with this library/application in the file license.txt.
#--------------------------------------------------------------------

MYNAME=`basename $0`

country="CH"
state="ZH"
locality="ZH"
org="Test"
orgunit="CertIssueing"
emailsuffix="wildplanet.com"
commonnameclient=""
commonnameserver=""
pathtoexistingcacert=""
digest="md5"
openssl_bin=`which openssl`
openssl_binOpts=""
conf=/etc/ssl/openssl.cnf
target_dir=""
serialnum=`date +'%s'`
if [ "$serialnum" = "%s" ]; then
	# date seems to be non-gnu style, workaround approximation!
	serialnum=`expr \( \( \`date +%Y\` - 1970 \) \* 365 + \( \`date +%Y\` - 1970 \) / 4 - 1 + \`date +%j\` \) \* 86400 + \`date +%H\` \* 3600 + \`date +%M\` \* 60 + \`date +%S\` `
fi

errorexit()
{
	echo "!!!!!!!!!!!!!!!!!! CERTIFICATE VERIFICATION FAILURE!!!!!!!!!!!!!!!"
	exit 99
}

showhelp()
{
	echo ''
	echo 'usage: '$MYNAME' [options]'
	echo 'where options are:'
	echo ' -b <path2sslbin> : full path and name to openssl binary, default '${openssl_bin}
	echo ' -B <options>     : options to openssl binary given in -b'
	echo ' -o <path2sslcnf> : full path and name to ssl configuration, default '${conf}
    echo ' -d <path>        : directory to place CA-Structure into. creates ca-hierarchy within this directory'
    echo ' -D <days>        : validity duration of generated certificates, default is 3650'
    echo ' -c <path>        : copy the created certs to given location'
    echo ' -E <path>        : location where previously built rootCA1 and subCA1 certs are kept'
    echo '                    This allows you to add additional client/server certs without re-generating previously generated certificates.'
    echo ' -C <common name> : subject of client, entity which is checked in cert verify, may be given repeatedly'
    echo ' -S <common name> : subject of server, entity which is checked in cert verify, may be given repeatedly'
    echo ' -s <digest>      : message digest used to sign server/client certificate. choose one of: md2/md5/sha1/mdc/mdc2, default is md5'
    echo ''
    exit 3
}

days=3650
copyto=""
while getopts ":b:B:o:d:D:c:C:e:E:s:S:-" opt
do
    case $opt in
		:)
			echo "ERROR: -$OPTARG parameter missing, exiting!";
			showhelp;
		;;
        b)
        	if [ -r ${OPTARG} -a -x ${OPTARG} ]; then
        		openssl_bin="${OPTARG}";
			else
				echo "openssl binary [${OPTARG}] not found or not executable, using default: ${openssl_bin}";
    		fi;
        ;;
        B)
        	if [ -n "${OPTARG}" ]; then
        		openssl_binOpts="${OPTARG}";
    		fi;
        ;;
        o)
    		# openssl needs absolute path to config file
    		_conf="`dirname ${OPTARG}`";
    		_conf="`cd ${_conf} 2>/dev/null && pwd -P`";
    		_conf="${_conf}/`basename ${OPTARG}`";
        	if [ -n "${_conf}" -a -r ${_conf} ]; then
				conf="${_conf}";
    		fi;
        ;;
        d)      target_dir="${OPTARG}";
        ;;
        D)      days="${OPTARG}";
        ;;
        c)      copyto="${OPTARG}";
        ;;
        C)      commonnameclient="${commonnameclient} ${OPTARG}";
        ;;
        s)      digest="${OPTARG}";
        ;;
        S)      commonnameserver="${commonnameserver} ${OPTARG}";
        ;;
        e)      emailclient="${OPTARG}";
        ;;
        E)      pathtoexistingcacert="${OPTARG}";
        ;;
		-)
			break;
		;;
        \?)
            showhelp;
        ;;
    esac
done

shift `expr $OPTIND - 1`

if [ ! -x ${openssl_bin} ]
then
	echo "Could not find openssl binary."
	showhelp;
fi
echo "Using openssl binary ${openssl_bin}"
openssl_bin="${openssl_bin} ${openssl_binOpts}"

if [ ! -f $conf ]
then
    echo "Could not find openssl configuration file ${conf}"
    showhelp;
fi
echo "Using openssl config file ${conf}"

if [ -z "${commonnameclient}" ]
then
	commonnameclient="IAMACLIENT";
fi
if [ -z "${commonnameserver}" ]
then
	commonnameserver="IAMASERVER";
fi

if [ -z "${target_dir}" ]
then
	echo "target directory not set, exiting!"
    showhelp;
fi

target_dir=${target_dir}/ca-hierarchy

if [ ! -d ${target_dir} ]
then
    mkdir -p ${target_dir}
    if [ $? -ne 0 ]
    then
        echo "Can't create ${target_dir}"
        exit 1
    fi
fi
cd ${target_dir}

echo "Creating RootCA1"
rm -Rf ${target_dir}/*
mkdir ${target_dir}/ucerts

CATOP=demoCA
CAKEY=cakey.pem
CACERT=cacert.pem

mkdir ${CATOP}
mkdir ${CATOP}/certs
mkdir ${CATOP}/crl
mkdir ${CATOP}/newcerts
mkdir ${CATOP}/private
echo ${serialnum} > ${CATOP}/serial
touch ${CATOP}/index.txt

if [ -z "${pathtoexistingcacert}" ]
then
	${openssl_bin} req \
		-config $conf \
		-new \
		-x509 \
		-passout pass:hank \
		-keyout ${CATOP}/private/$CAKEY \
		-out ${CATOP}/$CACERT \
		-days $days <<-EOF
			${country}
			${state}
			${locality}
			RootCA1
			${orgunit}
			RootCA1
			rootca1@${emailsuffix}

EOF

echo "Creating a PKCS#12 encoded copy of rootCA1.pem for Netcape use. Password is rootca1"
${openssl_bin} pkcs12 \
	-passin pass:hank \
	-passout pass:rootca1 \
	-export \
	-in ${CATOP}/$CACERT \
	-inkey ${CATOP}/private/$CAKEY \
	-out ${CATOP}/cacert.p12

echo "Stripping passphrase from rootCA1key.pem"
${openssl_bin} rsa \
	-passin pass:hank \
	-in ${CATOP}/private/$CAKEY \
	-out ${target_dir}/ucerts/rootCA1key.pem

echo "Certificate Request for SubCA1"
${openssl_bin} req \
	-config $conf \
	-new \
	-passout pass:gugus \
	-keyout ${target_dir}/newreq.pem \
	-out ${target_dir}/newreq.pem <<-EOF1
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
${openssl_bin} ca \
	-config $conf \
	-policy policy_anything \
	-passin pass:hank \
	-out ${target_dir}/newcert.pem \
	-extensions v3_ca \
	-days $days \
	-infiles ${target_dir}/newreq.pem <<-EOF2
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

mkdir subCA1
cd subCA1

mkdir ${CATOP}
mkdir ${CATOP}/certs
mkdir ${CATOP}/crl
mkdir ${CATOP}/newcerts
mkdir ${CATOP}/private
echo ${serialnum} > ${CATOP}/serial
touch ${CATOP}/index.txt

if [ -z "${pathtoexistingcacert}" ]
then
	echo "Stripping Certificate and private Key from Certificate for SubCA1"
	csplit -f work -s -n 2  ${target_dir}/newreq.pem '/-----BEGIN CERTIFICATE REQUEST-----/'
	mv work00 ${CATOP}/private/cakey.pem
	rm work01
	echo "Stripping passphrase from subCA1key.pem"
	${openssl_bin} rsa \
		-passin pass:gugus \
		-in ${CATOP}/private/$CAKEY \
		-out ${target_dir}/ucerts/subCA1key.pem
	cp ${CATOP}/private/cakey.pem ${target_dir}/ucerts/subCA1key.pem
	echo "Stripping plaintext from SubCA1 Certificate"
	${openssl_bin} x509 \
		-in ${target_dir}/newcert.pem \
		-out ${CATOP}/cacert.pem
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
	${openssl_bin} req \
		-config $conf \
		-new \
		-passout pass:gaga \
		-keyout newreq_${commonnameserver_var}.pem \
		-out newreq_${commonnameserver_var}.pem <<-EOF1
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
	${openssl_bin} ca \
		-md $digest \
		-config $conf \
		-policy policy_anything \
		-passin pass:gugus \
		-days $days \
		-out newcert_${commonnameserver_var}.pem \
		-infiles newreq_${commonnameserver_var}.pem <<-EOF2
			y
			y
EOF2

	echo "Stripping signed Certificate for Server ${commonnameserver_var}"
	${openssl_bin} x509 \
		-in newcert_${commonnameserver_var}.pem \
		-out xx_${commonnameserver_var}.pem
	mv xx_${commonnameserver_var}.pem newcert_${commonnameserver_var}.pem
	${openssl_bin} rsa  \
		-passin pass:gaga \
		-in newreq_${commonnameserver_var}.pem \
		-out newreq_${commonnameserver_var}.plain.pem
done

mkdir ${target_dir}/client && cd ${target_dir}/client && (

	for commonnameclient_var in ${commonnameclient}
	do
		echo "Creating client certificate for ${commonnameclient_var}"
		${openssl_bin} genrsa \
			-out client_${commonnameclient_var}.key 1024

		echo "Creating certificate request for Client ${commonnameclient_var}"
		${openssl_bin} req \
			-config $conf \
			-new \
			-passout pass:gaga \
			-key client_${commonnameclient_var}.key \
			-out client_${commonnameclient_var}.csr <<-EOF1
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
		${openssl_bin} x509 \
			-$digest \
			-req \
			-days $days \
			-passin pass:gugus \
			-CA ${target_dir}/subCA1/${CATOP}/cacert.pem \
			-CAkey ${target_dir}/subCA1/${CATOP}/private/cakey.pem \
			-CAcreateserial \
			-in client_${commonnameclient_var}.csr \
			-out clientcrt_${commonnameclient_var}.pem

		echo "Creating a PKCS#12 encoded copy of client.crt for Netcape use. Password is client"
		${openssl_bin} pkcs12 \
			-export \
			-passin pass:gugus \
			-passout pass:client \
			-clcerts \
			-in clientcrt_${commonnameclient_var}.pem \
	     	-inkey client_${commonnameclient_var}.key \
	     	-out clientcrt_${commonnameclient_var}.p12
	done
)

cd ${target_dir}/subCA1 && (
	echo "Building result directory  ${target_dir}/ucerts"
	cp  ${CATOP}/cacert.pem ${target_dir}/ucerts/subCA1crt.pem
	for commonnameserver_var in ${commonnameserver}
	do
		cp  newcert_${commonnameserver_var}.pem       ${target_dir}/ucerts/servercrt_${commonnameserver_var}.pem
		cp  newreq_${commonnameserver_var}.plain.pem  ${target_dir}/ucerts/serverkey_${commonnameserver_var}.pem
	done
)

cd ${target_dir} && (
	cp ${CATOP}/cacert.pem ${target_dir}/ucerts/rootCA1crt.pem
	if [ -z "${pathtoexistingcacert}" ]
	then
		cp ${CATOP}/cacert.p12 ${target_dir}/ucerts/rootCA1.p12
	fi
)

cd ${target_dir}/client && (
	for commonnameclient_var in ${commonnameclient}
	do
		cp  clientcrt_${commonnameclient_var}.p12      ${target_dir}/ucerts/clientcrt_${commonnameclient_var}.p12
		cp  clientcrt_${commonnameclient_var}.pem	   ${target_dir}/ucerts/clientcrt_${commonnameclient_var}.pem
		cp  client_${commonnameclient_var}.key		   ${target_dir}/ucerts/clientkey_${commonnameclient_var}.pem
	done
)

cd ${target_dir}/ucerts && (
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
)

echo "Your certificates are in ${target_dir}/ucerts"
if [ ! -z "${copyto}" ]
then
	if [ ! -d ${copyto} ]
	then
		mkdir ${copyto}
	fi
	echo "Copying SERVER related files: serverkey servercrt ...  to ${copyto}:"
	cd ${target_dir}/ucerts && (
		for commonnameserver_var in ${commonnameserver}
		do
			cp servercrt_${commonnameserver_var}.pem          ${copyto}
			cp serverkey_${commonnameserver_var}.pem          ${copyto}
			cp chainwoservercrt.pem   ${copyto}
			cp fullchain_${commonnameserver_var}.pem          ${copyto}
			cp serverchain_${commonnameserver_var}.pem    	  ${copyto}
		done
		echo "Copying CLIENT related files: clientcrt clientkey clientcrt.p12 to ${copyto}:"
		for commonnameclient_var in ${commonnameclient}
		do
			cp clientcrt_${commonnameclient_var}.pem	 ${copyto}
			cp clientkey_${commonnameclient_var}.pem	 ${copyto}
			cp clientcrt_${commonnameclient_var}.p12	 ${copyto}
		done

		echo "Copying CA related files: subCA1 rootCA1 to ${copyto}:"
		cp subCA1*.pem	 ${copyto}
		cp rootCA1*.pem	 ${copyto}
	)
fi

echo "Verifying certificates in ${target_dir}/ucerts"
cd ${target_dir}/ucerts
for commonnameserver_var in ${commonnameserver}
do
	echo "Verifying cert chain with -untrusted option for ${commonnameserver_var}"
	${openssl_bin} verify \
		-verbose \
		-purpose sslserver \
		-CAfile rootCA1crt.pem \
		-untrusted subCA1crt.pem \
		servercrt_${commonnameserver_var}.pem || errorexit;

	echo "Verifying cert chain without server cert (chainwoservercrt.pem) for ${commonnameserver_var}"
	${openssl_bin} verify \
		-verbose \
		-purpose sslserver \
		-CAfile chainwoservercrt.pem \
		servercrt_${commonnameserver_var}.pem || errorexit;

	echo "Verifying cert chain with full chain  (fullchain.pem)"
	${openssl_bin} verify \
		-verbose \
		-purpose sslserver \
		-CAfile fullchain_${commonnameserver_var}.pem \
		servercrt_${commonnameserver_var}.pem || errorexit;
done

for commonnameclient_var in ${commonnameclient}
do
	echo "Verifying cert chain with full chain and clientcrt.pem for ${commonnameclient_var}"
	${openssl_bin} verify \
		-verbose \
		-purpose sslclient \
		-CAfile fullchain_${commonnameserver_var}.pem \
		clientcrt_${commonnameclient_var}.pem || errorexit;
done

echo "RootCA1 Certificate is:"
${openssl_bin} x509 \
	-text \
	-in rootCA1crt.pem \
	-noout
echo "subCA1  Certificate is:"
${openssl_bin} x509 \
	-text \
	-in subCA1crt.pem \
	-noout
for commonnameserver_var in ${commonnameserver}
do
	echo "Server Certificate ${commonnameserver_var} is:"
	${openssl_bin} x509 \
		-text \
		-in servercrt_${commonnameserver_var}.pem \
		-noout
done
for commonnameclient_var in ${commonnameclient}
do
	echo "Client Certificate ${commonnameclient_var} is:"
	${openssl_bin} x509 \
		-text \
		-in clientcrt_${commonnameclient_var}.pem \
		-noout
done

echo ""
echo "What to do with all these certs.....?"
echo ""
echo "For proxy usage install serverchain_XXX as servercrt.pem"
echo "                        serverkey_XXX.pem as serverkey.pem (private key)"
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
echo "Possible setting would be mkca.sh -S bernina.hsr.ch -S dylan.hank.com"
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
if [ -z "${pathtoexistingcacert}" ]
then
	echo "Remember the PKCS#12 encrypted rootCA1.p12    cert password is rootca1"
fi
echo "Remember the PKCS#12 encrypted clientcrt.p12 cert password is client"
echo "Remember the password for the root CA is hank"
echo "Remember the password for the sub CA is gugus"

exit 0
