/************************************************************************/
/*                                                                      */
/*  DgioDemo.cpp --  DGIO DEMO Main Program								*/
/*                                                                      */
/************************************************************************/
/*	Author: 	Gene Apperson											*/
/*	Copyright:	2009 Digilent, Inc.										*/
/************************************************************************/
/*  Module Description: 												*/
/*                                                                      */
/*	This project illustrates used of the DGIO functions to interract	*/
/*	with the swiches, buttons, dip switches, and LEDs on the			*/
/*	I/O Explorer board.													*/
/*																		*/
/*	The I/O Explorer has the following GIO ports and channels			*/
/*		Port 0, chan 0	= LEDs, 16 bits, discrete out					*/
/*		Port 0, chan 1	= Slide Switches, 8 bits, discrete in			*/
/*		Port 0, chan 2	= Push Buttons, 4 bits, discrete in				*/
/*		Port 0, chan 3	= Dip Switches, 4 bits, discrete in				*/
/*																		*/
/************************************************************************/
/*  Revision History:													*/
/*                                                                      */
/*	08/05/2009(GeneA): created											*/
/*                                                                      */
/************************************************************************/

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

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
	#include <errno.h>
	#include <time.h>

#endif

#include <stdio.h>
#include <stdlib.h>

#include "dpcdecl.h"

#include "dmgr.h"
#include "dgio.h"

/* ------------------------------------------------------------ */
/*					Local Type Definitions						*/
/* ------------------------------------------------------------ */

const int	chnLed	= 0;
const int	chnSwt	= 1;
const int	chnBtn	= 2;
const int	chnDps	= 3;

/* ------------------------------------------------------------ */
/*					 Global Variables							*/
/* ------------------------------------------------------------ */

/* ------------------------------------------------------------ */
/*					Local Variables								*/
/* ------------------------------------------------------------ */
HIF			hif;


/* This is the default device name of the I/O Explorer board.
*/
char	szDUT[] = "ioexp";

/* ------------------------------------------------------------ */
/*					Forward Declarations						*/
/* ------------------------------------------------------------ */
void ErrorExit();

void SleepMs( DWORD tms );

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
**		Returns fTrue if successful, fFalse if not
**
**	Description:
**		Function used for general debugging of the AIO interface.
*/

int main() {

	int			cprt;
	int			prt;
	int			chn;
	DPRP		dprp;
	int			ival;
	DWORD		dwLed;
	DWORD		dwSwt;
	DWORD		dwBtn;
	DWORD		dwDps;
	DWORD		dwMask;

	/* Open the device, find out how many DGIO ports there are and
	** then get their properties.
	*/
	// DMGR API Call: DmgrOpen
	if (!DmgrOpen(&hif, szDUT)) {
		printf("Unable to open device: %s\n", szDUT);
		ErrorExit();
	}	

	// DGIO API Call: DgioGetPortCount
	if (DgioGetPortCount(hif, &cprt)) {
		printf("  DGIO port count: %d\n", cprt);
	}
	else {
		printf("DgioGetPortCount failed\n");
	}

	/* For each port, get and report the port properties. */
	for (prt = 0; prt < cprt; prt++) {
		// DGIO API Call: DgioGetPortProperties
		DgioGetPortProperties(hif, prt, &dprp);
		printf("  Port %d\n", prt);
		printf("    Properties: %8.8X\n", dprp);
	}

	/* Enable port with the LEDs, switches, buttons, etc. */
	prt = 0;
	// DGIO API Call: DgioEnableEx
	if (DgioEnableEx(hif, prt)) {
		printf("  DGIO port %d enabled\n", prt);
	}
	else {
		printf("  Unable to enable DGIO port %d\n", prt);
	}

	/* Test reading the bit masks from the channels on this port.
	*/	
	for (chn = 0; chn < 4; chn++) {
		// DGIO API Call: DgioGetDiscreteMask
		if (DgioGetDiscreteMask(hif, chn, &dwMask)) {
			printf("  DGIO Port 0, chan %d mask: %8.8X\n", chn, dwMask);
		}
		else {
			printf("  DgioGetDiscreteMask failed\n");
		}
	}
	
	/* Shift a '1' through the LEDs.
	*/
	dwLed = 1;
	for (ival = 0; ival < 16; ival++) {		
		// DGIO API Call: DgioPutData
		if (!DgioPutData(hif, 0, 0, &dwLed, 4, fFalse)) {
			printf("  DgioPutData to LEDs failed\n");
		}
		dwLed = dwLed << 1;
		SleepMs(500);
	}
	dwLed = 0;
	// DGIO API Call: DgioPutData
	DgioPutData(hif, 0, 0, &dwLed, 4, fFalse);

	/* Read the switches, buttons, and dip switches, and echo the
	** settings to the LEDs.
	** Get out when the setting on the switches is 0x0A
	*/
	printf("Echoing switches, buttons, and dip switch to LEDs\n");
	printf("  Set switches to 0x0A to exit\n");
		
	while(fTrue) {
		// DIGO API Call: DgioGetData
		if (!DgioGetData(hif, 1, 0, &dwSwt, 4, fFalse)) {		// get switches
			printf("  DgioGetData on switches failed\n");
		}
		// DGIO API Call: DgioGetData
		if (!DgioGetData(hif, 2, 0, &dwBtn, 4, fFalse)) {		// get buttons
			printf("  DgioGetData on buttons failed\n");
		}
		// DGIO API Call: DgioGetData
		if (!DgioGetData(hif, 3, 0, &dwDps, 4, fFalse)) {		// get slide switches
			printf("  DgioGetData on dip switch failed\n");
		}
		dwLed = dwSwt | (dwBtn << 8) | (dwDps << 12);
		// DGIO API Call: DgioPutData
		if (!DgioPutData(hif, 0, 0, &dwLed, 4, fFalse)) {
			printf("  DgioPutData on LEDs failed\n");
		}
		if ((dwSwt & 0x0F) == 0x0A) {
			break;
		}
	}

	dwLed = 0;

	// DGIO API Call: DgioPutData
	DgioPutData(hif, 0, 0, &dwLed, 4, fFalse);

	// DGIO API Call: DgioDisable
	if(!DgioDisable(hif)) {
		printf("Error: DgioDisable failed\n");
		ErrorExit();
	}
	
	/* Close the interface handle and get out.
	*/
	// DMGR API Call: DmgrClose
	if(!DmgrClose(hif)) {
		printf("DmgrClose failed");
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
**		Disables Dgio, closes the device, and exits the program
*/
void ErrorExit() {
	if(hif != hifInvalid) {

		// DGIO API Call: DgioDisable
		DgioDisable(hif);

		// DMGR API Call: DmgrClose
		DmgrClose(hif);
	}

	exit(1);
}


/* ------------------------------------------------------------ */
/***	SleepMs
**
**	Parameters:
**		tms - time in milliseconds to sleep
**
**	Return Value:
**		none
**
**	Errors:
**		none
**
**	Description:
**		Suspend the execution of the calling thread for the specified
**		number of milliseconds.
*/
void SleepMs( DWORD tms ) {

#if defined(WIN32)

	/* Windows implementation.
	*/
	
	return Sleep(tms);
	
#else

	/* Unix implementation.
	*/
	
	struct timespec timeSleep;
	struct timespec timeRemain;
	int    ret;
	
	timeSleep.tv_sec = (time_t)( tms / 1000 );
	timeSleep.tv_nsec = (long)(( tms % 1000 ) * 1000000 );
    
	ret = nanosleep(&timeSleep, &timeRemain);
	if ( 0 > ret ) {

		if ( EINTR == errno ) {

			/* The thread was woke by a signal before completing its
			** sleep cycle. Attempt to sleep again.
			*/
			timeSleep.tv_sec = timeRemain.tv_sec;
			timeSleep.tv_nsec = timeRemain.tv_nsec;
			ret = nanosleep(&timeSleep, &timeRemain);
			while (( 0 > ret ) && ( EINTR == errno )) {

				timeSleep.tv_sec = timeRemain.tv_sec;
				timeSleep.tv_nsec = timeRemain.tv_nsec;
				ret = nanosleep(&timeSleep, &timeRemain);
			}

			if ( 0 != ret ) {

				/* A system error occured.
				*/

				return;
			}
		}
		else if ( EINVAL == errno ) {

			/* The value of tv_nsec was out of range.
			*/

			return;
		}
		else {

			/* Some other type of system error occured.
			*/

			return;
		}
	}

	return;

#endif
}

/* ------------------------------------------------------------ */

/************************************************************************/

