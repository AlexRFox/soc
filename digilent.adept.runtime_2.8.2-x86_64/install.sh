#!/bin/bash

###########################################################################
#                                                                         #
#  install.sh -- Linux Adept Runtime Installation Script                  #
#                                                                         #
###########################################################################
#  Author: MTA                                                            #
#  Copyright 2011 Digilent Inc.                                           #
###########################################################################
#  File Description:                                                      #
#                                                                         #
#  This is a bash shell script that can be used to install the Adept      #
#  Runtime on a target system. This script should be executed from the    #
#  root of the Runtime release directory. Please note that you need to    #
#  run the script with superuser privileges for the installation to       #
#  succeed.                                                               #
#                                                                         #
#  Command Line Arguments:                                                #
#                                                                         #
#  This script accepts several command line arguments in the the form of  #
#  "key=value" pairs. Below is a list of the supported arguments along    #
#  with a description of what each argument is used for.                  #
#                                                                         #
#  "datapath=pathname" - used to specify the path of the directory where  #
#  data files should be installed during installation. If this path is    #
#  specified and an existing installation of the Runtime isn't present    #
#  then the data files associated with the Runtime will be installed in   #
#  the directory specified by "pathname". If an existing Runtime          #
#  installation is present then the data files will be placed in the data #
#  path of the exisiting installation. If the datapath isn't specified    #
#  and an existing installation isn't found then the data files will be   #
#  placed in "/usr/local/share/digilent/data". If the script isn't        #
#  running in silent mode then the user is prompted and asked if the      #
#  default location should be used. He or she can then choose to specify  #
#  an alternate location.                                                 #
#                                                                         #
#  "libpath=pathname" - used to specify the path of the directory where   #
#  shared libraries should be placed during installation. If this path is #
#  specified and an existing installation of the Runtime isn't present    #
#  then the shared libraries will be installed in the directory specified #
#  by "pathname". If an existing installation of the Runtime is detected  #
#  then the shared libraries will be installed in the same directory as   #
#  the libraries from the existing installation. If "libpath" isn't       #
#  specified then the system configuration will be used to determine a    #
#  suitable default path and the default path will be used for            #
#  installation. If the script isn't running in silent mode then the user #
#  will be prompted to accept the default path or specify a different     #
#  path.                                                                  #
#                                                                         #
#  "sbinpath=pathname" - used to specify the path of the directory where  #
#  system binaries should be placed during installation. If this path is  #
#  specified and an existing installation of the Runtime isn't present    #
#  then the system binaries used by the Runtime will be installed in the  #
#  directory specified by "pathname". If an existing installation of the  #
#  Runtime is detected then the system binaries will be placed in the     #
#  same directory as the system binaries of the existing installation. If #
#  "sbinpath" isn't specified then the system configuration is used to    #
#  determine a suitable default path and the default path is used for     #
#  installation. If the script isn't running in silent mode then the user #
#  will be prompted to accept the default path or specify a different     #
#  one.                                                                   #
#                                                                         #
#  "silent=1" - used to run the script in silent mode. If the script is   #
#  running in silent mode then the user will NOT be prompted for          #
#  confirmation of destination directories nor will he or she be asked    #
#  for confirmation before symbolic links are updated for existing shared #
#  library installations.                                                 #
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
#  7 - failed to remove a file                                            #
#  8 - error writing to a file                                            #
#  22 - installer run by a user without superuser (root) privileges       #
#  40 - no suitable destination dir found for shared library installation #
#  41 - no suitable destination dir found for system binary installation  #
#  42 - no suitable destination dir found for data file installation      #
#                                                                         #
###########################################################################
#  Revision History:                                                      #
#                                                                         #
#  03/05/2011(MTA): created                                               #
#  03/06/2011(MTA): added additional features and updated comments        #
#  03/08/2011(MTA): added support for systems using hotplug               #
#  04/07/2011(MTA): updated to use multiple default installation          #
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
let fExistingDataInstall=0
let fExistingLibInstall=0
let fExistingSbinInstall=0
let fNotSilent=1
let fUseUdev=0
szDataDst=""
szLibDst=""
szSbinDst=""
szSilent="silent=0"

# Go through the list of command line arguments and parse them.
for szArg in "$@"
do
	case "${szArg}" in
		datapath=* )
			szDataDst=$(echo ${szArg} | sed 's/datapath=//');;
		libpath=* )
			szLibDst=$(echo ${szArg} | sed 's/libpath=//');;
		sbinpath=* )
			szSbinDst=$(echo ${szArg} | sed 's/sbinpath=//');;
		silent=1 )
			szSilent="silent=1";
			let fNotSilent=0;;
	esac
done

echo "Adept Runtime Installer"

# Make sure the user has superuser (root) privileges.
if (( $EUID != 0 ))
then
	echo "error: superuser (root) privileges are required for installation"
	exit 22
fi

# Determine if the system is using UDEV. If the system isn't using UDEV
# then we will assume that it uses hotplug.
szKernVer=$(uname -r)
szVmjr=$(echo ${szKernVer} | cut -f 1 -d ".")
szVmin=$(echo ${szKernVer} | cut -f 2 -d ".")
szVmic=$(echo ${szKernVer} | cut -f 3 -d "." | cut -f 1 -d "-")
cprocUdev=$(ps -e | grep -i -c udevd)

if [ "${szVmjr}" = "2" ]
then
    if [ "${szVmin}" = "6" ]
    then
        if (( ${szVmic} >= 13 ))
        then
        	if (( $cprocUdev ))
        	then
                let fUseUdev=1
            fi
        fi
    fi
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
	
	rgszDestDir[2]="/opt"
	rgszDestDirParent[2]=""
	
	for (( i=0; i < ${#rgszDestDir[*]}; i++ ))
	do
		szDestPath=$(GetDestPath "${rgszDestDir[${i}]}" "${rgszDestDirParent[${i}]}" "digilent/adept")
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

echo "Installing runtime libraries....."

# Go through the list of shared libraries and install them.
let i=0
let iszLibDst=0
let clibInst=0
while (( $i < ${#rgszLibs[*]} ))
do
	if [ ! -L "${szLibSrc}/${rgszLibs[${i}]}" ]
	then
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
				# installed on the system. We must set fExistingLibInstall
				# so that we don't overwrite any existing dynamic loader
				# configuration.
			
				let fExistingLibInstall=1
				let i+=1
				let clibInst+=1
			elif (( $ret == 3 || $ret == 4 )) && (( $clibInst == 0 ))
			then
				# Failed to create the destination directory that the
				# library is being installed to. If we previously
				# determined an alternate installation location then we
				# will attempt to use that location during the next pass
				# through the loop.
			
				let iszLibDst+=1
			
				if (( $iszLibDst == $cszLibDst ))
				then
					echo "error: failed to install ${szLibSrc}/${rgszLibs[${i}]}"
					exit $ret
				fi
			else
				# An unknown error occured. We can't continue the
				# installation.
			
				echo "error: failed to install ${szLibSrc}/${rgszLibs[${i}]}"
				exit $ret
			fi
		else
			# No error occured so proceed to install the next library
			# during the next pass through the loop.
		
			let i+=1
			let clibInst+=1
		fi
	else
		let i+=1
	fi
done

echo "Successfully installed runtime libraries."

###########################################################################
# Determine the installation path for system binaries.

# Determine the source directory for system binaries based on the machine
# type.
if [ "${szMname}" = "x86_64" ]
then
	szSbinSrc="bin64"
else
	szSbinSrc="bin"
fi

# Make sure the expected system binaries exist in the source directory.
if [ ! -f "${szSbinSrc}/dftdrvdtch" ]
then
	echo "error: failed to find system binary \"${szSbinSrc}/dftdrvdtch\""
	exit 6
fi

# Check for an existing installation. The way this check is performed
# depends on whether or not the system uses UDEV or hotplug.
if (( $fUseUdev ))
then
	# The system is running UDEV. If the file 
	# "/etc/udev/rules.d/52-digilent-usb.rules" exists then there may be an
	# existing installation.
	if [ -f "/etc/udev/rules.d/52-digilent-usb.rules" ]
	then
		# Check to see if the rules file contains a path for an executable.
		szTemp=($(cat /etc/udev/rules.d/52-digilent-usb.rules | grep RUN+= | sed 's/.*RUN+="//'))
		if [ -n "${szTemp}" ]
		then
			szSbinDst="$(dirname ${szTemp})"
			let fExistingSbinInstall=1
		fi
	fi
else
	# The system should be running hotplug. If the file 
	# "/etc/hotplug/usb/digilentusb" exists then tthere may be an existing
	# installation.
	if [ -f "/etc/hotplug/usb/digilentusb" ]
	then
		# Check to see if the hotplug script contains a path for an
		# executable.
		szTemp=$(cat /etc/hotplug/usb/digilentusb | grep szFTDTCH= | sed "s/szFTDTCH='//" | sed "s/'*$//")
		if [ -n "${szTemp}" ]
		then
			szSbinDst="$(dirname ${szTemp})"
			let fExistingSbinInstall=1
		fi
	fi
fi

rgszSbinDst=()
let cszSbinDst=0

# Has the user specified a system binaries destination directory or was a
# previous installation detected?
if [ -n "${szSbinDst}" ]
then
	# Yes so we will only attempt to install system binaries in that
	# directory.
	
	rgszSbinDst[0]="${szSbinDst}"
	let cszSbinDst=1
else
	# No so we will generate a list of possible installation directories
	# based on system configuration.
	
	rgszDestDir=()
	rgszDestDirParent=()

	rgszDestDir[0]="/usr/local/sbin"
	rgszDestDirParent[0]="/usr/local"

	rgszDestDir[1]="/usr/sbin"
	rgszDestDirParent[1]=""

	rgszDestDir[2]="/sbin"
	rgszDestDirParent[2]=""
	
	for (( i=0; i < ${#rgszDestDir[*]}; i++ ))
	do
		szDestPath=$(GetDestPath "${rgszDestDir[${i}]}" "${rgszDestDirParent[${i}]}" "")
		if [ -n "${szDestPath}" ]
		then
			rgszSbinDst[${cszSbinDst}]="${szDestPath}"
			let cszSbinDst+=1
		fi
	done
fi

if [ -z "${szSbinDst}" ]
then
	# No destination directory was specified via command line argument and
	# no previous installation was found. If we aren't running in silent
	# mode then we should prompt the user and allow he or she to confirm
	# the default installation directory or specify another one.
	
	if (( $fNotSilent ))
	then
		if (( $cszSbinDst ))
		then
			# We previously determined at least one suitable default
			# installation directory. Allow the user to confirm this
			# directory or to specify another one.
			
			read -p "In which directory should system binaries be installed? [${rgszSbinDst[0]}] " szTemp
			if [ -n "${szTemp}" ]
			then
				# The user specified an installation directory so we will
				# only attempt to install system binaries in that
				# directory.
				
				rgszSbinDst[0]="${szTemp}"
				let cszSbinDst=1
			fi
		else
			# We didn't previously determine any suitable default
			# installation directories. The user must specify a directory
			# before we can continue.
			
			szTemp=""
			while [ -z "${szTemp}" ]
			do
				read -p "In which directory should system binaries be installed? " szTemp
			done
			
			rgszSbinDst[0]="${szTemp}"
			let cszSbinDst=1
		fi
	fi
fi

if (( ! $cszSbinDst ))
then
	echo "error: unable to determine suitable installation directory for system binaries"
	exit 41
fi

###########################################################################
# Install system binaries.

echo "Installing system binaries....."

let fSbinInstalled=0
let	iszSbinDst=0
while (( ! $fSbinInstalled ))
do
	# Create the destination directory if it doesn't exist.
	mkdir -p "${rgszSbinDst[${iszSbinDst}]}"
	let ret=$?
	
	if (( ! $ret ))
	then
		# We successfully created the destination directory or it already
		# existed. Install the binaries.
		
		cp -f "${szSbinSrc}/dftdrvdtch" "${rgszSbinDst[${iszSbinDst}]}" && \
		chmod 755 "${rgszSbinDst[${iszSbinDst}]}/dftdrvdtch"
		let ret=$?
	fi
	
	if (( $ret ))
	then
		# Failed to create destination directory or copy the files to that
		# location. We will assume that this failure is due to the
		# destination directory existing on a read-only file system. If we
		# previously determined an alternate installation location then we
		# will attempt to use that location during the next pass through
		# the loop.
		
		let iszSbinDst+=1
		if (( $iszSbinDst == $cszSbinDst ))
		then
			echo "error: failed to install \"${szSbinSrc}/dftdrvdtch\""
			exit 4
		fi
	else
		let fSbinInstalled=1
	fi
done

echo "    installed \"${rgszSbinDst[${iszSbinDst}]}/dftdrvdtch\""

echo "Successfully installed system binaries in \"${rgszSbinDst[${iszSbinDst}]}\"."

###########################################################################
# Determine the installation path for data files.

# If the file "/etc/digilent-adept.conf" exists then it's likely that the
# system contains a previous install of the Runtime. We should check to
# see if this file contains "DigilentDataPath=" and if it does then use
# that path when installing data files.
if [ -f "/etc/digilent-adept.conf" ]
then
	szTemp=$(cat /etc/digilent-adept.conf | grep DigilentDataPath | sed 's/DigilentDataPath=//')
	if [ -n "szTemp" ]
	then
		szDataDst="${szTemp}"
		let fExistingDataInstall=1
	fi
fi

rgszDataDst=()
let cszDataDst=0

# Was a destination data directory specified via command line argument or
# determined from a configuration file associated with an existing install?
if [ -n "${szDataDst}" ]
then
	# Yes so we will only attempt to install the data files in this
	# directory.
	
	rgszDataDst[0]="${szDataDst}"
	let cszDataDst=1
else
	# No so we will generate a list of possible installation directories.
	
	rgszDestDir=()
	rgszDestDirParent=()

	rgszDestDir[0]="/usr/local/share"
	rgszDestDirParent[0]="/usr/local"

	rgszDestDir[1]="/usr/share"
	rgszDestDirParent[1]=""

	rgszDestDir[2]="/opt"
	rgszDestDirParent[2]=""
	
	for (( i=0; i < ${#rgszDestDir[*]}; i++ ))
	do
		szDestPath=$(GetDestPath "${rgszDestDir[${i}]}" "${rgszDestDirParent[${i}]}" "digilent/data")
		if [ -n "${szDestPath}" ]
		then
			rgszDataDst[${cszDataDst}]="${szDestPath}"
			let cszDataDst+=1
		fi
	done
fi

if [ -z "${szDataDst}" ]
then
	# No destination directory was specified via command line and a
	# previous install wasn't found.
	
	if (( $fNotSilent ))
	then
		if (( $cszDataDst ))
		then
			# We aren't running in silent mode and we previously determined
			# at least one suitable default installation directory. Allow
			# the user to confirm this directory or specify another one.
			
			read -p "In which directory should data files be installed? [${rgszDataDst[0]}] " szTemp
			if [ -n "${szTemp}" ]
			then
				# The user specified an alternate data directory. We will
				# only attempt to install files into this directory.
				
				rgszDataDst[0]="${szTemp}"
				let cszDataDst=1
			fi
		else
			# We aren't running in silent mode and we didn't previously
			# determine any suitable default installation directories. We
			# can't continue until the user specifies a data directory.
			
			szTemp=""
			while [ -n "${szTemp}" ]
			do
				read -p "In which directory should data files be installed? " szTemp
			done
			
			rgszDataDst[0]="${szTemp}"
			let cszDataDst=1
		fi
	fi
fi

if (( ! $cszDataDst ))
then
	echo "error: unable to determine suitable installation directory for data files"
	exit 42
fi

###########################################################################
# Install firmware images.

echo "Installing firmware images....."

let iszDataDst=0
szFwiSrc="data/firmware"
szFwiDst="${rgszDataDst[${iszDataDst}]%/}/firmware"

let fFwiInst=0
while (( ! $fFwiInst ))
do
	# Attempt to install the firmware images.
	mkdir -p ${szFwiDst}              && \
	cp -f ${szFwiSrc}/*.HEX ${szFwiDst} && \
	cp -f ${szFwiSrc}/*.so ${szFwiDst} && \
	chmod 644 ${szFwiDst}/*.HEX && \
	chmod 755 ${szFwiDst}/*.so
	
	let ret=$?
	if (( $ret ))
	then
		# Failed to install the firmware images. Assume this is due to
		# mkdir or cp failing due to the destination directory being on a
		# read-only file system. Increment iszDataDst so that we will
		# attempt to install the firmware images into another directory
		# during the next pass through the loop.
		
		let iszDataDst+=1
		if (( $iszDataDst == $cszDataDst ))
		then
			# There are no more known directories to attempt to install
			# data files into.
			
			echo "error: failed to install firmware images"
			exit 4
		fi
		
		szFwiDst="${rgszDataDst[${iszDataDst}]%/}/firmware"
	else
		# The firmware images were successfully installed.
		
		let fFwiInst=1
	fi
done

echo "Successfully installed firmware images in \"${szFwiDst}\"."

###########################################################################
# Install JTSC Device List file. Please note that we will only make a
# single attempt to install any of the remaining data files even if
# rgszDataDst contains additional alternate directories because we already
# installed the firmware images in the currently selected directory.

szJdlSrc="data/jtscdvclist.txt"
szJdlDst="${rgszDataDst[${iszDataDst}]%/}"

echo "Installing JTSC device list....."

mkdir -p ${szJdlDst}        && \
cp -f ${szJdlSrc} ${szJdlDst} && \
chmod 644 ${szJdlDst}/$(basename ${szJdlSrc})
if (( $? )); then echo "error: failed to install JTSC device list"; exit 4; fi

echo "Successfully installed JTSC device list \"${szJdlDst}/$(basename ${szJdlSrc})\"."

###########################################################################
# Install CoolRunner support files.

szCrsfSrc="data/xpla3"
szCrsfDst="${rgszDataDst[${iszDataDst}]%/}/xpla3"

echo "Installing CoolRunner support files....."

mkdir -p ${szCrsfDst}               && \
cp -f ${szCrsfSrc}/*.map ${szCrsfDst} && \
chmod 644 ${szCrsfDst}/*.map
if (( $? )); then echo "error: failed to install CoolRunner support files"; exit 4; fi

echo "Successfully installed CoolRunner support files in \"${szCrsfDst}\"."

###########################################################################
# Install CoolRunner 2 support files.

szCr2sfSrc="data/xbr"
szCr2sfDst="${rgszDataDst[${iszDataDst}]%/}/xbr"

echo "Installing CoolRunner 2 support files....."

mkdir -p ${szCr2sfDst}               && \
cp -f ${szCr2sfSrc}/*.map ${szCr2sfDst} && \
chmod 644 ${szCr2sfDst}/*.map
if (( $? )); then echo "error: failed to install CoolRunner 2 support files"; exit 4; fi

echo "Successfully installed CoolRunner 2 support files in \"${szCr2sfDst}\"."

###########################################################################
# Generate Adept Runtime Configuration File.

# The Adept Runtime configuration is used to configure various aspects of
# the Adept Runtime. Most notably the configuration file contains a key
# value pair that specifies the Digilent Data path. The Digilent Data Path
# is the path to the directory that contains the firmware images directory
# and the JTSC device list file. The configuration file must be generated
# each time the script is run because the user is allowed to change the
# Digilent Data Path.

# Specify filenames for the adept configuration template file
# and a temporary file that will be used to modify the template so that
# it contains the appropriate path.
szAdcSrc="digilent-adept.conf"
szAdcTmp="digilent-adept.conf.tmp"

# Check to see if the adept configuration template file exists.
if [ -e "${szAdcSrc}" ]
then
	
	# The template file exists. It may already contain a path for
	# the digilent data directory. This path must be removed before we
	# can generate the final adept configuration file.
	
	# If the temporary file exists we must remove it before continuing.
	if [ -e "${szAdcTmp}" ]
	then
		rm "${szAdcTmp}"
		if (( $? ))
		then
			echo "error: failed to remove temporary file ${szAdcTmp}"
			exit 7
		fi
	fi
	
	# Copy the contents of the adept configuration template file to the
	# temporary file.
	while read szLine
	do
		# Only copy lines that don't contain the digilent data path.
		szTemp="$(echo "${szLine}" | grep "DigilentDataPath")"
		if [ -z "${szTemp}" ]
		then
			echo "${szLine}" >> "${szAdcTmp}"
			if (( $? ))
			then
				echo "error: failed to write adept runtime configuration to ${szAdcTmp}"
				exit 8
			fi
		fi
	done < "${szAdcSrc}"
fi

# Now append the DigilentDataPath key to the temporary adept configuration
# file with the appropriate path specified as the value.

echo "DigilentDataPath=${rgszDataDst[${iszDataDst}]}" >> "${szAdcTmp}"
if (( $? ))
then
	echo "error: failed to write adept configuration to ${szAdcTmp}"
	exit 8
fi

# If we get this far the temporary adept configuration file should exist.
# We need to overwrite the adept configuration template file with the real
# configuration file and then remove the temporary file.
if [ -e "${szAdcTmp}" ]
then
	
	# Overwrite template configuration file with the real dynamic loader
	# configuration file.
	cp -f "${szAdcTmp}" "${szAdcSrc}"
	if (( $? ))
	then
		echo "error: failed to create adept configuration file ${szAdcSrc}"
		exit 8
	fi
	
	# Remove the temporary file now that we no longer need it.
	rm -f "${szAdcTmp}"
else
	echo "error: failed to adept create configuration file"
	exit 6
fi

###########################################################################
# Install Adept Runtime configuration file.

szAdcDst="/etc"

echo "Installing Adept Runtime configuration....."

cp -f ${szAdcSrc} ${szAdcDst} && \
chmod 644 ${szAdcDst}/$(basename ${szAdcSrc})

if (( $? ))
then
	
	# An error occured while installing the Adept Runtime configuration
	# file.
	
	echo "error: failed to install Adept Runtime configuration file. Please "
	echo "see the documentation for instructions on manual installation."
else
	
	# We successfully installed the Adept Runtime configuration file.
	
	echo "Successfully installed Adept Runtime configuration \"${szAdcDst}/$(basename ${szAdcSrc})\"."
fi

###########################################################################
# Generate UDEV USB rules.

if (( $fUseUdev ))
then
	# The UDEV USB rules file is used to set the permissions that are
	# applied to each Digilent device that is attached to the system. It is
	# also used to instruct UDEV to call an external application that
	# detaches (unloads) any kernel driver attached to a Digilent device
	# that contains an FTDI chip. This is necessary because the Adept
	# Runtime can't access the device via libftd2xx while a kernel driver
	# is attached.

	# The dftdrvdtch application is system binary used to detach kernel
	# drivers from FTDI devices that contain the Digilent manufacturer
	# string. We must generate the UDEV USB rules file from a template file
	# because the user is allowed to change the path used for installing
	# system binaries.

	szUurSrc="52-digilent-usb.rules"
	szUurTmp="52-digilent-usb.rules.tmp"
	szUurDst="/etc/udev/rules.d/52-digilent-usb.rules"

	# If the temporary file exists we must remove it before continuing.
	if [ -e "${szUurTmp}" ]
	then
		rm "${szUurTmp}"
		if (( $? ))
		then
			echo "error: failed to remove temporary file ${szUurTmp}"
			exit 7
		fi
	fi

	# Copy the contents of the template file to the temporary file, making
	# replacements as necessary.
	while read szLine
	do
		szTemp="$(echo "${szLine}" | grep "ACTION==\"add\", SYSFS{idVendor}==\"0403\", SYSFS{manufacturer}==\"Digilent\",")"
		if [ -z "${szTemp}" ]
		then
			echo "${szLine}" >> "${szUurTmp}"
		else
			szTemp="${rgszSbinDst[${iszSbinDst}]%/}/dftdrvdtch"
			echo "ACTION==\"add\", SYSFS{idVendor}==\"0403\", SYSFS{manufacturer}==\"Digilent\", MODE=\"666\", RUN+=\"${szTemp} %s{busnum} %s{devnum}\"" >> "${szUurTmp}"
		fi
	
		if (( $? ))
		then
			echo "error: failed to write UDEV rules to ${szUurTmp}"
			exit 8
		fi
	done < "${szUurSrc}"

	# Now overwrite the template UDEV USB rules file with the real UDEV USB
	# rules file.
	cp -f "${szUurTmp}" "${szUurSrc}"
	if (( $? ))
	then
		echo "error: failed to create UDEV USB rules file ${szUurSrc}"
		exit 8
	fi

	# The UDEV USB rules need to be modified if the target system is
	# running Red Hat Enterprise Linux 5 (RHEL5) or CentOS 5. If the target
	# system is running any other distribution then we should install the
	# rules as is.

	# If this is a Redhat or CentOS distribution then the file
	# "/etc/redhat-release" should exist and should contain the
	# version number.
	if [ -f "/etc/redhat-release" ]
	then
		# We expect "/etc/redhat-release" to be a text file containing a
		# single line of text that is typically formatted similar to 
		# "CentOS release 5.5 (Final)" for CentOS distributions and similar
		# to "Red Hat Enterprise Linux Server release 5 (Tikanga)" for Red
		# Hat distributions.
	
		# We will attempt to parse the version number from
		# "/etc/redhat-release" and store it as a string in szVersion. We
		# will then attempt to parse that string into a major version
		# number and store it in szVerMjr. Ensure that both of these
		# strings have initial length zero in case we fail to parse either
		# one of them.
		szVersion=""
		szVerMjr=""
	
		# Create an array of strings based on the contents of
		# "/etc/redhat-release". Please note that the following assignment
		# will parse based on whitespace. If "/etc/redhat-release" contains
		# the string "CentOS release 5.5 (Final)" then rgszRelease will be
		# formatted as follows:
		#   rgszRelease[0] = "CentOS"
		#   rgszRelease[1] = "release"
		#   rgszRelease[2] = "5.5"
		#   rgszRelease[3] = "(Final)"
		#
		rgszRelease=($(cat /etc/redhat-release))
	
		# Go through the array and look for the version number.
		for (( i=0; i < ${#rgszRelease[*]} ; i++ ))
		do
			if [[ "${rgszRelease[${i}]}" == "release" ]]
			then
				# The version number is expected to follow "release". The
				# next entry in the array should be the version number. If
				# this entry exists then we will assume that it is the
				# version number and copy it to szVersion.
				let i+=1
				if [ $i -lt ${#rgszRelease[*]} ]
				then
					szVersion="${rgszRelease[${i}]}"
					break
				fi
			fi
		done
	
		# If we managed to parse the version number then szVersion should
		# have a non-zero length.
		if [ -n "${szVersion}" ]
		then
			# The version number may be formatted as "x" or "x.y" where x
			# is an integer representing the major version and y is an
			# integer representing the minor version. We can parse the
			# version number string into it's components by replacing the
			# '.' with a space and storing it in an array, which removes
			# the whitespace.
			rgszVer=(${szVersion//./ })
			if (( ${#rgszVer[*]} != 0 ))
			then
				szVerMjr="${rgszVer[0]}"
			fi
		fi
	
		# If we managed to parse the major version number then szVerMjr
		# should have a non-zero length. We should take an appropriate
		# action based on the value of szVerMajor.
		if [ "${szVerMjr}" = "5" ]
		then
			# The system is running RHEL5 or CentOS 5. These systems
			# require additional rules to be added to the UDEV USB rules
			# file in order to work around what appears to be a bug in the
			# kernel used by these distributions. Append the additional
			# rules to the temporary file.
			echo "SUBSYSTEM==\"usb_endpoint\", ACTION==\"add\", OPTIONS=\"ignore_device\"" >> "${szUurTmp}" && \
			echo "SUBSYSTEM==\"usb_endpoint\", ACTION==\"remove\", OPTIONS=\"ignore_device\"" >> "${szUurTmp}"
		
			if (( $? ))
			then
				# Failed to create the new UDEV USB rules file.
			
				echo "error: failed to write UDEV USB rules to \"${szUurTmp}\"."
				echo "The UDEV USB rules will not be installed. Please "
				echo "consult the documentation for instructions on how to "
				echo "manually create and install the UDEV USB rules."
			
				# Set the paths for szUurSrc to an empty string to ensure
				# that no rules are installed.
				szUurSrc=""
			else
				# Successfully created the temporary USB UDEV rules file
				# with the additional rules. Set the szUurSrc to be the
				# name of the temporary rules file so that it is installed
				# in place of the standard USB UDEV rules.
				szUurSrc="${szUurTmp}"
			fi
		fi
	fi
fi

###########################################################################
# Install UDEV USB rules.

if (( $fUseUdev ))
then
	# If the UDEV USB rules are to be installed then szUurSrc will be a
	# string with non-zero length.
	if [ -n "${szUurSrc}" ]
	then
		echo "Installing USB UDEV rules....."
	
		cp -f ${szUurSrc} ${szUurDst} && \
		chmod 644 ${szUurDst}
	
		if (( $? ))
		then
			# An error occured during installation.
		
			echo "error: failed to install UDEV USB rules. Please see the documentation for instructions on manual installation."
		
			# Set the szUurSrc string to be an empty string so that we
			# don't attempt to execute the command to reload the rules
			# during post installation.
			szUurSrc=""
		else
			# We successfully installed the rules.
		
			echo "Successfully installed USB UDEV rules \"${szUurDst}\"."
		fi
	fi

	# If a temporary UDEV USB rules file was created then szUurTmp will
	# have a nonzero length. We should remove this file now since it's no
	# longer needed.
	if [ -n "${szUurTmp}" ]
	then
		if [ -e "${szUurTmp}" ]; then rm -f "${szUurTmp}"; fi
	fi
fi

###########################################################################
# Generate hotplug script.

if (( ! $fUseUdev ))
then
	# The hotplug script is used to set the permissions that are applied to
	# each Digilent device that is attached to the system. It is also used
	# to call an external application that detaches (unloads) any kernel
	# driver attached to a Digilent device that contains an FTDI chip. This
	# is necessary because the Adept Runtime can't access the device via
	# libftd2xx while a kernel driver is attached.

	# The dftdrvdtch application is system binary used to detach kernel
	# drivers from FTDI devices that contain the Digilent manufacturer
	# string. We must generate the hotplug script from a template because
	# the user is allowed to change the path used for installing system
	# binaries.	
	szHpsSrc="digilentusb"
	szHpsTmp="digilentusb.tmp"
	szHpsDst="/etc/hotplug/usb/digilentusb"
	
	# Make sure the template file is present.
	if [ ! -f "${szHpsSrc}" ]
	then
		echo "error: couldn't find hotplug script \"${szHpsSrc}\""
		exit 6
	fi
	
	# If the temporary file exists we must remove it before continuing.
	if [ -e "${szHpsTmp}" ]
	then
		rm -f "${szHpsTmp}"
		if (( $? ))
		then
			echo "error: failed to remove temporary file \"${szHpsTmp}\""
			exit 7
		fi
	fi
	
	# Generate the new hotplug script, replacing the path specified for
	# dftdrvdtch with the path to where the executable was installed.
	szTemp="${rgszSbinDst[${iszSbinDst}]%/}/dftdrvdtch"
	cat "${szHpsSrc}" | sed "s|szFTDTCH='.*'|szFTDTCH=\'${szTemp}\'|" > "${szHpsTmp}"
	if (( $? ))
	then
		echo "error: failed to generate hotplug script \"${szHpsTmp}\""
		exit 8
	fi
	
	# Replace the original hotplug script with the new one.
	cp -f "${szHpsTmp}" "${szHpsSrc}"
	if (( $? ))
	then
		echo "error: failed to create hotplug script \"${szHpsSrc}\""
		exit 8
	fi
	
	# Remove the temporary hotplug script since we no longer need it.
	rm -f "${szHpsTmp}"
fi

###########################################################################
# Install the hotplug script.

if (( ! $fUseUdev ))
then
	# Install the hotplug script.
	echo "Installing hotplug script....."
	
	cp -f ${szHpsSrc} ${szHpsDst} && \
	chmod 755 ${szHpsDst}
	if (( $? ))
	then
		echo "error: failed to install hotplug script \"${szHpsDst}\""
		exit 4
	fi
	
	echo "Successfully installed hotplug script \"${szHpsDst}\"."
	
	# Update the hotplug usb usermap as necessary.
	echo "Updating hotplug usermap....."
	
	fIsInUsermap=$(cat /etc/hotplug/usb.usermap | grep digilentusb | grep -i -c 0x1443)
	if (( $fIsInUsermap ))
	then
		echo "    Vendor ID \"0x1443\" is already in the usermap."
	else
		echo "    Adding Vendor ID \"0x1443\" to usermap."
		echo "# Digilent USB Devices using Digilent Vendor ID" >> "/etc/hotplug/usb.usermap"
		echo "digilentusb          0x0001      0x1443   0x0000    0x0000       0x0000      0x00         0x00            0x00            0x00            0x00               0x00               0x00000000" >> "/etc/hotplug/usb.usermap"
	fi
	
	fIsInUsermap=$(cat /etc/hotplug/usb.usermap | grep digilentusb | grep -i -c 0x0403)
	if (( $fIsInUsermap ))
	then
		echo "    Vendor ID \"0x0403\" is already in the usermap."
	else
		echo "    Adding Vendor ID \"0x0403\" to usermap."
		echo "# Digilent USB Devices using FTDI Vendor ID" >> "/etc/hotplug/usb.usermap"
		echo "digilentusb          0x0001      0x0403   0x0000    0x0000       0x0000      0x00         0x00            0x00            0x00            0x00               0x00               0x00000000" >> "/etc/hotplug/usb.usermap"
	fi
	
	echo "Successfully updated hotplug usermap."
fi

###########################################################################
# Generate Dynamic Loader Configuration File.

# The Dynamic Loader Configuration file is used to tell the dynamic loader
# where to search for the Adept runtime libraries. This file must be
# generated from a template file each time the installation script is run
# because the user is allowed to change the path that the runtime libraries
# are installed under.

# Specify filenames for the dynamic loader configuration template file
# and a temporary file that will be used to modify the template so that it
# contains the appropriate paths.
szDlcSrc="digilent-adept-libraries.conf"
szDlcTmp="digilent-adept-libraries.conf.tmp"

# We only need to generate a config file if an existing installation of
# the shared library wasn't found during installation. If an existing
# installation was found then the dynamic loader already knows how to find
# the library in the directory that it's installed in.
if (( ! $fExistingLibInstall ))
then
	# Check to see if the dynamic loader configuration template file
	# exists.
	if [ -e "${szDlcSrc}" ]
	then
	
		# The template file exists. It may already contain paths for
		# runtime libraries. These paths must be removed before we can
		# generate the final dynamic loader configuration file.
	
		# If the temporary file exists we must remove it before continuing.
		if [ -e "${szDlcTmp}" ]
		then
			rm "${szDlcTmp}"
			if (( $? ))
			then
				echo "error: failed to remove temporary file ${szDlcTmp}"
				exit 7
			fi
		fi
	
		# Copy the contents of the dynamic loader configuration template
		# file to the temporary file.
		while read szLine
		do
			# We only want to copy lines that actually contain text.
			if [ -n "${szLine}" ]
			then
				# Only copy lines that don't contain paths to runtime
				# libraries.
				szTemp="$(echo "${szLine}" | grep "# Path to [36][24]-bit runtime libraries.")"
				if [ -z "${szTemp}" ]
				then
					echo "${szLine}" >> "${szDlcTmp}"
					if (( $? ))
					then
						echo "error: failed to write dynamic loader configuration "
						echo "to ${szDlcTmp}"
						exit 8
					fi
				fi
			fi
		done < "${szDlcSrc}"
	fi

	if [ "${szMname}" = "x86_64" ]
	then
		# 64-bit shared libraries were installed. Write the path used for
		# these libraries to the temporary dynamic loader configuration
		# file.
		echo "" >> "${szDlcTmp}" && \
		echo "${rgszLibDst[${iszLibDst}]} # Path to 64-bit runtime libraries." >> "${szDlcTmp}"
	
		if (( $? ))
		then
			echo "error: failed to write dynamic loader configuration to "
			echo "${szDlcTmp}"
			exit 8
		fi
	else
		# 32-bit shared libraries were installed. Write the path used for
		# these libraries to the temporary dynamic loader configuration
		# file.
		echo "" >> "${szDlcTmp}" && \
		echo "${rgszLibDst[${iszLibDst}]} # Path to 32-bit runtime libraries." >> "${szDlcTmp}"
	
		if (( $? ))
		then
			echo "error: failed to write dynamic loader configuration to ${szDlcTmp}"
			exit 8
		fi
	fi

	# If we get this far the temporary dynamic loader configuration file
	# should exist. We need to overwrite the dynamic loader configuration
	# template file with the real configuration file and then remove the
	# temporary file.
	if [ -e "${szDlcTmp}" ]
	then
	
		# Overwrite template configuration file with the real dynamic
		# loader configuration file.
		cp -f "${szDlcTmp}" "${szDlcSrc}"
		if (( $? ))
		then
			echo "error: failed to create dynamic loader configuration file ${szDlcTmp}"
			exit 8
		fi
	
		# Remove the temporary file now that we no longer need it.
		rm -f "${szDlcTmp}"
	else
		echo "error: failed to generate dynamic loader configuration file"
		exit 8
	fi
fi

###########################################################################
# Install dynamic loader configuration.

szDlcDst="/etc/ld.so.conf.d"

# We only need to install a dynamic loader configuration file if an
# existing installation of the shared libraries wasn't found during
# installation.
if (( ! $fExistingLibInstall ))
then
	echo "Installing dynamic loader configuration....."

	cp -f "${szDlcSrc}" "${szDlcDst}" && \
	chmod 644 "${szDlcDst}/$(basename ${szDlcSrc})"
	if (( $? ))
	then
		echo "error: failed to install dynamic loader configuration. Please see the documentation for instructions on manual installation."
	
		# Set szDlcSrc to an empty string so that we don't bother updating
		# the dynamic loader cache during post installation.
		szDlcSrc=""
	else
		echo "Successfully installed dynamic loader configuration \"${szDlcDst}/$(basename ${szDlcSrc})\"."
	fi
fi

###########################################################################
# Post installation tasks.

if (( $fUseUdev ))
then
	# We may have installed a file containing rules that tell UDEV how to
	# create entries for Digilent USB devices in "/dev/bus/usb/busXXX". We
	# must tell UDEV to reload the rules that its using so that the new
	# rules are used when Digilent devices are attached to the system. 
	# Please note that this is only necessary to avoid restarting the
	# system. Each time the system starts up the UDEV rules are reloaded.
	# If we installed a UDEV USB rules file then szUurSrc will have a
	# non-zero length.
	if [ -n "${szUurSrc}" ]
	then
		# We installed a UDEV USB rules file. Take the necessary actions.

		# Depending on the distribution "/sbin/udevcontrol" may not be
		# available. If that's the case then most likely the user is
		# running a distribution that has replaced "/sbin/udevcontrol" with
		# "/sbin/udevadm control". We must determine which application is
		# available and specify the appropriate command prior to executing
		# it.
		szUdrrcmd=""
		if [ -e "/sbin/udevcontrol" ]
		then
			szUdrrcmd="/sbin/udevcontrol reload_rules"
		elif [ -e "/sbin/udevadm" ]
		then
			szUdrrcmd="/sbin/udevadm control --reload-rules"
		fi

		# Execute the appropriate command to reload the UDEV rules.
		if [ -n "${szUdrrcmd}" ]
		then
			# We determine the correct command to use. Now attempt to
			# reload the rules.
			${szUdrrcmd}
			if (( $? ))
			then
				echo "error: failed to reload UDEV reules. Please reboot your "
				echo "system after installation completes"
			else
				echo "Successfully reloaded UDEV rules."
			fi
		else
			echo "error: failed to determine command to reload UDEV rules. Please reboot your system after installation completes"
		fi
	fi
fi

# If szDlcSrc is a string of non-zero length then we installed a 
# configuration file specifying the location of Digilent's shared libraries
# and/or we need to update the dynamic loader's cache so that the loader
# and linker can find Digilent's shared libraries. The "ldconfig" utility
# causes the cache to be updated. Executing this command as part of the
# post installation is only necessary in order to avoid restarting the
# system. Each time the system starts up the cache is automatically
# updated.
if [ -n "${szDlcSrc}" ]
then

	/sbin/ldconfig
	if (( $? ))
	then
		echo "error: failed to update dynamic loader cache. Please reboot your system after installation completes"
	else
		echo "Successfully updated dynamic loader cache."
	fi
fi

###########################################################################
# Done

echo "Successfully installed Adept Runtime."

