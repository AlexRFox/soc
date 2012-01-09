/************************************************************************/
/*																		*/
/*  DtwiDemo.cpp  --  DTWI DEMO Main Program							*/
/*																		*/
/************************************************************************/
/*  Author:	Aaron Odell													*/
/*  Copyright:	2010 Digilent, Inc.										*/
/************************************************************************/
/*  Module Description: 												*/
/*		DTWI Demo is a project to demonstrate how to communicate		*/
/*		via TWI using the DTWI module of the Adept SDK and an			*/
/*		I/O Explorer board.												*/
/*																		*/
/************************************************************************/
/*  Revision History:													*/
/*																		*/
/*	07/21/2010(AaronO): created											*/
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
#include <string.h>

#include "dpcdecl.h" 
#include "dmgr.h"
#include "dtwi.h"

/* ------------------------------------------------------------ */
/*					Local Type and Constant Definitions			*/
/* ------------------------------------------------------------ */


/* ------------------------------------------------------------ */
/*					Global Variables							*/
/* ------------------------------------------------------------ */


/* ------------------------------------------------------------ */
/*					Local Variables								*/
/* ------------------------------------------------------------ */
HIF hif = hifInvalid;

BYTE addrCls = 0x48;  // TWI slave address defined in PmodCLS reference manual
const char szClearScreen[] = { 0x1B, '[', 'j', 0 };  // Command strings are defined in the PmodCLS reference manual
const char szMsg[] = "TWI is working";  // Message to print on PmodCLS display
const char szDvc[] = "ioexp";	// Device name

/* ------------------------------------------------------------ */
/*					Forward Declarations						*/
/* ------------------------------------------------------------ */
void ErrorExit();

/* ------------------------------------------------------------ */
/*					Procedure Definitions						*/
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
**		DtwiDemo main function
*/
int main(void) {

	/* Open Device */
	// DMGR API Call: DmgrOpen
	if(!DmgrOpen(&hif, (char*) szDvc)) {
		printf("Error: Could not open device ioexp\n");
		ErrorExit();
	}

	/* Enable TWI */
	// DTWI API Call: DtwiEnable
	if(!DtwiEnable(hif)) {
		printf("Error: DtwiEnable failed\n");
		ErrorExit();
	}

	/* Send escape sequence to PmodCLS via TWI */
	// DTWI API Call: DtwiMasterPut
	if(!DtwiMasterPut(hif, addrCls, strlen(szClearScreen), (BYTE*) szClearScreen, fFalse)) {
		printf("Error: DtwiMasterPut failed\n");
		ErrorExit();
	}

	/* Send text to PmodCLS via TWI */
	// DTWI API Call: DtwiMasterPut
	if(!DtwiMasterPut(hif, addrCls, strlen(szMsg), (BYTE*) szMsg, fFalse)) {
		printf("Error: DtwiMasterPut failed\n");
		ErrorExit();
	}

	/* Disable TWI */
	// DTWI API Call: DtwiDisable
	if(!DtwiDisable(hif)) {
		printf("Error: DtwiDisable failed\n");
		ErrorExit();
	}

	// DMGR API Call: DmgrClose
	if(!DmgrClose(hif)) {
		printf("Error: DmgrClose failed\n");
		ErrorExit();
	}
	
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
**		Disables Dstm, closes the device, and exits the program
*/
void ErrorExit() {
	if(hif != hifInvalid) {
		// DTWI API Call: DtwiDisable
		DtwiDisable(hif);

		// DMGR  API Call: DmgrClose
		DmgrClose(hif);
	}
	
	exit(1);
}
