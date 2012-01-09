/************************************************************************/
/*																		*/
/*  DstmDemo.cpp  --  DSTM DEMO Main Program							*/
/*																		*/
/************************************************************************/
/*  Author:	Aaron Odell													*/
/*  Copyright:	2010 Digilent, Inc.										*/
/************************************************************************/
/*  Module Description: 												*/
/*		DSTM Demo is a project to demonstrate how to transfer data		*/
/*		to and from a Digilent FPGA board using the DSTM module of		*/
/*		the Adept SDK.													*/
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

#include "dpcdecl.h"
#include "dmgr.h"
#include "dstm.h"

/* ------------------------------------------------------------ */
/*					Local Type and Constant Definitions			*/
/* ------------------------------------------------------------ */

char szDvc[128] = "Nexys2";   // Device name


/* ------------------------------------------------------------ */
/*					Global Variables							*/
/* ------------------------------------------------------------ */

const int cbTx = 8;

/* ------------------------------------------------------------ */
/*					Local Variables								*/
/* ------------------------------------------------------------ */
HIF hif;


BYTE rgbOut[cbTx] = { 0, 1, 2, 3, 4, 5, 6, 7 };
BYTE rgbIn[cbTx];
BOOL fFail = fFalse;

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
**		none
**
**	Errors:
**		none
**
**	Description:
**		DstmDemo main
*/
int main(void) {
	int ibTx;

	// DMGR API Call: DmgrOpen
	if(!DmgrOpen(&hif, szDvc)) {
		printf("Error: Could not open device %s\n", szDvc);
		ErrorExit();
	}

	// DSTM API Call: DstmEnable
	if(!DstmEnable(hif)) {
		printf("Error: DstmEnable failed\n");
		ErrorExit();
	}
	

	/* Tranfer data into FPGA block ram using Dstm */
	// DSTM API Call: DstmIO
	if(!DstmIO(hif, rgbOut, cbTx, NULL, 0, fFalse)) {
		printf("Error: DstmIO failed\n");
		ErrorExit();
	}

	/* Retrieve data from FPGA block ram using Dstm */
	// DSTM API Call: DstmIO
	if(!DstmIO(hif, NULL, 0, rgbIn, cbTx, fFalse)) {
		printf("Error: DstmIO failed\n");
		ErrorExit();
	}


	// Verify that recieved data matches transmitted data
	for(ibTx=0; ibTx<cbTx; ibTx++) {
		if(rgbIn[ibTx] != rgbOut[ibTx]) {
			fFail = fTrue;  // Set fFail if data mismatched
		}
	}

	// Print results of test
	if(fFail) {
		printf("Error: Recieved data did not match transmitted data\n");
		for(ibTx=0; ibTx<cbTx; ibTx++) {
			printf("rgbOut[%d]: %d      rgbIn[%d]: %d\n", ibTx, rgbOut[ibTx], ibTx, rgbIn[ibTx]);
		}
	}
	else {
		printf("Success: Recieved data matched transmitted data\n");
	}
	
	// DSTM API Call: DstmDisable
	if(!DstmDisable(hif)) {
		printf("Error: DstmDisable failed\n");
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

		// DSTM API Call: DstmDisable
		DstmDisable(hif);

		// DMGR API Call: DmgrClose
		DmgrClose(hif);
	}
	
	exit(1);
}
