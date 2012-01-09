/************************************************************************/
/*																		*/
/*  DaioDemo.cpp  --  DAIO Demo Main Program							*/
/*																		*/
/************************************************************************/
/*  Author:	Aaron Odell													*/
/*  Copyright:	2010 Digilent, Inc.										*/
/************************************************************************/
/*  Module Description: 												*/
/*		DAIO Demo is a project to demonstrate how to measure an			*/
/*		analog input voltage using the DAIO Module of the Adept			*/
/*		SDK and an I/O Explorer board.									*/
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
#include "daio.h"

/* ------------------------------------------------------------ */
/*					Local Type and Constant Definitions			*/
/* ------------------------------------------------------------ */


/* ------------------------------------------------------------ */
/*					Global Variables							*/
/* ------------------------------------------------------------ */


/* ------------------------------------------------------------ */
/*					Local Variables								*/
/* ------------------------------------------------------------ */

HIF hif; // Device handle

char szDvc[128] = "ioexp";   // Device name
int prtReq = 0;    // Analog I/O port number
int	chn = 0;       // Channel number

int resAdc = 10;   // ADC resolution (in bits)
double vltRefReq = 3.3;  // ADC reference voltage to request

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
**		DaioDemo main function
*/
int main(void) {

	int smpAdc;   // Sample value from ACD
	double vltRefSet;  // Refernce voltage for ADC
	double vltBat;	// Calculated battery voltage


	/* Connect to device using DMGR */
	// DMGR API Call: Dmgr Open
	if(!DmgrOpen(&hif, szDvc)) {
		printf("Error: Could not open device %s\n", szDvc);
		ErrorExit();
	}
	
	/* Enable Daio on the port defined by prtReq. On a device with only one DAIO port, call DaioEnable, rather than DaioEnableEx */
	// DAIO API Call: DaioEnableEx
	if(!DaioEnableEx(hif, prtReq)) {
		printf("Error: DaioEnable failed\n");
		ErrorExit();
	}

	/* Enable channel chn on the Daio port associated with this handle (hif) */
	// DAIO API Call: DaioChannelEnable
	if(!DaioChannelEnable(hif, chn)) {
		printf("Error: DaioChannelEnable failed\n");
		ErrorExit();
	}

	/* Configure the reference voltage for the ADC. This should be larger than the voltage we expect to measure */
	// DAIO API Call: DaioSetReference
	if(!DaioSetReference(hif, vltRefReq, &vltRefSet)) {
		printf("Error: DaioSetReference failed\n");
		ErrorExit();
	}

	printf("Set Reference Voltage %f\n", vltRefSet);

	/* Get sample from DAIO device on the specified channel */
	// DAIO API Call: DaioGetSample
	if(!DaioGetSample(hif, chn, &smpAdc)) {
		printf("Error: DaioGetSample failed\n");
		ErrorExit();
	}

	printf("ADC Sample value: %d\n", smpAdc);

	/* Calculate actual voltage from ADC sample value */
	vltBat = vltRefSet * ( (float)smpAdc / (1 << resAdc) );

	printf("Battery Voltage: %.2f V\n", vltBat);

	// DAIO API Call: DaioDisable
	if(!DaioDisable(hif)) {
		printf("Error: DaioDisable failed\n");
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
		// DAIO API Call: DaioDisable
		DaioDisable(hif);

		// DMGR API Call: DmgrClose
		DmgrClose(hif);
	}

	exit(1);
}

/* ------------------------------------------------------------ */

/************************************************************************/

