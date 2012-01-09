/************************************************************************/
/*																		*/
/*  DemcStepDemo.cpp  --  DEMC Step Demo Main Program					*/
/*																		*/
/************************************************************************/
/*  Author:	Aaron Odell													*/
/*  Copyright:	2010 Digilent, Inc.										*/
/************************************************************************/
/*  Module Description: 												*/
/*		DEMC Step Demo is a project that demonstrates how to			*/
/*		control a stepper motor using an I/O Explorer board and			*/
/*		the DEMC module of the Adept SDK.								*/
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
#include "demc.h"

/* ------------------------------------------------------------ */
/*					Local Type and Constant Definitions			*/
/* ------------------------------------------------------------ */

char szDvc[128] = "ioexp";   // Device name


/* ------------------------------------------------------------ */
/*					Global Variables							*/
/* ------------------------------------------------------------ */



/* ------------------------------------------------------------ */
/*					Local Variables								*/
/* ------------------------------------------------------------ */
HIF hif = hifInvalid;

// Motor control paramater variables
double stvReq = 100;
INT32 stpReq = 200;

// Variables to be set by DEMC functions
double stvSet;
BOOL fMov;

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
**		main function
*/
int main(void)
{
	/* Open device for communication */
	// DMGR API Call: DmgrOpen
	if(!DmgrOpen(&hif, szDvc)) {
		printf("Error: Could not open device %s\n", szDvc);
		ErrorExit();
	}

	/* Enable DEMC */
	// DEMC API Call: DemcEnable
	if(!DemcEnable(hif)) { 
		printf("Error: DemcEnable failed\n");
		ErrorExit();
	}

	/* Set stepper motor turn rate */
	// DEMC API Call: DemcStepSetRate
	if(!DemcStepSetRate(hif, stvReq, &stvSet)) {
		printf("Error: DemcStepSetRate failed\n");
		ErrorExit();
	}

	/* Move stepper motor relative to current position */
	// DEMC API Call: DemcStepMoveRel
	if(!DemcStepMoveRel(hif, stpReq)) {
		printf("Error: DemcStepMoveRel failed\n");
		ErrorExit();
	}

	/* Wait for current move to complete */
	do {
		// DEMC API Call: DemcStepGetMotion
		if(!DemcStepGetMotion(hif, &fMov)) {
			printf("Error: DemcStepGetMotion failed\n");
			ErrorExit();
		}
	} while( fMov );


	// DEMC API Call: DemcDisable
	if(!DemcDisable(hif)) {
		printf("Error: DemcDisable failed\n");
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
**		Disables DEMC and closes the device handle (hif) if enabled, then exits the program
*/
void ErrorExit() {
	if(hif != hifInvalid) {
		// DEMC API Call: DemcDisable
		DemcDisable(hif);	

		// DMGR API Call: DmgrClose
		DmgrClose(hif);
	}

	exit(1);
}

/* ------------------------------------------------------------ */

/************************************************************************/

