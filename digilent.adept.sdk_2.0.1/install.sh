#!/bin/bash

###########################################################################
#                                                                         #
#  install.sh -- Linux Adept 2 SDK Installation Script                    #
#                                                                         #
###########################################################################
#  Author: MTA                                                            #
#  Copyright 2010 Digilent Inc.                                           #
###########################################################################
#  File Description:                                                      #
#                                                                         #
#  This is a bash shell script that can be used to install the Adept 2    #
#  SDK. This script should be executed from the root of the Adept 2 SDK   #
#  release directory. Please note that you need to run the script from an #
#  account with superuser privileges for the installation to succeed.     #
#                                                                         #
###########################################################################
#  Revision History:                                                      #
#                                                                         #
#  08/09/2010(MTA): created                                               #
#                                                                         #
###########################################################################

echo "Installing Adept 2 SDK"

###########################################################################
# Install include (header files).

incsrc="inc"
incdst="/usr/local/include/digilent/adept"

# Does the release contain include files?
if [ -d "${incsrc}" ]
then

	# The directory contains a include files directory. Does the include
	# files directory actually contain any header files?
	rgszIncs=($(ls "${incsrc}" | grep .h))
	if (( $? ))
	then
		echo "error: no include files were found"
		exit 1
	fi
else
	echo "error: no include files were found"
	exit 1
fi

# If we get this far then the release appears to contain include files.
# Let the user decide if the default installation location will be used
# or if the files will be installed somewhere else.
read -p "In which directory should include files be installed? [${incdst}] " inctmp
if [ -n "${inctmp}" ]; then incdst="${inctmp}"; fi

incdst="${incdst%/}"

# Install the include (header) files in the appropriate location.
echo "Installing include files....."

mkdir -p ${incdst}            && \
cp -f ${incsrc}/*.h ${incdst} && \
chmod 644 ${incdst}/*.h
if (( $? ))
then
	echo "error: failed to install include files"
	exit 1
fi

echo "Installed include files in \"${incdst}\""

###########################################################################
# Done

echo "Successfully installed Adept 2 SDK"

