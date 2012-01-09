/************************************************************************/
/*																		*/
/*  DjtgDemo.cpp  --  DJTG DEMO Main Program							*/
/*																		*/
/************************************************************************/
/*  Author:	Aaron Odell													*/
/*  Copyright:	2010 Digilent, Inc.										*/
/************************************************************************/
/*  Module Description: 												*/
/*		DJTG Demo demonstrates how to read in IDCODEs from the			*/
/*		JTAG scan chain. Codes for some Digilent FPGA boards are		*/
/*		given below.													*/
/*																		*/
/*		Nexys2: 0x41c22093												*/
/*				0xf5046093												*/
/*																		*/
/*		Basys2: 0x11c10093												*/
/*				0xf5045093												*/
/*																		*/
/************************************************************************/
/*  Revision History:													*/
/*																		*/
/*	03/16/2010(AaronO): created											*/
/*																		*/
/************************************************************************/

#define _CRT_SECURE_NO_WARNINGS

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
#include "djtg.h"
#include "dmgr.h"

/* ------------------------------------------------------------ */
/*				Local Type and Constant Definitions				*/
/* ------------------------------------------------------------ */


/* ------------------------------------------------------------ */
/*				Global Variables								*/
/* ------------------------------------------------------------ */
HIF hif;

/* ------------------------------------------------------------ */
/*				Local Variables									*/
/* ------------------------------------------------------------ */


/* ------------------------------------------------------------ */
/*				Forward Declarations							*/
/* ------------------------------------------------------------ */

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
**
**	Errors:
**
**	Description:
**		Run the program.
*/

int main(int cszArg, char* rgszArg[]) {
	int i;
	int cCodes = 0;
	BYTE rgbSetup[] = {0xaa, 0x22, 0x00};
	BYTE rgbTdo[4];

	INT32 idcode;
	INT32 rgIdcodes[16];

	/* Command checking */
	if( cszArg < 3 ) {
		ShowUsage(rgszArg[0]);
		ErrorExit();
	}
	if( strcmp(rgszArg[1], "-d") != 0 ) {
		ShowUsage(rgszArg[0]);
		ErrorExit();
	}

	// DMGR API Call: DmgrOpen
	if(!DmgrOpen(&hif, rgszArg[2])) {
		printf("Error: Could not open device. Check device name\n");
		ErrorExit();
	}

	// DJTG API CALL: DjtgEnable
	if(!DjtgEnable(hif)) {
		printf("Error: DjtgEnable failed\n");
		ErrorExit();
	}

	/* Put JTAG scan chain in SHIFT-DR state. RgbSetup contains TMS/TDI bit-pairs. */
	// DJTG API Call: DgtgPutTmsTdiBits
	if(!DjtgPutTmsTdiBits(hif, rgbSetup, NULL, 9, NULL)) {
		printf("DjtgPutTmsTdiBits failed\n");
		ErrorExit();
	}

	/* Get IDCODES from device until we receive a value of 0x00000000 */
	do {
		
		// DJTG API Call: DjtgGetTdoBits
		if(!DjtgGetTdoBits(hif, 0, 0, rgbTdo, 32, NULL)) {
			printf("Error: DjtgGetTdoBits failed\n");
			ErrorExit();
		}

		// Convert array of bytes into 32-bit value
		idcode = (rgbTdo[3] << 24) | (rgbTdo[2] << 16) | (rgbTdo[1] << 8) | (rgbTdo[0]);

		// Place the IDCODEs into an array for LIFO storage
		rgIdcodes[cCodes] = idcode;

		cCodes++;

	} while( idcode != 0 );

	/* Show the IDCODEs in the order that they are connected on the device */
	printf("Ordered JTAG scan chain:\n");
	for(i=cCodes-2; i >= 0; i--) {
		printf("0x%08x\n", rgIdcodes[i]);
	}

	// Disable Djtg and close device handle
	if( hif != hifInvalid ) {
		// DGTG API Call: DjtgDisable
		DjtgDisable(hif);

		// DMGR API Call: DmgrClose
		DmgrClose(hif);
	}

	return 0;
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

void 
ShowUsage(char* szProgName) {
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
**		Disables DJTG, closes the device, and exits the program
*/
void ErrorExit() {
	if( hif != hifInvalid ) {

		// DJGT API Call: DjtgDisable
		DjtgDisable(hif);

		// DMGR API Call: DmgrClose
		DmgrClose(hif);
	}

	exit(1);
}

/* ------------------------------------------------------------ */

/************************************************************************/
