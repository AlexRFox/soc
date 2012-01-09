/************************************************************************/
/*                                                                      */
/*  DemcSrvDemo.cpp --  DEMC Servo Demo Main Program					*/
/*                                                                      */
/************************************************************************/
/*	Author: 	Gene Apperson											*/
/*	Copyright:	2009 Digilent, Inc.										*/
/************************************************************************/
/*  Module Description: 												*/
/*                                                                      */
/*	Demc Servo Demo demonstrates use of the servo functions in the DEMC	*/
/*	interface of the Digilent Adept SDK using the I/O Explorer		*/
/*	board.																*/
/*																		*/
/************************************************************************/
/*  Revision History:													*/
/*                                                                      */
/*	08/04/2009(GeneA): created											*/
/*	04/29/2010(AaronO): Dividded demo routines into seperate functions	*/
/*                                                                      */
/************************************************************************/



/* ------------------------------------------------------------ */
/*					Include File Definitions					*/
/* ------------------------------------------------------------ */

#if defined(WIN32)
	
	/* Include Windows specific headers here.
	*/
	#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
	#include <windows.h>
	
#else

	/* Include Unix specific headers here.
	*/
	#include <errno.h>
	#include <time.h>

#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dpcdecl.h"
#include "dmgr.h"
#include "demc.h"

/* ------------------------------------------------------------ */
/*					Local Type Definitions						*/
/* ------------------------------------------------------------ */

/* ------------------------------------------------------------ */
/*					 Global Variables							*/
/* ------------------------------------------------------------ */

char		szMsg[1024];
HIF			hif;
int			cprt;
int			iprt;
DPRP		dprp;
int			ichn;
int			ichnMax = 8;
USHORT		tusWdt;
USHORT		tusMin;
USHORT		tusMax;
USHORT		tusCtr;
short		pos;
USHORT		vel;
USHORT		tusTmp;
short		posTmp;
USHORT		velTmp;
DWORD		dwMov;

bool fPulseWidth;
bool fAbsolutePos;
bool fRelativePos;
bool fVelocity;

/* ------------------------------------------------------------ */
/*					Local Variables								*/
/* ------------------------------------------------------------ */

/* Default name of the I/O Explorer board.
*/
char szDUT[] = { "ioexp" };

/* Number of servos connected */
const int cServos = 4;

/* ------------------------------------------------------------ */
/*					Forward Declarations						*/
/* ------------------------------------------------------------ */

bool FParseParams(int cszArg, char* rgszArg[]);
void ErrorExit();
bool FInit();
void DoPulseWidth();
void DoAbsolutePos();
void DoRelativePos();
void DoVelocity();
void DoTerminate();
void ShowUsage(char* progName);
void SleepMs( DWORD tms );


/* ------------------------------------------------------------ */
/*					Procedure Definitions						*/
/* ------------------------------------------------------------ */
/***	main
**
**	Parameters:
**		cszArg		- count of command line argument strings
**		rgszArg		- array of command lin argument strings
**
**	Return Value:
**		0 if successful 
**		non-zero otherwise
**
**	Errors:
**		none
**
**	Description:
**		DemcSrvDemo main function
*/

int main(int cszArg, char * rgszArg[]) {

	/* Parse paramaters */
	if(!FParseParams(cszArg, rgszArg)) {
		return 1;
	}

	/* Initialization routine */
	if(!FInit()) {
		printf("Error: Initialization failed\n");
		return 1;
	}

	/* Perform various program actions */
	if( fPulseWidth ) {
		DoPulseWidth();
	}
	if( fAbsolutePos ) {
		DoAbsolutePos();
	}
	if( fRelativePos ) {
		DoRelativePos();
	}
	if( fVelocity ) {
		DoVelocity();
	}

	/* Program termination */
	DoTerminate();

	return 0;
}

/* ------------------------------------------------------------ */
/***	FParseParams
**
**	Parameters:
**		cszArg	-	Number of argument strings
**		rgszArg	-	Array of argument strings
**
**	Return Value:
**		true if parsed successfuly
**		false otherwise
**
**	Errors:
**		none
**
**	Description:
**		Parses paramater strings and sets global flags
*/
bool FParseParams(int cszArg, char* rgszArg[]) {

	int iszArg; // Argument string index
	
	if( cszArg < 2 ) {
		printf("ERROR: No action specified\n");
		ShowUsage(rgszArg[0]);
		return false;
	}

	iszArg = 1; // Point to first argument after program name

	while( iszArg < cszArg ) {

		if( strcmp("-p", rgszArg[iszArg]) == 0 ) {
			fPulseWidth = true;
			iszArg++;
		}
		else if( strcmp("-a", rgszArg[iszArg]) == 0 ) {
			fAbsolutePos = true;
			iszArg++;
		}
		else if( strcmp("-r", rgszArg[iszArg]) == 0 ) {
			fRelativePos = true;
			iszArg++;
		}
		else if( strcmp("-v", rgszArg[iszArg]) == 0 ) {
			fVelocity = true;
			iszArg++;
		}
		else {
			printf("Error: Unrecognized argument\n");
			return false; 
		}
	}

	return true;
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
	if (hif != hifInvalid) {

		// DEMC API Call: DemcDisable
		DemcDisable(hif);

		// DMGR API Call: DmgrClose
		DmgrClose(hif);
	}	

	exit(1);
}

/* ------------------------------------------------------------ */
/***	FInit
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
**		Performs program initialization tasks
*/
bool FInit() {
	/* Open the device, find out how many DEMC ports there are and
	** then get their properties.*/
	// DMGR API Call: DmgrOpen
	if (!DmgrOpen(&hif, szDUT)) {
		printf("Unable to open device: %s\n", szDUT);
		return false;
	}	

	// DEMC API Call: DemcGetPortCount
	if (!DemcGetPortCount(hif, &cprt)) {
		printf("DemcGetPortCount failed\n");
		return false;
	}
	else {
		printf("  DEMC port count: %d\n", cprt);
	}

	/* For each port, get and report the port properties.
	*/
	for (iprt = 0; iprt < cprt; iprt++) {
		// DEMC API Call: DemcGetPortProperties
		DemcGetPortProperties(hif, iprt, &dprp);
		printf("  Port %d\n", iprt);
		printf("    Properties: %8.8X\n", dprp);
	}

	/* Enable the servo port
	*/
	iprt = 3;
	// DEMC API Call: DemcEnableEx
	if (DemcEnableEx(hif, iprt)) {
		printf("  DEMC port %d enabled\n", iprt);
	}
	else {
		printf("  Unabled to enable DEMC port %d\n", iprt);
		return false;
	}
	
	/* Report the channel min, center, max, and width values
	*/
	for (ichn = 0; ichn < ichnMax; ichn++) {
		// DEMC API Call: DemcServoGetMin
		DemcServoGetMin(hif, ichn, &tusMin);
		
		// DEMC API Call: DemcServoGetMax
		DemcServoGetMax(hif, ichn, &tusMax);

		// DEMC API Call: DemcServoGetCenter
		DemcServoGetCenter(hif, ichn, &tusCtr);

		// DEMC API Call: DemcServoGetWidth
		DemcServoGetWidth(hif, ichn, &tusWdt);

		printf("  Chan: %d, Min=%d, Max=%d, Center=%d, Width=%d\n",
					ichn, tusMin, tusMax, tusCtr, tusWdt);
		
		// DEMC API Call: DemcServoGetPos
		DemcServoGetPos(hif, ichn, &pos);

		// DEMC API Call: DemcServoGetVel
		DemcServoGetVel(hif, ichn, &vel);
		printf("           Pos=%d, Vel=%d\n", pos, vel);
	}
		
	/* Enable some of the servo channels.
	*/
	for (ichn = 0; ichn < ichnMax; ichn++) {
		// DEMC API Call: DemcServoChnEnable
		if (DemcServoChnEnable(hif, ichn)) {
			printf("  Servo Channel %d enabled\n", ichn);
		}
		else {
			printf("  Unable to enable servo channel %d\n", ichn);
			return false;
		}
	}
	
	// DEMC API Call: DemcServoSetWidth
	DemcServoSetWidth(hif, 0, 1000);

	return true;
}


/* ------------------------------------------------------------ */
/***	DoPulseWidth
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
**		Demonstrates servo control using the pulse width control functions
*/
void DoPulseWidth() {
	/* Set some servo pulse widths.
	*/
	printf("Setting absolute pulse width\n\n");
	for (ichn = 0; ichn < cServos; ichn++) {
		// DEMC API Call: DemcServoGetMin
		DemcServoGetMin(hif, ichn, &tusMin);

		// DEMC API Call: DemcServoGetMax
		DemcServoGetMax(hif, ichn, &tusMax);
	
		for (tusWdt = tusMin; tusWdt <= tusMax; tusWdt += 25) {
			// DEMC API Call: DemcServoSetWidth
			DemcServoSetWidth(hif, ichn, tusWdt);

			// DEMC API Call: DemcServoGetWidth
			DemcServoGetWidth(hif, ichn, &tusTmp);
			printf("  Chan: %d, SetWdt=%5d, GetWdt=%5d\n", ichn, tusWdt, tusTmp);
			SleepMs(100);
		}
		
		printf("\n");	
	}
	
}


/* ------------------------------------------------------------ */
/***	DoAbsolutePos
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
**		Demonstrates servo control using the absolute position functions
*/
void DoAbsolutePos() {

	/* Set some servo aboslute positions
	*/
	printf("Setting absolute servo position\n\n");
	for (ichn = 0; ichn < cServos; ichn++) {
		// DEMC API Call: DemcServoGetMin
		DemcServoGetMin(hif, ichn, &tusMin);

		// DEMC API Call: DemcServoGetMax
		DemcServoGetMax(hif, ichn, &tusMax);

		tusCtr = (tusMin + tusMax) / 2;

		// DEMC API Call: DemcServoSetCenter
		DemcServoSetCenter(hif, ichn, tusCtr);
		
		for (pos = (short)tusMin-(short)tusCtr; pos <= tusMax-tusCtr; pos += 25) {
			// DEMC API Call: DemcServoSetPosAbc
			DemcServoSetPosAbs(hif, ichn, pos);

			// DEMC API Call: DemcServoGetPos
			DemcServoGetPos(hif, ichn, &posTmp);

			// DEMC API Call: DemcServoGetWidth
			DemcServoGetWidth(hif,ichn, &tusTmp);
			printf("  Chan: %d, PosSet=%5d, PosGet=%5d, WdtGet=%5d\n",
						 ichn, pos, posTmp, tusTmp);
			SleepMs(100);
		}
		
		printf("\n");	
	}
	
	for (ichn = 0; ichn < cServos; ichn++) {
		// DEMC API Call: DemcServoGetMin
		DemcServoGetMin(hif, ichn, &tusMin);

		// DEMC API Call: DemcServoGetMax
		DemcServoGetMax(hif, ichn, &tusMax);

		tusCtr = (tusMin + tusMax) / 2;

		// DEMC API Call: DemcServoSetCenter
		DemcServoSetCenter(hif, ichn, tusCtr);
		
		for (pos = tusMax-tusCtr; pos >= (short)tusMin-(short)tusCtr; pos -= 25) {
			// DEMC API Call: DemcServoSetPosAbs
			DemcServoSetPosAbs(hif, ichn, pos);

			// DEMC API Call: DemcServoGetPos
			DemcServoGetPos(hif, ichn, &posTmp);

			// DEMC API Call: DemcServoGetWidth
			DemcServoGetWidth(hif,ichn, &tusTmp);
			printf("  Chan: %d, PosSet=%5d, PosGet=%5d, WdtGet=%5d\n",
						 ichn, pos, posTmp, tusTmp);
			SleepMs(100);
		}
		
		printf("\n");	
	}
}


/* ------------------------------------------------------------ */
/***	DoRelativePos
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
**		Demonstrates servo control using the relative position functions
*/
void DoRelativePos() {
	/* Set some servo relative positions
	*/
	printf("Setting relative servo position\n\n");
	for (ichn = 0; ichn < cServos; ichn++) {
		// DEMC API Call: DemcServoGetMin
		DemcServoGetMin(hif, ichn, &tusMin);

		// DEMC API Call: DemcServoGetMax
		DemcServoGetMax(hif, ichn, &tusMax);

		tusCtr = (tusMin + tusMax) / 2;

		// DEMC API Call: DemcServoSetCenter
		DemcServoSetCenter(hif, ichn, tusCtr);

		for (pos = (short)tusMin-(short)tusCtr; pos <= tusMax-tusCtr; pos += 25) {
			if (pos == ((short)tusMin-(short)tusCtr)) {
				// DEMC API Call: DemcServoSetPosAbs
				DemcServoSetPosAbs(hif, ichn, pos);
			}
			else {
				// DEMC API Call: DemcServoSetPosRel
				DemcServoSetPosRel(hif, ichn, 25);
			}

			// DEMC API Call: DemcServoGetPos
			DemcServoGetPos(hif, ichn, &posTmp);

			// DEMC API Call: DemcServoGetWidth
			DemcServoGetWidth(hif,ichn, &tusTmp);
			printf("  Chan: %d, PosSet=%5d, PosGet=%5d, WdtGet=%5d\n",
						 ichn, pos, posTmp, tusTmp);
			SleepMs(100);
		}
		
		printf("\n");	
	}
	
	for (ichn = 0; ichn < cServos; ichn++) {
		// DEMC API Call: DemcServoGetMin
		DemcServoGetMin(hif, ichn, &tusMin);

		// DEMC API Call: DemcServoGetMax
		DemcServoGetMax(hif, ichn, &tusMax);

		tusCtr = (tusMin + tusMax) / 2;

		// DEMC API Call: DemcServoSetCenter
		DemcServoSetCenter(hif, ichn, tusCtr);

		for (pos = tusMax-tusCtr; pos >= (short)tusMin-(short)tusCtr; pos -= 25) {
			if (pos == (tusMax-tusCtr)) {
				// DEMC API Call: DemcServoSetPosAbs
				DemcServoSetPosAbs(hif, ichn, pos);
			}
			else {
				// DEMC API Call: DemcServoSetPosRel
				DemcServoSetPosRel(hif, ichn, -25);
			}

			// DEMC API Call: DemcServoGetPos
			DemcServoGetPos(hif, ichn, &posTmp);

			// DEMC API Call: DemcServoGetWidth
			DemcServoGetWidth(hif,ichn, &tusTmp);
			printf("  Chan: %d, PosSet=%5d, PosGet=%5d, WdtGet=%5d\n",
						 ichn, pos, posTmp, tusTmp);
			SleepMs(100);
		}
		
		printf("\n");	
	}
}


/* ------------------------------------------------------------ */
/***	DoVelocity
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
**		Demonstrates servo control using the velocity control functions
*/
void DoVelocity() {
	int i;

	/* Try using servo velocity.
	*/
	printf(" Using servo velocity\n\n");
	
	ichn = 0;

	// DEMC API Call: DemcServoGetMin
	DemcServoGetMin(hif, ichn, &tusMin);

	// DEMC API Call: DemcServoGetMax
	DemcServoGetMax(hif, ichn, &tusMax);

	// DEMC API Call: DemcServoGetCenter
	DemcServoGetCenter(hif, ichn, &tusCtr);
	
	pos = (short)tusMin - (short)tusCtr;

	for(i=0; i<cServos; i++) {
		// DEMC API Call: DemcServoSetPosAbs
		DemcServoSetPosAbs(hif, ichn+i, pos);
	}

	// DEMC API Call: DemcServoGetPos
	DemcServoGetPos(hif, ichn, &posTmp);
	printf("  Start PosSet=%5d, PosGet=%5d\n", pos, posTmp);
	

	for(i=0; i<cServos; i++) {
		vel = (USHORT) ( i * 256 );

		// DEMC API Call: DemcServoSetVel
		DemcServoSetVel(hif, ichn+i, vel);
	}
	
	// DEMC API Call: DemcServoGetVel
	DemcServoGetVel(hif, ichn+i, &velTmp);
	printf("  VelSet= 0x%4.4X, VelGet= 0x%4.4X\n", vel, velTmp);
	
	pos = (short)tusMax - (short)tusCtr;

	for(i=0; i<cServos; i++) {
		// DEMC API Call: DemcServoSetPosAbs
		DemcServoSetPosAbs(hif, ichn+i, pos);
	}

	// DEMC API Call: DemcServoGetPos
	DemcServoGetPos(hif, ichn, &posTmp);
	printf("  End   PosSet=%5d, PosGet=%5d\n", pos, posTmp);

	while(fTrue) {
		// DEMC API Call: DemcServoGetMotion
		DemcServoGetMotion(hif, &dwMov);
		if (dwMov == 0) {
			break;
		}
	}

	for(i=0; i < cServos; i++) {
		// DEMC API Call: DemcServoSetVel
		DemcServoSetVel(hif, ichn+i, 0);
	}

	for(i=0; i < cServos; i++) {
		// DEMC API Call: DemcServoSetPosAbs
		DemcServoSetPosAbs(hif, ichn+i, 0);
	}

	SleepMs(500);
		
	pos = (short)tusMax - (short)tusCtr;

	for(i=0; i < cServos; i++) {
		// DEMC API Call: DemcServoSetPosAbs
		DemcServoSetPosAbs(hif, ichn+i, pos);
	}

	// DEMC API Call: DemcServoGetPos
	DemcServoGetPos(hif, ichn, &posTmp);
	printf("  Start PosSet=%5d, PosGet=%5d\n", pos, posTmp);
	SleepMs(500);

	for(i=0; i < cServos; i++) {
		vel = (USHORT) ( (cServos + 1 - i) * 256 );
		// DEMC API Call: DemcServoSetVel
		DemcServoSetVel(hif, ichn+i, vel);
	}

	// DEMC API Call: DemcServoGetVel
	DemcServoGetVel(hif, ichn+3, &velTmp);
	printf("  VelSet= 0x%4.4X, VelGet= 0x%4.4X\n", vel, velTmp);
	
	pos = (short)tusMin - (short)tusCtr;

	for(i=0; i < cServos; i++) {
		// DEMC API Call: DemcServoSetPosAbs
		DemcServoSetPosAbs(hif, ichn+i, pos);
	}

	// DEMC API Call: DemcServoGetPos
	DemcServoGetPos(hif, ichn, &posTmp);
	printf("  End   PosSet=%5d, PosGet=%5d\n", pos, posTmp);
	
	while(fTrue) {
		// DEMC API Call: DemcServoGetMotion
		DemcServoGetMotion(hif, &dwMov);
		if (dwMov == 0) {
			break;
		}
	}
	
	for(i=0; i < cServos; i++) {
		// DEMC API Call: DemcServoSetVel
		DemcServoSetVel(hif, ichn+i, 0);
	}	
}


/* ------------------------------------------------------------ */
/***	DoTerminate
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
**		Successful program cleanup tasks
*/
void DoTerminate() {

	// DEMC API Call: DemcDisable
	DemcDisable(hif);
	
	// DMGR API Call: DmgrClose
	if (!DmgrClose(hif)) {
		hif = hifInvalid;
		printf("DmgrClose failed\n");
	}
	
	return;
}


/* ------------------------------------------------------------ */
/***	ShowUsage
**
**	Parameters:
**		szProgName	-	Name of the program as called
**
**	Return Value:
**		none
**
**	Errors:
**		none
**
**	Description:
**		Shows proper usage of program arguments
*/
void ShowUsage(char* szProgName) {
	printf("\nUsage: %s [options]\n", szProgName);
	printf("Options:\n");
	printf("\t-p\tDemo pulse widths\n");
	printf("\t-a\tDemo absolute servo positions\n");
	printf("\t-r\tDemo relative servo positions\n");
	printf("\t-v\tDemo servo velocity controls\n");
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

