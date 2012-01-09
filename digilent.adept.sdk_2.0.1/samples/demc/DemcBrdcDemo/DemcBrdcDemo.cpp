/************************************************************************/
/*																		*/
/*  DemcBrdcDemo.cpp  --  DEMC Brdc Demo Main Program					*/
/*																		*/
/************************************************************************/
/*  Author:	Aaron Odell													*/
/*  Copyright:	2010 Digilent, Inc.										*/
/************************************************************************/
/*  Module Description: 												*/
/*		DEMC Brdc Demo is a project to demonstrate how to control		*/
/*		a brushed DC motor connected to an I/O Explorer using the		*/
/*		DEMC module of the Adept SDK.									*/
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
	#include <errno.h>
	#include <time.h>

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

const int prtBrdc = 2;

/* ------------------------------------------------------------ */
/*					Global Variables							*/
/* ------------------------------------------------------------ */



/* ------------------------------------------------------------ */
/*					Local Variables								*/
/* ------------------------------------------------------------ */
HIF hif = hifInvalid;


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
**		0 if successful 
**		non-zero otherwise
**
**	Errors:
**		none
**
**	Description:
**		DemcBrdcDemo main function
*/
int main(void) {

	const int vel = 20000;

	/* Open device for communication */
	// DMGR API Call: DmgrOpen
	if(!DmgrOpen(&hif, szDvc)) {
		printf("Error: Could not open device %s\n", szDvc);
		ErrorExit();
	}

	/* Enable DEMC */
	// DEMC API Call: DemcEnableEx
	if(!DemcEnableEx(hif, prtBrdc)) { 
		printf("Error: DemcEnable failed\n");
		ErrorExit();
	}

	
	/* Set velocity */
	// DEMC API Call: DemcBrdcSetVel
	if(!DemcBrdcSetVel(hif, vel)) {
		printf("Error: DemcBrdcSetVel failed\n");
		ErrorExit();
	}

	printf("Motor should be running...\n");

	SleepMs(1000);

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

