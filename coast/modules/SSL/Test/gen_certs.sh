#!/bin/ksh
function showhelp
{
    echo "$0 -e environment"
        exit 99
}

while getopts "e:" opt
do
        case $opt in
                e)      targetenv="${OPTARG}";
                ;;
                \?)
                        showhelp;
                ;;
        esac
done

if [ -z ${targetenv} ]
then
        showhelp
fi

if [ ${targetenv} == "hank" ]
then
	mkca.sh -d /tmp/mkca -D 3650 -S dylan.hank.com -C dylan.hank.com
fi

