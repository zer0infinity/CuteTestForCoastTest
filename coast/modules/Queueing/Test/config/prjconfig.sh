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
#LOGDIR=log

#SERVERNAME="myApp"
#PRJ_DESCRIPTION="SomeCompany myApp"
#PID_FILE=$PROJECTDIR/$LOGDIR/$SERVERNAME.PID

# name of the tared and gzipped distribution file, caution: try to use short names
# because some CD burning software do only store 8.3 names
#TARGZNAME=$SERVERNAME.tgz

# overwrite this one ONLY if the WD_PATH variable points to the wrong place
#WD_PATH=config

# use this function to do preparations prior to creating the distribution package
# e.g. increment a build number
#function preDoallFunc {}

############################################################################
#
# from here on you can add your own variables which you can use in the
# project specific prjcopy.sh file for example
