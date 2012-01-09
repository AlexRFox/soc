/************************************************************************/
/*                                                                      */
/*  DpioDemo.cpp	--  DPIO Demo Main Program							*/
/*                                                                      */
/************************************************************************/
/*	Author: 	Gene Apperson											*/
/*	Copyright:	2009 Digilent, Inc.										*/
/************************************************************************/
/*  Module Description: 												*/
/*                                                                      */
/* This project illustrates using the pin I/O API functions in the DPIO	*/
/* interface in the Digilent Adept system being used with an I/O		*/
/* Explorer board.														*/
/*																		*/
/*																		*/
/************************************************************************/
/*  Revision History:													*/
/*                                                                      */
/*	08/04/2009(GeneA): created											*/
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
#include "dpio.h"

/* ------------------------------------------------------------ */
/*					Local Type Definitions						*/
/* ------------------------------------------------------------ */

/* ------------------------------------------------------------ */
/*					 Global Variables							*/
/* ------------------------------------------------------------ */

/* ------------------------------------------------------------ */
/*					Local Variables								*/
/* ------------------------------------------------------------ */

HIF		hif;

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
**		error messages printed to console
**
**	Description:
**		Demonstration of DPIO operation on I/O Explorer board.
*/

int main() {

	DWORD	mskOutput;
	DWORD	mskInput;
	DWORD	fsDirSet;
	int		iprt;
	int		cprt;
	DWORD	dprp;	

	
	hif = hifInvalid;

	/* Open the device, find out how many PIO ports there are and
	** then get their properties.
	** Most access to a device requires an open interface handle
	** (HIF) on the device. An interface handle is analogous to
	** a file handle and is a token used to refer to an open
	** connection to a device.
	*/
	// DMGR API Call: DmgrOpen
	if (!DmgrOpen(&hif, szDUT)) {
		printf("Unable to open device: %s\n", szDUT);
		ErrorExit();
	}	

	/* Find out how many PIO ports the device supports. This is
	** useful for writing generic code to access PIO ports on any
	** device. If you know what device you are talking to, you can
	** know the number of ports by reading the documentation.
	** In that case this call is redundant.
	*/
	// DPIO API Call: DpioGetPortCount
	DpioGetPortCount(hif, &cprt);
	printf("  PIO port count: %d\n", cprt);

	/* For each port, get and report the port properties and
	** input and output masks.
	** This illustrates performing this operation. Again, if
	** you know what device you are talking to and the properties
	** of each of its ports, this is not necessary.
	*/	
	for (iprt = 0; iprt < cprt; iprt++) {
		// DPIO API Call: DpioGetPortProperties
		DpioGetPortProperties(hif, iprt, &dprp);
		printf("  Port %d\n", iprt);
		printf("    Properties: %8.8X\n", dprp);

		// DPIO API Call: DpioEnableEx
		DpioEnableEx(hif, iprt);

		// DPIO API Call: DpioGetPinMask
		DpioGetPinMask(hif, &mskOutput, &mskInput);
		printf("    Out Mask:   %8.8X\n", mskOutput);
		printf("    In Mask:    %8.8X\n", mskInput);		

		// DPIO API Call: DpioDisable
		DpioDisable(hif);
	}

	/* Drive some values on the Ports 0 - 3
	** These are Pmod connectors JA-JD. These are 8 bit ports.
	** This code sets and clears each bit on these ports
	** in succession, waiting about a half second on each
	** pin.
	*/
	for (iprt = 0; iprt < 4; iprt++) {
		/* Enable the specified port for access. A port must
		** be enabled before any calls that perform i/o on
		** the port can be made.
		*/
		// DPIO API Call: DpioEnableEx
		DpioEnableEx(hif, iprt);
		
		/* Set all of the pins on this port as outputs.
		** Set a bit to 1 to set that pin as an output pin,
		** and set it to 0 to make the pin be an input.
		** The value passed in the second parameter is the
		** requested pin directions. The value returned in *fsDirSet
		** below indicates the resulting pin direction, which will
		** be the same as requested unless the hardware can't support
		** what is requested. On the I/O explorer all pins can be set
		** independently to be inputs or outputs. In some cases, the
		** hardware may not support this, and so the result of setting
		** the direction may not be the same as what was requested.
		*/
		// DPIO API Call: DpioSetPinDir
		DpioSetPinDir(hif, 0x00FF, &fsDirSet);
		
		/* Set the each pin on the port high for a half second.
		*/
		// DPIO API Call: DpioSetPinState
		DpioSetPinState(hif, 0x01);
		SleepMs(500);

		// DPIO API Call: DpioSetPinState
		DpioSetPinState(hif, 0x02);
		SleepMs(500);

		// DPIO API Call: DpioSetPinState
		DpioSetPinState(hif, 0x04);
		SleepMs(500);

		// DPIO API Call: DpioSetPinState
		DpioSetPinState(hif, 0x08);
		SleepMs(500);

		// DPIO API Call: DpioSetPinState
		DpioSetPinState(hif, 0x010);
		SleepMs(500);

		// DPIO API Call: DpioSetPinState
		DpioSetPinState(hif, 0x020);
		SleepMs(500);

		// DPIO API Call: DpioSetPinState
		DpioSetPinState(hif, 0x040);
		SleepMs(500);

		// DPIO API Call: DpioSetPinState
		DpioSetPinState(hif, 0x080);
		SleepMs(500);

		// DPIO API Call: DpioSetPinState
		DpioSetPinState(hif, 0);
		
		/* Disable the port on this handle. A given handle can only
		** have one port enabled at a time. Disable the current port
		** so that the next port can be enabled.
		*/
		// DPIO API Call: DpioDisable
		DpioDisable(hif);	
	}

	/* Drive some values on port 4
	** This is Pmod connector JE, pins 1-4
	*/
	iprt = 4;
	// DPIO API Call: DpioEnableEx
	DpioEnableEx(hif, iprt);
	
	// DPIO API Call: DpioSetPinDir
	DpioSetPinDir(hif, 0x000F, &fsDirSet);
	
	// DPIO API Call: DpioSetPinState
	DpioSetPinState(hif, 0x01);
	SleepMs(500);

	// DPIO API Call: DpioSetPinState
	DpioSetPinState(hif, 0x02);
	SleepMs(500);

	// DPIO API Call: DpioSetPinState
	DpioSetPinState(hif, 0x04);
	SleepMs(500);

	// DPIO API Call: DpioSetPinState
	DpioSetPinState(hif, 0x08);
	SleepMs(500);

	// DPIO API Call: DpioSetPinState
	DpioSetPinState(hif, 0);

	// DPIO API Call: DpioDisable
	DpioDisable(hif);
	

	/* Drive some values on port 9
	** This is Pmod connector JE, pins 7-10
	*/
	iprt = 5;

	// DPIO API Call: DpioEnableEx
	DpioEnableEx(hif, iprt);
	
	// DPIO API Call: DpioSetPinDir
	DpioSetPinDir(hif, 0x000F, &fsDirSet);
	
	// DPIO API Call: SetPinState
	DpioSetPinState(hif, 0x01);
	SleepMs(500);
	
	// DPIO API Call: SetPinState
	DpioSetPinState(hif, 0x02);
	SleepMs(500);
	
	// DPIO API Call: SetPinState
	DpioSetPinState(hif, 0x04);
	SleepMs(500);
	
	// DPIO API Call: SetPinState
	DpioSetPinState(hif, 0x08);
	SleepMs(500);
	
	// DPIO API Call: SetPinState
	DpioSetPinState(hif, 0);
	
	// DPIO API Call: DpioDisable
	DpioDisable(hif);

	/* Drive some values on port 10
	** This is the 8 servo connectors accessed as a PIO port.
	*/	
	iprt = 6;

	// DPIO API Call: DpioEnableEx
	DpioEnableEx(hif, iprt);
	
	// DPIO API Call: DpioSetPinDir
	DpioSetPinDir(hif, 0x00FF, &fsDirSet);
	

	
	// DPIO API Call: SetPinState
	DpioSetPinState(hif, 0x01);
	SleepMs(500);
	
	// DPIO API Call: SetPinState
	DpioSetPinState(hif, 0x02);
	SleepMs(500);
	
	// DPIO API Call: SetPinState
	DpioSetPinState(hif, 0x04);
	SleepMs(500);
	
	// DPIO API Call: SetPinState
	DpioSetPinState(hif, 0x08);
	SleepMs(500);
	
	// DPIO API Call: SetPinState
	DpioSetPinState(hif, 0x010);
	SleepMs(500);
	
	// DPIO API Call: SetPinState
	DpioSetPinState(hif, 0x020);
	SleepMs(500);
	
	// DPIO API Call: SetPinState
	DpioSetPinState(hif, 0x040);
	SleepMs(500);
	
	// DPIO API Call: SetPinState
	DpioSetPinState(hif, 0x080);
	SleepMs(500);
	
	// DPIO API Call: SetPinState
	DpioSetPinState(hif, 0);
	
	
	// DPIO API Call: DpioDisable
	DpioDisable(hif);	

	/* Close the handle once access to the device is no longer required.
	*/
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
**		Closes open handle and exits the program
*/
void ErrorExit() {
	if(hif != hifInvalid) {

		// DPIO API Call: DpioDisable
		DpioDisable(hif);

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

