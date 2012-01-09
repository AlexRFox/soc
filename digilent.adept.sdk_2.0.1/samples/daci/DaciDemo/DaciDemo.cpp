/************************************************************************/
/*																		*/
/*  DaciDemo.cpp  --  DACI Demo Main Program							*/
/*																		*/
/************************************************************************/
/*  Author:	Aaron Odell													*/
/*  Copyright:	2010 Digilent, Inc.										*/
/************************************************************************/
/*  Module Description: 												*/
/*		DACI Demo is a project designed to demonstrate how				*/
/*		to send and recieve data asynchronously on an I/O				*/
/*		Explorer board using the DACI module of the Adept SDK.			*/
/*																		*/
/************************************************************************/
/*  Revision History:													*/
/*																		*/
/*	07/21/2010(AaronO): created											*/
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
#include "daci.h"

/* ------------------------------------------------------------ */
/*					Local Type and Constant Definitions			*/
/* ------------------------------------------------------------ */


/* ------------------------------------------------------------ */
/*					Global Variables							*/
/* ------------------------------------------------------------ */


/* ------------------------------------------------------------ */
/*					Local Variables								*/
/* ------------------------------------------------------------ */

// Device handle
HIF hif = hifInvalid;

// Device name string
char szDvc[64] = "ioexp";

// Variables to be set by Daci functions
ULONG bdrSet;
ULONG cchRcv;

// Strings to send and recieve
char szSend[] = "Digilent test message";
char szRcv[1024];

// UART configuration
ULONG	bdrReq		= 2400;
INT32	cbtData		= 8;
INT32	idStop		= 1;
INT32	idParity	= 0;
BOOL	fRxBlock	= fTrue;

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
**		DaciDemo main function
*/
int main(void) {

	/* Clear recieve string buffer */
	memset(szRcv, 0, 1024);

	/* Connect to device using DMGR */
	// DMGR API Call: DmgrOpen
	if(!DmgrOpen(&hif, szDvc)) {
		printf("Error: Could not open device %s\n", szDvc);
		ErrorExit();
	}

	/* Enable asynchronous communications interface (UART) on device */
	// DACI API Call: DaciEnable
	if(!DaciEnable(hif)) {
		printf("Error: DtwiEnable failed\n");
		ErrorExit();
	}

	/* Configure baud rate on board */
	// DACI API Call: DaciSetBaud
	if(!DaciSetBaud(hif, bdrReq, &bdrSet)) {
		printf("Error: DaciSetBaud failed\n");
		ErrorExit();
	}

	/* Verify that the baud rate was set correctly */
	if( bdrSet != bdrReq ) {
		printf("WARNING: Baud rate set was different from requested\n");
		printf("Requested: %lu\n", bdrReq);
		printf("Set: %lu\n", bdrSet);
	}

	/* Configure UART frame */
	// DACI API Call: DaciSetMode
	if(!DaciSetMode(hif, cbtData, idStop, idParity)) {
		printf("Error: DaciSetMode failed\n");
		ErrorExit();
	}

	/* Make recieve functions blocking. This allows all data sent by the transmitter to arrive before the DaciGetBuf call returns. */
	// DACI API Call: DaciSetRxBlock
	if(!DaciSetRxBlock(hif, fTrue)) {
		printf("Error: DaciSetRxBlock failed\n");
		ErrorExit();
	}

	/* Send data to board for UART transmission */
	// DACI API Call: DaciPutBuf
	if(!DaciPutBuf(hif, (BYTE*) szSend, strlen(szSend), fFalse)) {
		printf("Error: DaciPutBuf failed\n");
		ErrorExit();
	}

	/* Get recieved UART data. Note that without the previous call to DaciSetRxBlock, this call will fail because
	   insufficient time will have elapsed from when DaciPutBuf was called for any data to have arrived at the reciever */
	// DACI API Call: DaciGetBuf
	if(!DaciGetBuf(hif, (BYTE*) szRcv, strlen(szSend), &cchRcv, fFalse)) {
		printf("Error: DaciGetBuf failed\n");
		ErrorExit();
	}


	/* Compare recieved data to transmitted data */
	if( strcmp(szSend, szRcv) == 0 ) {
		printf("Recieved data matched transmitted data. Success\n");
	}
	else {
		printf("Error: Recieved string was different from send string.\n");
		printf("Sent: %s\n", szSend);
		printf("Recieved: %s\n", szRcv);
	}


	// DACI API Call: DaciDisable
	DaciDisable(hif);

	// DMGR API Call: DmgrClose
	DmgrClose(hif);


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
**		Disables Daci, closes the device, and exits the program
*/
void ErrorExit() {
	if( hif != hifInvalid ) {
		// DACI API Call: DaciDisable
		DaciDisable(hif);

		// DMGR API Call: DmgrClose
		DmgrClose(hif);
	}

	exit(1);
}

/* ------------------------------------------------------------ */

/************************************************************************/

