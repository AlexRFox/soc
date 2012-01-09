#!/bin/bash

###########################################################################
#                                                                         #
#  install-slib.sh -- Shared Library Installation Script                  #
#                                                                         #
###########################################################################
#  Author: MTA                                                            #
#  Copyright 2011 Digilent Inc.                                           #
###########################################################################
#  File Description:                                                      #
#                                                                         #
#  This is a bash shell script that can be used to install a shared a     #
#  library. You should only use this script to install shared libraries   #
#  that have been properly named, meaning they adhere to the              #
#  "libname.so.x.y.z" format, where x is the major version, y is the      #
#  minor version, and z is the micro version associated with the library. #
#                                                                         #
#  This script takes 3 parameters. The first parameter is path/filename   #
#  of the shared library to be installed. The second parameter is the     #
#  desired destination directory for the shared library. This path is     #
#  typically "/usr/local/lib" but can be set to any valid path within the #
#  file system. The shared library may or may not be installed to this    #
#  location. If there is already another version of the shared library    #
#  installed on the system then the shared library will be installed in   #
#  that directory instead of in the directory specified. The third        #
#  parameter is an optional parameter used to indicate whether or not     #
#  this script is run in silent mode. If this script is run in silent     #
#  mode then the user will not be asked for confirmation when an existing #
#  version of the library is detected. Instead the new version will be    #
#  installed and all symbolic links will be updated, as needed.           #
#                                                                         #
#  To run in silent mode call this script with the third parameter        #
#  specfied as "silent=1".                                                #
#                                                                         #
#  Exit Codes:                                                            #
#                                                                         #
#  0  - shared library successfully installed, no existing installation   #
#  30 - shared library successfully installed, existing installation      #
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
#                                                                         #
#  Example Usage:                                                         #
#                                                                         #
#  "install-slib.sh lib/libdabs.so.2.7.1 /usr/local/lib/digilent/adept"   #
#                                                                         #
###########################################################################
#  Revision History:                                                      #
#                                                                         #
#  03/03/2011(MTA): created                                               #
#  03/04/2011(MTA): updated exit codes                                    #
#                                                                         #
###########################################################################

##################################################################
####	CreateSymLink
##
##	Parameters:
##		1 - pathname/filename of the target (file we are linking to)
##		2 - pathname/filename of the link (the link we are creating)
##
##	Return Value:
##		0 for success, 1 otherwise
##
##	Errors:
##		none
##
##	Description:
##		Create a symbolic link for the specified target and link filenames.
function CreateSymLink()
{
	local szDNT
	local szDNL
	local szBNT
	local szBNL
	local szCWD
	
	szDNT=$(dirname "${1}")
	szBNT=$(basename "${1}")
	szDNL=$(dirname "${2}")
	szBNL=$(basename "${2}")
	
	szCWD=$(pwd)
	
	# Change the current working directory to the directory that we are
	# creating the link in.
	cd "${szDNL}"
	if (( $? ))
	then
		echo "    error: failed to change working directory to ${szDNL}"
		return 1
	fi
	
	# Create the symbolic link using the appropriate command. If both
	# the target and link reside in the same directory then we want to
	# link directly to the basename of the target.
	if [ "${szDNT}" = "${szDNL}" ]
	then
		ln -fs "${szBNT}" "${szBNL}" && \
		chmod 755 "${szBNL}"
	else
		ln -fs "${1}" "${szBNL}" && \
		chmod 755 "${szBNL}"
	fi
	
	if (( $? ))
	then
		echo "    error: failed to create symbolic link or set permissions"
		cd "${szCWD}"
		return 1
	fi
	
	# Change back to the original working directory.
	cd "${szCWD}"
	if (( $? ))
	then
		echo "    error: failed to change working directory to ${szCWD}"
		return 1
	fi
	
	return 0
}

##################################################################
####	GetUserConfirmation
##
##	Parameters:
##		1 - string containing the question to ask the user
##
##	Return Value:
##		1 if the user confirms, 0 otherwise
##
##	Errors:
##		none
##
##	Description:
##		Ask the user for confirmation. This function will display the
##		specified string followed by "? (y/n) [y] " and wait for input. If
##		the user responds with "n", "N", "no", or "NO" then 0 is returned.
##		In all other cases 1 is returned. If the fNotSilent is not set then
##		the user is not prompted and 1 is returned.
function GetUserConfirmation()
{
	local sz
	
	if (( $fNotSilent ))
	then
		read -p "${1}? (y/n) [y] " sz
		if [ "${sz}" = "n" -o "${sz}" = "N" -o "${sz}" = "no" -o "${sz}" = "NO" ]
		then
			return 0
		fi
	fi
	
	return 1
}

##################################################################
####	IsLibInstalled
##
##	Parameters:
##		1 - name of the library
##
##	Return Value:
##		1 if the dynamic loader can find the library, 0 otherwise
##
##	Errors:
##		none
##
##	Description:
##		Check to see if the dynamic loader thinks the specified library
##		is installed on the system. If the dynamic loader knows how to
##		find the library then rgszLibPath will contain a list of all of
##		the paths that the dynamic knows about for the library and 1 will
##		be returned. If the dynamic loader doesn't know how to find the
##		library then 0 is returned and rgszLibPath won't contain any paths.
function IsLibInstalled()
{
	# Create an array of pathnames that the dynamic loader knows about for
	# the specified library.
	rgszLibPath=($(/sbin/ldconfig -p | grep "${1}" | cut -f 2 -d ">"))
	
	if (( $? != 0 || ${#rgszLibPath[*]} == 0 ))
	then
		return 0
	else
		return 1
	fi
}

##################################################################
####	ParseLibFilename
##
##	Parameters:
##		1 - shared library file name
##
##	Return Value:
##		szLibName - library name
##		szLibVmjr - library major version number
##		szLibVmin - library minor version number
##		szLibVmic - library micro version number
##
##	Errors:
##		none
##
##	Description:
##		Parse a shared library filename into it's useful components. A
##		shared library's filename is expected to be formatted as
##		"libname.so.x.y.z" where x is the major revision, y is the minor
##		revision, and z is the micro revision. Please note that the shared
##		library filename should be a basename (e.g. not contain a path).
function ParseLibFilename()
{
	local rgszTemp
	local szTemp
	
	szTemp=$(basename "${1}")
	
	rgszTemp=(${szTemp//.so./ })
	szLibName=${rgszTemp[0]}
	
	rgszTemp=(${rgszTemp[1]//./ })
	szLibVmjr=${rgszTemp[0]}
	szLibVmin=${rgszTemp[1]}
	szLibVmic=${rgszTemp[2]}
}

# Make sure that the user provided the necessary parameters.
if [ -z "${1}" ]
then
	echo "    error: you must specify the pathname/filename of the shared library to be installed"
	exit 1
fi

if [ -z "${2}" ]
then
	echo "    error: you must specify an installation directory"
	exit 1
fi

# Determine if we should run in silent-mode.
let fNotSilent=1
if [ "${3}" = "silent=1" ]; then let fNotSilent=0; fi

# Get the filename of the shared library that we wish to install.
szLibInstFN="${1}"
if [  ! -f "${szLibInstFN}" -o ! -x "${szLibInstFN}" ]
then
	echo "    error: invalid filename specified - ${szLibInstFN}"
	exit 2
fi

# Parse the library filename into its useful components.
ParseLibFilename "${szLibInstFN}"
szLibInstName=${szLibName}
szLibInstVmjr=${szLibVmjr}
szLibInstVmin=${szLibVmin}
szLibInstVmic=${szLibVmic}
szLibInstSOVER="${szLibVmjr}.${szLibVmin}.${szLibVmic}"

# Make sure that we managed to parse the filename into it's useful
# components and that the filename contained all of the expected
# components.
if [ -z "${szLibInstName}" -o -z "${szLibInstVmjr}" \
     -o -z "${szLibInstVmin}" -o -z "${szLibInstVmic}" ]
then
	echo "    error: invalid filename - ${szLibInstFN}"
	exit 2
fi

# Check to see if the specified library is already installed.
echo "    Checking to see if ${szLibInstName}.so is already installed...."
IsLibInstalled "${szLibInstName}.so"
if (( $? ))
then
	let fExistingInstall=1
	
	echo "    Existing installation of ${szLibInstName}.so found. Checking to see if this version should be installed."
	
	let fInstallLib=1
	for (( i=0; i < ${#rgszLibPath[*]} ; i++ ))
	do
		if [ -L "${rgszLibPath[${i}]}" ]
		then
			rgszLibFN[${i}]=$(ls -l "${rgszLibPath[${i}]}" | cut -f 2 -d ">" | sed 's/[ ]*//')
		else
			rgszLibFN[${i}]="${rgszLibPath[${i}]}"
		fi
		
		ParseLibFilename "${rgszLibFN[${i}]}"
		rgszLibName[${i}]=${szLibName}
		rgszLibVmjr[${i}]=${szLibVmjr}
		rgszLibVmin[${i}]=${szLibVmin}
		rgszLibVmic[${i}]=${szLibVmic}
		
		if (( ${rgszLibVmjr[${i}]} == ${szLibInstVmjr} ))
		then
			if (( ${rgszLibVmin[${i}]} == ${szLibInstVmin} ))
			then
				if (( ${rgszLibVmic[${i}]} >= ${szLibInstVmic} ))
				then
					let fInstallLib=0
					szVerCurInst="${rgszLibVmjr[${i}]}.${rgszLibVmin[${i}]}.${rgszLibVmic[${i}]}"
					break
				fi
			elif (( ${rgszLibVmin[${i}]} > ${szLibInstVmin} ))
			then
				let fInstallLib=0
				szVerCurInst="${rgszLibVmjr[${i}]}.${rgszLibVmin[${i}]}.${rgszLibVmic[${i}]}"
				break;
			fi
		fi
	done
	
	if (( $fInstallLib ))
	then
		# Figure out where to install the library.
		szLibInstPath=""
		for (( i=0; i < ${#rgszLibFN[*]}; i++ ))
		do
			szTemp=$(dirname "${rgszLibFN[${i}]}")
			if [ "${szTemp}" != "." ]
			then
				szLibInstPath="${szTemp}"
				break
			fi
		done
		
		if [ -z "${szLibInstPath}" ]
		then
			
			for (( i=0; i < ${#rgszLibPath[*]}; i++ ))
			do
				szTemp=$(dirname "${rgszLibPath[${i}]}")
				if [ "${szTemp}" != "." ]
				then
					szLibInstPath="${szTemp}"
					break
				fi
			done
		fi
		
		if [ -z "${szLibInstPath}" ]
		then
			szLibInstPath="${2}"
		fi
		
		# Create the destination directory if it doesn't exist.
		mkdir -p "${szLibInstPath}"
		if (( $? ))
		then
			echo "    error: failed to create destination directory - ${szLibInstPath}"
			exit 3
		fi
		
		# Copy the file to the destination directory.
		cp -f "${szLibInstFN}" "${szLibInstPath}" && \
		chmod 755 "${szLibInstPath}/$(basename ${szLibInstFN})"
		if (( $? ))
		then
			echo "    error: failed to install shared library in ${szLibInstPath}"
			exit 4
		fi
		
		echo "    Installed shared library \"${szLibInstPath}/$(basename ${szLibInstFN})\""
		
		# Go through the list of references to the library and update any
		# links as necessary.
		let fVmjrLinkExists=0
		for (( i=0; i < ${#rgszLibPath[*]}; i++ ))
		do
			# Determine if we should update a link based on the version of
			# the library that the link currently points to and the version
			# of the library that we are installing. If the link reference
			# contains the library major version number then we need to
			# confirm that it matches the major version of the library that
			# we are installing and that the library that we are installing
			# has the major version in it's SONAME.
			let fUpdateLink=0
			ParseLibFilename "${rgszLibPath[${i}]}"
			if [ -n "${szLibVmjr}" ]
			then
				if (( ${szLibVmjr} == ${szLibInstVmjr} ))
				then
					let fVmjrLinkExists=1
					
					szSONAME=$(objdump -x "${szLibInstFN}" | grep SONAME | sed 's/[ ]*SONAME[ ]*//')
					if [ "${szSONAME}" = "${szLibInstName}.so.${szLibInstVmjr}" ]
					then
						let fUpdateLink=1
					fi
				fi
			else
				if (( ${szLibInstVmjr} > ${rgszLibVmjr[${i}]} ))
				then
					let fUpdateLink=1
				elif (( ${szLibInstVmjr} == ${rgszLibVmjr[${i}]} && ${szLibInstVmin} > ${rgszLibVmin[${i}]} ))
				then
					let fUpdateLink=1
				elif (( ${szLibInstVmjr} == ${rgszLibVmjr[${i}]} && ${szLibInstVmin} == ${rgszLibVmin[${i}]} && ${szLibInstVmic} > ${rgszLibVmic[${i}]} ))
				then
					let fUpdateLink=1
				fi
			fi
			
			if (( $fUpdateLink ))
			then
				GetUserConfirmation "    Update existing link for ${rgszLibPath[${i}]}"
				if (( $? ))
				then
					CreateSymLink "${szLibInstPath}/$(basename ${szLibInstFN})" "${rgszLibPath[${i}]}"
					if (( $? ))
					then
						echo "    error: failed to update symbolic link for ${rgszLibPath[${i}]}"
						exit 5
					fi
					
					echo "    Updated symbolic link \"${rgszLibPath[${i}]}\""
				fi
			fi
		done
		
		if (( ${fVmjrLinkExists} == 0 ))
		then
			# There isn't currently a symbolic link containing the major
			# version of the shared library. We should create this link
			# if the shared library that we installed contains the version
			# in its SONAME.
			szSONAME=$(objdump -x "${szLibInstFN}" | grep SONAME | sed 's/[ ]*SONAME[ ]*//')
			if [ "${szSONAME}" = "${szLibInstName}.so.${szLibInstVmjr}" ]
			then
				CreateSymLink "${szLibInstPath}/$(basename ${szLibInstFN})" "${szLibInstPath}/${szLibInstName}.so.${szLibInstVmjr}"
				if (( $? ))
				then
					echo "    error: failed to create symbolic link ${szLibInstPath}/${szLibInstName}.so.${szLibInstVmjr}"
					exit 5
				fi
				
				echo "    Created symbolic link \"${szLibInstPath}/${szLibInstName}.so.${szLibInstVmjr}\""
			fi
		fi
	else
		echo "    Version ${szVerCurInst} is currently installed. Version ${szLibInstName}.so.${szLibInstSOVER} will not be installed."
	fi
else
	let fExistingInstall=0
	
	echo "    No existing installation of ${szLibInstName}.so found."
	
	# The installation directory path is expected to be provided as the
	# second parameter to the script.
	szLibInstPath="${2}"
	
	# Create the destination directory if it doesn't exist.
	mkdir -p "${szLibInstPath}"
	if (( $? ))
	then
		echo "    error: failed to create destination directory - ${szLibInstPath}"
		exit 3
	fi
	
	# Copy the file to the destination directory.
	cp -f "${szLibInstFN}" "${szLibInstPath}" && \
	chmod 755 "${szLibInstPath}/$(basename ${szLibInstFN})"
	if (( $? ))
	then
		echo "    error: failed to install shared library in ${szLibInstPath}"
		exit 4
	fi
	
	echo "    Installed shared library \"${szLibInstPath}/$(basename ${szLibInstFN})\""
	
	# Create symbolic link for "libname.so".
	CreateSymLink "${szLibInstPath}/$(basename ${szLibInstFN})" "${szLibInstPath}/${szLibInstName}.so"
	if (( $? ))
	then
		echo "    error: failed to create symbolic link ${szLibInstPath}/${szLibInstName}.so"
		exit 5
	fi
	
	echo "    Created symbolic link \"${szLibInstPath}/${szLibInstName}.so\""
	
	# Create a symbolic link for "libname.so.x" where x is the major
	# version of the shared library. Please note that this link should only
	# be created if the SONAME of the shared library that we installed
	# contains the major version.
	szSONAME=$(objdump -x "${szLibInstFN}" | grep SONAME | sed 's/[ ]*SONAME[ ]*//')
	if [ "${szSONAME}" = "${szLibInstName}.so.${szLibInstVmjr}" ]
	then
		CreateSymLink "${szLibInstPath}/$(basename ${szLibInstFN})" "${szLibInstPath}/${szLibInstName}.so.${szLibInstVmjr}"
		if (( $? ))
		then
			echo "    error: failed to create symbolic link ${szLibInstPath}/${szLibInstName}.so.${szLibInstVmjr}"
			exit 5
		fi
		
		echo "    Created symbolic link \"${szLibInstPath}/${szLibInstName}.so.${szLibInstVmjr}\""
	fi
fi

if (( $fExistingInstall ))
then
	exit 30
else
	exit 0
fi

