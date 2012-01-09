/************************************************************************/
/*																		*/
/*  GetInfoDemo.cpp  --  Get Info Demo Main Program						*/
/*																		*/
/************************************************************************/
/*  Author:	Aaron Odell													*/
/*  Copyright:	2010 Digilent, Inc.										*/
/************************************************************************/
/*  Module Description: 												*/
/*		Get Info Demo is a project to demonstrate how to get			*/
/*		information regarding a connected device using the DMGR			*/
/*		module of the Adept SDK.										*/
/*																		*/
/************************************************************************/
/*  Revision History:													*/
/*																		*/
/*	03/16/2010(AaronO): created											*/
/*																		*/
/************************************************************************/


/* ------------------------------------------------------------ */
/*					Include File Definitions					*/
/* ------------------------------------------------------------ */

#if defined(WIN32)
	
	/* Include Windows specific headers here.
	*/
	#include <windows.h>
	
#else

	/* Include Unix specific headers here.
	*/
	

#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dpcdecl.h"
#include "dmgr.h"

/* ------------------------------------------------------------ */
/*				Local Type and Constant Definitions				*/
/* ------------------------------------------------------------ */


/* ------------------------------------------------------------ */
/*				Global Variables								*/
/* ------------------------------------------------------------ */


/* ------------------------------------------------------------ */
/*				Local Variables									*/
/* ------------------------------------------------------------ */


/* ------------------------------------------------------------ */
/*				Forward Declarations							*/
/* ------------------------------------------------------------ */

bool FFindDvc(char * szDev, DVC * pdvc);
void ShowUsage(char* szProgName);
void ErrorExit();

/* ------------------------------------------------------------ */
/*				Procedure Definitions							*/
/* ------------------------------------------------------------ */
/***	main
**
**	Parameters:
**		cszArg		- number of command line arguments
**		rgszArg		- array of command line argument strings
**
**	Return Value:
**		0 if successful 
**		non-zero otherwise
**
**	Errors:
**		none
**
**	Description:
**		EnumDemo main function
*/

int main(int cszArg, char* rgszArg[]) {

	DVC		dvc;
	char	szTmp[1024];
	DCAP	dcapTmp;
	DWORD	pdidTmp;
	DWORD	dwTmp;

	/* Check for command parameter errors.
	*/
	if( cszArg < 3 ) {
		ShowUsage(rgszArg[0]);
		ErrorExit();
	}
	// Confirm device flag
	if( strcmp(rgszArg[1], "-d") != 0 ) {
		ShowUsage(rgszArg[0]);
		ErrorExit();
	}
		
	/* Find the device to use to run the test.
	*/
	if (!FFindDvc(rgszArg[2], &dvc)) {
		printf("Device not found: %s\n", rgszArg[2]);
		ErrorExit();
	}
	
	printf("\n");	

	/* Read and print product name.
	*/	
	// DMGR API Call: DmgrGetInfo
	if (!DmgrGetInfo(&dvc, dinfoProdName, szTmp)) {
		printf("Error: DmgrGetInfo failed\n");
		ErrorExit();
	}
	
	printf("  Product Name:           %s\n",szTmp);
	
	/* Read and print user name.
	*/
	// DMGR API Call: DmgrGetInfo
	if (!DmgrGetInfo(&dvc, dinfoUsrName, szTmp)) {
		printf("Error: DmgrGetInfo failed\n");
		ErrorExit();
	}
	
	printf("  User Name:              %s\n",szTmp);

	/* Read and print serial number
	*/	
	// DMGR API Call: DmgrGetInfo
	if (!DmgrGetInfo(&dvc, dinfoSN, szTmp)) {
		printf("Error: DmgrGetInfo failed\n");
		ErrorExit();
	}
	
	printf("  Serial Number:          %s\n", szTmp);

	/* Read and print product id.
	*/	
	// DMGR API Call: DmgrGetInfo
	if (!DmgrGetInfo(&dvc, dinfoPDID, &pdidTmp)) {
		printf("Error: DmgrGetInfo failed\n");
		ErrorExit();
	}
	
	printf("  Product ID:             %8.8X\n", pdidTmp);
	
	/* Read and print public device capabilities
	*/	
	// DMGR API Call: DmgrGetInfo
	if (!DmgrGetInfo(&dvc, dinfoDCAP, &dcapTmp)) {
		printf("Error: DmgrGetInfo failed\n");
		ErrorExit();
	}
	
	printf("  Device Capabilities:    %8.8X\n", dcapTmp);
	
	/* Print the names of the various application protocols supported.
	*/
	if ((dcapTmp & dcapJtag) != 0) {
		printf("    DJTG  - JTAG scan chain access\n");
	}
	if ((dcapTmp & dcapPio) != 0) {
		printf("    DPIO  - Digital Pin Input/Output\n");
	}
	if ((dcapTmp & dcapEpp) != 0) {
		printf("    DEPP  - Asynchronous Parallel Input/Output\n" );
	}
	if ((dcapTmp & dcapStm) != 0) {
		printf("    DSTM  - Streaming Synchronous Parallel Input/Output\n");
	}
	if ((dcapTmp & dcapSpi) != 0) {
		printf("    DSPI  - Serial Peripheral Interface\n");
	}
	if ((dcapTmp & dcapTwi) != 0) {
		printf("    DTWI  - Two Wire Serial Interface\n");
	}
	if ((dcapTmp & dcapAci) != 0) {
		printf("    DACI  - Asynchronous Serial Interface\n");
	}
	if ((dcapTmp & dcapAio) != 0) {
		printf("    DAIO  - Analog Input/Output Interface\n");
	}
	if ((dcapTmp & dcapEmc) != 0) {
		printf("    DEMC  - Electro-Mechanical Interface\n");
	}
	if ((dcapTmp & dcapGio) != 0) {
		printf("    DGIO  - General Sensor and U/I Device Interface\n");
	}
	
	/* Read and print the firmware version number
	*/
	dwTmp = 0;
	// DMGR API Call: DmgrGetInfo
	if (DmgrGetInfo(&dvc, dinfoFWVER, &dwTmp)) {
		printf("  Firmware Version:       %4.4X\n", dwTmp);	
	}

	/* Print the transport type.
	*/
	printf("  Device Transport Type:  %8.8X\n", dvc.dtp);
	
	/* All done
	*/
	return 0;

}


/* ------------------------------------------------------------ */
/***	FFindDvc
**
**	Parameters:
**		szDev		- name of device to find
**		pdvc		- variable to receive DVC for the device
**
**	Return Value:
**		none
**
**	Errors:
**		Returns fTrue if device found, fFalse if not
**
**	Description:
**		Enumerate and find the specified device.
*/
bool FFindDvc(char * szDev, DVC * pdvc) {

	int		idvc;
	int		cdvc;
	bool	fRes;
	
	/* Find the device to use to run the test.
	*/
	fRes = fFalse;

	// DMGR API Call: DmgrEnumDevices
	DmgrEnumDevices(&cdvc);

	for (idvc = 0; idvc < cdvc; idvc++) {
		// DMGR API Call: DmgrGetDvc
		DmgrGetDvc(idvc, pdvc);
		if (strcmp(pdvc->szName, szDev) == 0) {
			fRes = fTrue;
			break;
		}
	}
	
	// DMGR API Call: DmgrFreeDvcEnum
	DmgrFreeDvcEnum();
	return fRes;
	
}

/* ------------------------------------------------------------ */
/***	ShowUsage
**
**	Parameters:
**		szProgName	- name of program as called (from rgszArg[0])
**
**	Return Value:
**		none
**
**	Errors:
**		none
**
**	Description:
**		Demonstrates proper paramater usage to the user
*/
void ShowUsage(char* szProgName) {

	printf("Error: Invalid paramaters\n");
	printf("Usage: %s -d <device> \n\n", szProgName);
}

/* ------------------------------------------------------------ */
/***	ErrorExit
**
**	Parameters:
**		none
**
**	Return Value:
**		none
**
**	Errors:
**		none
**
**	Description:
**		Exits the program
*/
void ErrorExit() {
	exit(1);
}


/* ------------------------------------------------------------ */

/************************************************************************/
