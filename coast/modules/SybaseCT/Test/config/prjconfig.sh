#-----------------------------------------------------------------------------------------------------
# Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
# All rights reserved.
#
# This library/application is free software; you can redistribute and/or modify it under the terms of
# the license that is included with this library/application in the file license.txt.
#-----------------------------------------------------------------------------------------------------

############################################################################
#
# In this ection you should only adjust but NOT remove any of the given variables
# because these are used from within other scripts
#

# overwrite this one ONLY if the LOGDIR variable points to the wrong place
LOGDIR=.

#SERVERNAME="myApp"
#PRJ_DESCRIPTION="SomeCompany myApp"
#PID_FILE=$PROJECTDIR/$LOGDIR/$SERVERNAME.PID

# name of the tared and gzipped distribution file, caution: try to use short names
# because some CD burning software do only store 8.3 names
#TARGZNAME=$SERVERNAME.tgz

# overwrite this one ONLY if the WD_PATH variable points to the wrong place
#WD_PATH=config

############################################################################
#
# from here on you can add your own variables which you can use in the
# project specific prjcopy.sh file for example

SybInPrjDir=${PROJECTDIR}/sybase
SybIn3rdParty=${DEV_HOME}/3rdparty/installed/sybase

if [ -d "${SybInPrjDir}" ]; then
        export SYBASE=${SybInPrjDir};
elif [ -d "${SybIn3rdParty}" ]; then
        export SYBASE=${SybIn3rdParty};
else
        if [ -z "${SYBASE}" ]; then
                echo 'ERROR: SYBASE variable not set!'
        else
                echo 'WARNING: check if you really wanted to use this Sybase-Dir
ectory ['${SYBASE}']';
        fi;
fi
echo 'prjconfig.sh: using sybase from ['${SYBASE}']';

