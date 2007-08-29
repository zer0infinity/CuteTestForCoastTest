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

# The flag WD_USE_MMAP_STREAMS controls the usage of memory mapped files. Default is to use mmap streams
#  because for most operations and conditions this seems to be fast.
# When setting this variable to 0, fstreams will be used instead
# note: Memory mapped files will always increase the file size by an internally managed blocksize,
#  on SunOS_5.8, this blocksize seems to be 8192 bytes. If you intend to use a tail -f on these files
#  you will probably not get what you expect. tail can not handle the reserved - and still unused - space.
#export WD_USE_MMAP_STREAMS=0

# The flag TRACE_STORAGE defines the logging level of memory statistics
#  0: No pool statistic tracing, except when excess memory was used
#  1: Trace overall statistics
#  2: Trace detailed statistics
export TRACE_STORAGE=1

# use this function to do preparations prior to creating the distribution package
# e.g. increment a build number
#function preDoallFunc {}

############################################################################
#
# from here on you can add your own variables which you can use in the
# project specific prjcopy.sh file for example
