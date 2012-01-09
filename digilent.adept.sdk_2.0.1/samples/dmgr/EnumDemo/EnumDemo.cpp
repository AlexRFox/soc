/************************************************************************/
/*																		*/
/*  EnumDemo.cpp  --  Enum Demo Main Program							*/
/*																		*/
/************************************************************************/
/*  Author:	Aaron Odell													*/
/*  Copyright:	2010 Digilent, Inc.										*/
/************************************************************************/
/*  Module Description: 												*/
/*		Enum Demo is a project to demonstrate how to enumerate			*/
/*		connected devices using the DMGR module of the Adept SDK.		*/
/*																		*/
/************************************************************************/
/*  Revision History:													*/
/*																		*/
/*	03/16/2010(AaronO): created											*/
/*																		*/
/************************************************************************/

#define	_CRT_SECURE_NO_WARNINGS

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
void ErrorExit();

/* ------------------------------------------------------------ */
/*				Procedure Definitions							*/
/* ------------------------------------------------------------ */
/***	main
**
**	Parameters:
**		none
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

int main(void) {

	DVC		dvc;
	int		idvc;
	int		cdvc;
	char	szTmp[1024];
	bool	fSkip;


	/* Produce the enumerated list of devices. */	
	// DMGR API Call: DmgrEnumDevices
	if (!DmgrEnumDevices(&cdvc)) {
		printf("Error enumerating devices\n");
		ErrorExit();
	}

	/* Print some basic information about each device. */
	printf("\n");
	for (idvc = 0; idvc < cdvc; idvc++) {
		fSkip = false;

		// DMGR API Call: DmgrGetDvc
		if (!DmgrGetDvc(idvc, &dvc)) {
			printf("Error getting device info\n");
			ErrorExit();
		}

		printf("Device: %s\n", dvc.szName);
		
		/* Read and print product name. */
	
		// DMGR API Call: DmgrGetInfo
		if (!DmgrGetInfo(&dvc, dinfoProdName, szTmp)) {
			sprintf(szTmp, "Not accessible");
		}

		printf("  Product Name:           %s\n",szTmp);

		/* Read and print user name. */	
		// DMGR API Call: DmgrGetInfo

		if (!DmgrGetInfo(&dvc, dinfoUsrName, szTmp)) {
			sprintf(szTmp, "Not accessible");
		}
	
		printf("  User Name:              %s\n",szTmp);


		/* Read and print serial number */
		// DMGR API Call: DmgrGetInfo
		if (!DmgrGetInfo(&dvc, dinfoSN, szTmp)) {
			sprintf(szTmp, "Not accessible");
		}

		printf("  Serial Number:          %s\n", szTmp);

		printf("\n");
	}  // End for


	/* Clean up and get out */
	// DMGR API Call: DmgrFreeDvcEnum
	DmgrFreeDvcEnum();
	return 0;
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
**		Frees dmgr device enumeration and exits
**	
*/
void ErrorExit() {
	// DMGR API Call: DmgrFreeDeviceEnum
	DmgrFreeDvcEnum();

	exit(1);
}

/* ------------------------------------------------------------ */

/************************************************************************/
