#!/bin/bash

###########################################################################
#                                                                         #
#  install.sh -- Digilent Adept Utilities Installation Script             #
#                                                                         #
###########################################################################
#  Author: MTA                                                            #
#  Copyright 2010 Digilent Inc.                                           #
###########################################################################
#  File Description:                                                      #
#                                                                         #
#  This is a bash shell script that can be used to install Digilent's     #
#  Adept Utilities on a target system. This script should be executed     #
#  from the root of the Adept Utilities release directory. Please note    #
#  that you need to run the script as root for the installation to        #
#  succeed.                                                               #
#                                                                         #
###########################################################################
#  Revision History:                                                      #
#                                                                         #
#  06/09/2010(MTA): created                                               #
#                                                                         #
###########################################################################

binsrc=""
bindst=""
mansrc=""
mandst=""
manpath=""

echo "Adept Utilities Installer"

###########################################################################
# Determine the source for the binaries.

# Is the machine we are installing on running a 64-bit distribution?
mname=$(uname -m)
if [ "${mname}" = "x86_64" ]
then	
	# Yes the system is running a 64-bit distribution.
	
	echo "64-bit operating system detected"
	
	# Make sure the release contains 64-bit binaries.
	binsrc="bin64"
	if [ -d "${binsrc}" ]
	then
		
		# Attempt to create a list of 64-bit binaries.
		rgszBins=($(ls ${binsrc}))
		if (( $? != 0 || ${#rgszBins[*]} == 0 ))
		then
			echo "error: 64-bit binaries not found"
			exit 1
		fi
	else
		echo "error: 64-bit binaries not found"
		exit 1
	fi
else
	
	echo "32-bit operating system detected"
	
	# Make sure the release contains 32-bit binaries.
	binsrc="bin"
	if [ -d "${binsrc}" ]
	then
		# Attempt to create a list of 32-bit binaries.
		rgszBins=($(ls ${binsrc}))
		if (( $? != 0 || ${#rgszBins[*]} == 0 ))
		then
			echo "error: 32-bit binaries not found"
			exit 1
		fi
	else
		echo "error: 32-bit binaries not found"
		exit 1
	fi
fi


###########################################################################
# Determine the destination for the binaries.

# Attempt to determine a suitable default destination path for the
# binraries based on the PATH variable. 

szBinPath="${PATH}"
if [[ "${szBinPath}" == */usr/local/bin* ]]
then
	szBinPath="/usr/local/bin"
elif [[ "${szBinPath}" == */usr/bin* ]]
then
	szBinPath="/usr/bin"
else
	szBinPath=""
fi

if [ -n "${szBinPath}" ]
then
	# A suitable default path was found. Prompt the user for the path
	# used to install the binaries. If no path is specified by the user 
	# then the default will be used.
	
	read -p "In which directory should binaries be installed? [${szBinPath}] " bindst
	if [ -z "${bindst}" ]; then bindst="${szBinPath}"; fi
	
	bindst="${bindst%/}"
else

	# No suitable default path was found. Prompt the user for the path used
	# to install the binaries. He or she must provide a path.
	
	read -p "In which directory should binaries be installed? " bindst
	
	while [ -z "${bindst}" ]
	do
		echo "You must specify a directory."
		
		read -p "In which directory should binaries be installed? " bindst
	done
	
	bindst="${bindst%/}"
fi


###########################################################################
# Determine the source for manuals.

# Manuals are not required to be included with a release. Check to see if
# any manuals were included.
mansrc="man"
if [ -d "${mansrc}" ]
then

	# The directory that should contain manuals exists. Check to see if it
	# actually contains any files and create a list of those files.
	rgszMans=($(ls "${mansrc}"))
	if (( $? != 0 || ${#rgszMans[*]} == 0 ))
	then
	
		# No manuals were found. Set mansrc to an empty string so that we
		# don't attempt to install the manuals later.
		
		mansrc=""
	fi
	
else
	# No manuals were found. Set mansrc to an empty string so that we
	# don't attempt to install the manuals later.
	
	mansrc=""
fi

# If we found manuals then we need to determine the path that will be used
# for installation.
mandst=""
if [ -n "${mansrc}" ]
then
	
	# At least one manual was found.
	
	# Attempt to determine a suitable default installation path.
	manpath=$(manpath)
	if (( $? == 0 ))
	then
		
		if [[ "${manpath}" == */usr/local/man* ]]
		then
			manpath="/usr/local/man"
		elif [[ "${manpath}" == */usr/local/share/man* ]]
		then
			manpath="/usr/local/share/man"
		elif [[ "${manpath}" == */usr/share/man* ]]
		then
			manpath="/usr/share/man"
		else
			manpath=""
		fi
	else
		manpath=""
	fi
	
	if [ -n "${manpath}" ]
	then
		
		# A suitable default installation path was found. Prompt the user
		# for the manpath. If he or she doesn't provide a manpath then we
		# will use the default path.
		
		read -p "In which directory should manuals be installed? [${manpath}] " mandst
		if [ -z "${mandst}" ]; then mandst="${manpath}"; fi
	
		mandst="${mandst%/}"
	else
	
		# No suitable default installation path was found. Prompt the
		# user for the manpath. He or she must provide a path.
		
		read -p "In which directory should manuals be installed? " mandst
		
		while [ -z "${mandst}" ]
		do
			echo "You must specify a directory."
			read -p "In which directory should manuals be installed? " mandst
		done
		
		mandst="${mandst%/}"
	fi
else
	echo "No manuals were found"
fi


###########################################################################
# Install the binaries.

echo "Installing binaries....."

# Create the destination directory if it doesn't exist.
mkdir -p ${bindst}
if (( $? ))
then
	echo "error: failed to create destination directory \"${bindst}\""
fi

# rgszBins should contain a list of the binary files to be installed. Use
# this list to install the binaries and set the appropriate permissions.
for (( i=0; i < ${#rgszBins[*]} ; i++ ))
do
	
	cp -f "${binsrc}/${rgszBins[${i}]}" "${bindst}" && \
	chmod 755 "${bindst}/${rgszBins[${i}]}"
	
	if (( $? ))
	then
		echo "error: failed to install binaries"
		exit 1
	fi
	
	echo "    installed ${rgszBins[${i}]}: \"${bindst}/${rgszBins[${i}]}\""
done

echo "Successfully installed binaries in \"${bindst}\""


###########################################################################
# Install the manuals (if necessary).

# If any manuals were found then the "mandst" variable will have a non-zero
# length and will contain the path to be used during installation.
if [ -n "${mandst}" ]
then
	
	# The release contains manuals and we have determined an appropriate
	# installation directory.
	
	echo "Installing manuals....."
	
	# rgszMans should contain a list of the manuals that need to be
	# installed. Go through the list and install the manuals in the
	# appropriate location.
	let cman=0
	let cmanInst=0
	for (( i=0; i < ${#rgszMans[*]} ; i++ ))
	do
		
		# Manuals must named in the following format "name.section" where
		# section should be a single decimal number in the range of 1 - 8.
		# If the manual doesn't have a valid section specified then it will
		# not be installed.
		
		# Determine the section based on the file name.
		case "${rgszMans[${i}]}" in
			*.[1] ) mansect="man1";;
			*.[2] ) mansect="man2";;
			*.[3] ) mansect="man3";;
			*.[4] ) mansect="man4";;
			*.[5] ) mansect="man5";;
			*.[6] ) mansect="man6";;
			*.[7] ) mansect="man7";;
			*.[8] ) mansect="man8";;
				* ) mansect="";;
		esac
		
		if [ -n "${mansect}" ]
		then
		
			# We managed to determine the section for the manual that's
			# being installed. Proceed with the installation.
			
			#Increment the count of valid manuals found.
			let cman+=1
			
			# Create the directory if it doesn't exist.
			mkdir -p "${mandst}/${mansect}"
			if (( $? == 0 ))
			then
				
				# Destination directory exists. Copy the file and set
				# the appropriate permissions.
				
				cp -f "${mansrc}/${rgszMans[${i}]}" "${mandst}/${mansect}" && \
				chmod 644 "${mandst}/${mansect}/${rgszMans[${i}]}"
				if (( $? == 0 ))
				then
					echo "    installed ${rgszMans[${i}]}: \"${mandst}/${mansect}/${rgszMans[${i}]}\""
					let cmanInst+=1
				else
					echo "error: failed to install \"${mandst}/${mansect}/${rgszMans[${i}]}\""
				fi
			else
				echo "error: failed to create directory \"${mandst}/${mansect}\""
			fi
		fi
	done
	
	if (( $cman == $cmanInst ))
	then
		echo "Successfully installed manuals in \"${mandst}\""
	fi
fi


###########################################################################
# All done.

echo "Successfully installed Adept Utilities"

