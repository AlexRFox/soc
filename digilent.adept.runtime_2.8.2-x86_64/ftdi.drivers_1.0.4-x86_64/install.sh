#!/bin/bash

###########################################################################
#                                                                         #
#  install.sh -- FTDI Driver Installation Script                          #
#                                                                         #
###########################################################################
#  Author: MTA                                                            #
#  Copyright 2011 Digilent Inc.                                           #
###########################################################################
#  File Description:                                                      #
#                                                                         #
#  This is a bash shell script that can be used to install the FTDI       #
#  d2xx driver for Linux.                                                 #
#                                                                         #
#  This script optionally accepts two parameters. The parameters are      #
#  specified using key=value pairs. This parameters are descript below.   #
#                                                                         #
#  "instpath=pathname" - pathname to install the file to                  #
#  "silent=1" - run in silent mode (e.g. don't prompt the user)           #
#                                                                         #
#  Specifying a pathname doesn't gaurantee that the shared libraries      #
#  used by the driver will actually be installed in that location. If     #
#  there's another version of the library already installed then the      #
#  library will be installed in the same directory that the existing      #
#  version is installed in. In the event that no path is specified a      #
#  reasonable default path will be chosen based on the system             #
#  configuration. If the script isn't running in silent mode then the     #
#  user will be given the opportunity to use the default path or specify  #
#  a different one.                                                       #
#                                                                         #
#  Exit Codes:                                                            #
#                                                                         #
#  If an error occurs during installation then a message will be sent to  #
#  stdout and the script will exit with a non-zero code indicating its    #
#  error status. Below is a list of error codes and their meaning.        #
#                                                                         #
#  1 - invalid parameter (e.g. you forgot to specify a parameter)         #
#  2 - bad parameter (e.g. shared library filename is no good)            #
#  3 - failed to create a directory (e.g. mkdir error)				      #
#  4 - failed to copy a file or set permissions						      #
#  5 - error updating symbolic link (most likely due to it being in use)  #
#  6 - couldn't find files (corrupt installation source)                  #
#  22 - installer run by a user without superuser (root) privileges       #
#  40 - no suitable destination dir found for shared library installation #
#                                                                         #
#  Example Usage:                                                         #
#                                                                         #
#  "install-slib.sh lib/libdabs.so.2.7.1 /usr/local/lib/digilent/adept"   #
#                                                                         #
###########################################################################
#  Revision History:                                                      #
#                                                                         #
#  03/04/2011(MTA): created                                               #
#  04/06/2011(MTA): updated to use multiple default installation          #
#      directories in case one fails due to a ready only file system      #
#      being present                                                      #
#                                                                         #
###########################################################################

##################################################################
####	GetDestPath
##
##	Parameters:
##		1 - destination directory string
##		2 - parent directory of the destination directory
##		3 - basename string
##
##	Return Value:
##		destination path if destination directory or parent directory
##		exists and is writable, emptry string otherwise
##
##	Errors:
##		none
##
##	Description:
##		This function constructs a destination path string from a
##		destination directory string and a basename string. Before
##		constructing the destination path string a check is done to see if
##		the destination directory exists and is writable. If the
##		destination directory exists and is writable or the parent
##		directory of the destination directory exists and is writable then
##		a destination path string is constructed. If neither directory
##		exists or they exist but aren't writable then an empty string is
##		returned.
##
function GetDestPath()
{
	local szDestDir
	local szDestDirParent
	local szBasename
	local szDestPath
	
	szDestDir="${1}"
	szDestDirParent="${2}"
	szBasename="${3}"
	szDestPath=""
	
	# Check to see if the destination directory exists.
	if [ -d "${szDestDir}" ]
	then
		if [ -w "${szDestDir}" ]
		then
			if [ -n "${szBasename}" ]
			then
				szDestPath="${szDestDir}/${szBasename}"
			else
				szDestPath="${szDestDir}"
			fi
		fi
	elif [ -d "${szDestDirParent}" ]
	then
		if [ -w "${szDestDirParent}" ]
		then
			if [ -n "${szBasename}" ]
			then
				szDestPath="${szDestDir}/${szBasename}"
			else
				szDestPath="${szDestDir}"
			fi
		fi
	fi
	
	echo "${szDestPath}"
}

# Set globals to their default values.
let fNotSilent=1
let fExistingInstall=0
szLibDst=""
szLibSrc=""
szSilent="silent=0"

# Go through the list of command line arguments and parse them.
for szArg in "$@"
do
	case "${szArg}" in
		instpath=* )
			szLibDst=$(echo ${szArg} | sed 's/instpath=//');;
		silent=1 )
			szSilent="silent=1";
			let fNotSilent=0;;
	esac
done

# Inform the user that they are running the FTDI Driver Install Script
echo "FTDI Driver Installer"

# Make sure the user has superuser (root) privileges.
if (( $EUID != 0 ))
then
	echo "error: superuser (root) privileges are required for installation"
	exit 22
fi

###########################################################################
# Determine which directory contains the libraries and where we should
# install shared libraries.

szMname=$(uname -m)
if [ "${szMname}" = "x86_64" ]
then
	echo "64-bit operating system detected"
	
	szLibSrc="lib64"
else
	echo "32-bit operating system detected"
	
	szLibSrc="lib"
fi

rgszLibDst=()
let cszLibDst=0

# Was the destination directory specified via command line argument?
if [ -n "${szLibDst}" ]
then
	# Yes so we will only attempt to install the shared libraries in that
	# directory.
	
	rgszLibDst[0]="${szLibDst}"
	let cszLibDst=1
else
	# No so we will generate a list of possible installation directories
	# based on the system configuration.
	
	rgszDestDir=()
	rgszDestDirParent=()
	
	if [ "${szMname}" = "x86_64" ] && [ -d "/lib64" -o -d "/usr/lib64" -o -d "/usr/local/lib64" ]
	then
		rgszDestDir[0]="/usr/local/lib64"
		rgszDestDirParent[0]="/usr/local"
	
		rgszDestDir[1]="/usr/lib64"
		rgszDestDirParent[1]=""
	else
		rgszDestDir[0]="/usr/local/lib"
		rgszDestDirParent[0]="/usr/local"
	
		rgszDestDir[1]="/usr/lib"
		rgszDestDirParent[1]=""
	fi
	
	for (( i=0; i < ${#rgszDestDir[*]}; i++ ))
	do
		szDestPath=$(GetDestPath "${rgszDestDir[${i}]}" "${rgszDestDirParent[${i}]}" "")
		if [ -n "${szDestPath}" ]
		then
			rgszLibDst[${cszLibDst}]="${szDestPath}"
			let cszLibDst+=1
		fi
	done
fi

if [ -z "${szLibDst}" ]
then
	# No destination directory was specified via command line argument. If
	# we aren't running in silent mode then we need to prompt the user and
	# allow he or she to confirm the default installation directory or
	# specify another one.
	
	if (( $fNotSilent ))
	then
		if (( $cszLibDst ))
		then
			# We previously determined at least one suitable default
			# installation directory. Allow the user to confirm this
			# directory or to specify another one.
			
			read -p "In which directory should libraries be installed? [${rgszLibDst[0]}] " szTemp
			if [ -n "${szTemp}" ]
			then
				# The user specified an installation directory so we will
				# only attempt to install shared libraries in that
				# directory.
				
				rgszLibDst[0]="${szTemp}"
				let cszLibDst=1
			fi
		else
			# We didn't previously determine any suitable default
			# installation directories. The user must specify a directory
			# before we can continue.
			
			szTemp=""
			while [ -z "${szTemp}" ]
			do
				read -p "In which directory should libraries be installed? " szTemp
			done
			
			rgszLibDst[0]="${szTemp}"
			let cszLibDst=1
		fi
	fi
fi

if (( ! $cszLibDst ))
then
	echo "error: unable to determine suitable installation directory for shared libraries"
	exit 40
fi

###########################################################################
# Install the shared libraries.

# Create a list of shared libraries to install.
rgszLibs=($(ls ${szLibSrc} | grep .so))
if (( $? ))
then
	echo "error: couldn't find any shared libraries in ${szLibSrc}"
	exit 6
fi

# Go through the list of shared libraries and install them.
let i=0
let iszLibDst=0
while (( $i < ${#rgszLibs[*]} ))
do
	./install-slib.sh "${szLibSrc}/${rgszLibs[${i}]}" "${rgszLibDst[${iszLibDst}]}" "${szSilent}"
	let ret=$?
	if (( $ret ))
	then
		# An error occured.
		
		if (( $ret == 30 ))
		then
			# An existing installation of the library currently being
			# installed was found on the system. That installation was
			# upgraded or the same version of the library is already
			# installed on the system. We must set fExistingInstall so that
			# we don't overwrite any existing dynamic loader configuration.
			
			let fExistingInstall=1
			let i+=1
		elif (( $ret == 3 || $ret == 4 )) && (( $i == 0 ))
		then
			# Failed to create the destination directory that the library
			# is being installed to. If we previously determined an
			# alternate installation location then we will attempt to use
			# that location during the next pass through the loop.
			
			let iszLibDst+=1
			
			if (( $iszLibDst == $cszLibDst ))
			then
				echo "error: failed to install ${szLibSrc}/${rgszLibs[${i}]}"
				exit $ret
			fi
		else
			# An unknown error occured. We can't continue the installation.
			
			echo "error: failed to install ${szLibSrc}/${rgszLibs[${i}]}"
			exit $ret
		fi
	else
		# No error occured so proceed to install the next library during
		# the next pass through the loop.
		
		let i+=1
	fi
done

###########################################################################
# Generate and install dynamic loader configuration file. This file is used
# to tell the dynamic loader to search a specific directory for shared
# libraries in addition to the other directories that it normally searches.

szDlcSrc="ftdi-drivers.conf"
szDlcDst="/etc/ld.so.conf.d"

# We only need to generate a config file if an existing installation of
# the shared library wasn't found during installation. If an existing
# installation was found then the dynamic loader already knows how to find
# the library in the directory that it's installed in.
if (( ! $fExistingInstall ))
then
	# Remove any existing copy of the dynamic loader config file.
	rm -f ${szDlcSrc}
	if (( $? ))
	then
		echo "error: failed to remove existing dynamic loader configuration ${szDlcSrc}"
		szDlcSrc=""
	fi

	if [ -n "${szDlcSrc}" ]
	then
		# Generate the dynamic loader configuration file.
		echo "# Path to libftd2xx" >> "${szDlcSrc}" && \
		echo "${rgszLibDst[${iszLibDst}]}" >> "${szDlcSrc}"
		if (( $? ))
		then
			echo "error: failed to generate dynamic loader configuration file"
			szDlcSrc=""
		fi
	fi
	
	if [ -n "${szDlcSrc}" ]
	then
		# Install the dynamic loader configuration file.
		cp -f ${szDlcSrc} ${szDlcDst} && \
		chmod 644 ${szDlcDst}/$(basename ${szDlcSrc})
		if (( $? ))
		then
			echo "error: failed to install dynamic loader configuration file"
			szDlcSrc=""
		fi
	fi
	
	if [ -n "${szDlcSrc}" ]
	then
		rm -f ${szDlcSrc}
	
		echo "Successfully installed dynamic loader configuration \"${szDlcDst}/$(basename ${szDlcSrc})\""
	fi
fi

###########################################################################
# Perform post installation tasks.

# If we installed a dynamic loader configuration file or we need to update
# the dynamic loader's cache then szDlcSrc is a string with a non-zero
# length.
if [ -n "${szDlcSrc}" ]
then
	/sbin/ldconfig
	if (( $? ))
	then
		echo "error: failed to update dynamic loader cache. Please reboot your"
		echo "system after installation completes"
	else
		echo "Successfully updated dynamic loader cache"
	fi
fi

###########################################################################
# Done

echo "Successfully installed FTDI Driver"

