/************************************************************************/
/*																		*/
/*  DspiDemo.cpp  --  DSPI DEMO Main Program							*/
/*																		*/
/************************************************************************/
/*  Author:	Aaron Odell													*/
/*  Copyright:	2010 Digilent, Inc.										*/
/************************************************************************/
/*  Module Description: 												*/
/*		DSPI Demo is a project to demonstrate how to transfer data		*/
/*		via SPI using the DSPI module of the Adept SDK.					*/
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
#include <string.h>
#include <stdlib.h>

#include "dpcdecl.h"
#include "dmgr.h"
#include "dspi.h"

/* ------------------------------------------------------------ */
/*				Local Type and Constant Definitions				*/
/* ------------------------------------------------------------ */

const int cchSzLen = 1024;

/* ------------------------------------------------------------ */
/*				Global Variables								*/
/* ------------------------------------------------------------ */


/* ------------------------------------------------------------ */
/*				Local Variables									*/
/* ------------------------------------------------------------ */

HIF hif = hifInvalid;


char szDvc[cchSzLen];
char szSend[cchSzLen];
char szByte[cchSzLen];
char szCount[cchSzLen];

BOOL fPutByte;
BOOL fPut;
BOOL fGet;

BOOL fDvc;
BOOL fString;
BOOL fByte;
BOOL fCount;

/* ------------------------------------------------------------ */
/*				Forward Declarations							*/
/* ------------------------------------------------------------ */

BOOL FParseParam(int cszArg, char* rgszArg[]);
void ShowUsage(char* szProgName);
void DoPutByte();
void DoPut();
void DoGet();
void ErrorExit();

void StrcpyS( char* szDst, size_t cchDst, const char* szSrc );

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

	if(! FParseParam(cszArg, rgszArg)) {
		ShowUsage(rgszArg[0]);
		ErrorExit();
	}

	// DMGR API Call: DmgrOpen
	if(!DmgrOpen(&hif, szDvc)) {
		printf("Error: Could not open device %s\n", szDvc);
		ErrorExit();
	}
	
	// DSPI API Call: DspiEnable
	if(!DspiEnable(hif)) {
		printf("Error: DspiEnable failed\n");
		ErrorExit();
	}
	

	if( fPutByte ) {
		DoPutByte();
	}
	else if( fPut ) {
		DoPut();
	}
	else if( fGet ) {
		DoGet();
	}
	else {
		printf("Error: No action specified\n");
		ShowUsage(rgszArg[0]);
	}

	if( hif != hifInvalid ) {
		// DSPI API Call: DspiDisable
		DspiDisable(hif);

		// DMGR API Call: DmgrClose
		DmgrClose(hif);
	}
}

/* ------------------------------------------------------------ */
/***	DoPutByte
**
**	Synopsis
**		void DoPutByte()
**
**	Input:
**		none
**
**	Output:
**		none
**
**	Errors:
**		none
**
**	Description:
**		Sends a byte via SPI. Expects MISO and MOSI to be shorted, so the 
**		same byte should be recieved. Compares the two bytes to ensure they
**		are equal. 
*/

void DoPutByte() {

	BYTE bSnd;
	BYTE bRcv;

	bSnd = atoi(szByte);

	//DSPI API Call: DspiPutByte
	if(!DspiPutByte(hif, fFalse, fFalse, bSnd, &bRcv, fFalse)) {
		printf("Error: DspiPutByte failed\n");
		ErrorExit();
	}
	
	printf("Recieved byte %d\n", bRcv);

	if(bSnd != bRcv) {
		printf("Error: Sent and recieved byte not equal. Check to ensure MOSI and MISO are shorted.\n");
	}
}

/* ------------------------------------------------------------ */
/***	DoPut
**
**	Synopsis
**		void DoPut()
**
**	Input:
**		none
**
**	Output:
**		none
**
**	Errors:
**		none
**
**	Description:
**		Sends the bytes (chars) from szSend via SPI. Expects MOSI and MISO
**		to be shorted. So the data recieved in szRcv is expected to be the
**		same as the data from szSend. Does a string compare at the end to
**		check the validitiy of the recieved string.
*/
void DoPut() {

	BYTE* bSend = (BYTE*) szSend;
	BYTE szRcv[cchSzLen];

	
	// DSPI API Call: DspiPut
	if(!DspiPut(hif, fFalse, fFalse, bSend, szRcv, strlen(szSend)+1, fFalse)) { // send strlen() + 1 characters also copy null terminator
		printf("Error: DspiPut failed\n");
		ErrorExit();
	}
	
	printf("Recieved string: %s\n", szRcv);

	if(strcmp(szSend, (char*) szRcv) != 0) {
		printf("Error: Sent and recieved strings not equal. Check to ensure MOSI and MISO are shorted.\n");
	}
}


/* ------------------------------------------------------------ */
/***	DoGet
**
**	Synopsis
**		void DoPut()
**
**	Input:
**		none
**
**	Output:
**		none
**
**	Errors:
**		none
**
**	Description:
**		Gets bytes from SPI. Still requires filler bytes to be sent.
**		Checks recieved data against fill byte.
*/
void DoGet() {

	BYTE bFill;
	BYTE* rgbRcv;
	int cbRcv;
	bool fFail = false;


	bFill = atoi(szByte);
	cbRcv= atoi(szCount);

	rgbRcv = (BYTE*) malloc(cbRcv);
	memset(rgbRcv, 0, cbRcv);


	// DSPI API Call: DspiGet
	if(!DspiGet(hif, fFalse, fFalse, bFill, rgbRcv, cbRcv, fFalse)) {
		printf("Error: DspiGet failed\n");
		free(rgbRcv);
		ErrorExit();	
	}

	for(int i=0; i<cbRcv; i++) {
		if(rgbRcv[i] != bFill) {
			fFail = true;
		}
	}

	if( fFail ) {
		printf("Warning: Recieved data did not match fill data. Ensure MOSI and MISO are shorted.\n");
	}
	else {
		printf("Success: Recieved data matched fill data.\n");
	}

	

	free(rgbRcv);
}

/* ------------------------------------------------------------ */
/***	FParseParam
**
**	Parameters:
**		cszArg		- number of command line arguments
**		rgszArg		- array of command line argument strings
**
**	Return Value:
**		none
**
**	Errors:
**		Returns fTrue if not parse errors, fFalse if command line
**		errors detected.
**
**	Description:
**		Parse the command line parameters 
*/

BOOL
FParseParam(int cszArg, char* rgszArg[]) {

	int		iszArg;

	/* Initialize default flag values */
	fPutByte	= fFalse;
	fPut		= fFalse;
	fGet		= fFalse;
	fCount		= fFalse;

	// Ensure sufficient paramaters. Need at least program name, action flag, device flag,
	// and device name
	if (cszArg < 4) {
		return fFalse;
	}
	
	/*	The rgszArg[1] is the action flag. Do sting
		comparisons to find and set the appropriate
		action flag 
	*/
	if(strcmp(rgszArg[1], "-pb") == 0) {
		fPutByte = fTrue;
	}
	else if( strcmp(rgszArg[1], "-p") == 0) {
		fPut = fTrue;
	}
	else if( strcmp(rgszArg[1], "-g") == 0) {
		fGet = fTrue;
	}
	else { // unrecognized action
		return fFalse;
	}


	/* Parse the command line parameters.
	*/
	iszArg = 2;
	while(iszArg < cszArg) {
	
		/* Check for the -d parameter which is used to specify
		** the device name of the device to query.
		*/
		if (strcmp(rgszArg[iszArg], "-d") == 0) {
			iszArg += 1;
			if (iszArg >= cszArg) {
				return fFalse;
			}
			StrcpyS(szDvc, cchSzLen, rgszArg[iszArg++]);
			fDvc = fTrue;
		}
		
		/* Check for the -str parameter used to specify the
		** string to send.
		*/
		else if (strcmp(rgszArg[iszArg], "-str") == 0) {
			iszArg += 1;
			if (iszArg >= cszArg) {
				return fFalse;
			}
			StrcpyS(szSend, cchSzLen, rgszArg[iszArg++]);
			fString = fTrue;
		}
		
		/* Check for the -b paramater used to specify the 
		** value of a single data byte to send 
		*/
		else if (strcmp(rgszArg[iszArg], "-b") == 0) {
			iszArg += 1;
			if (iszArg >= cszArg) {
				return fFalse;
			}
			StrcpyS(szByte, cchSzLen, rgszArg[iszArg++]);
			fByte = fTrue;
		}

		/* Check for the -c paramater used to specify the 
		** number of bytes to get 
		*/
		else if (strcmp(rgszArg[iszArg], "-c") == 0) {
			iszArg += 1;
			if (iszArg >= cszArg) {
				return fFalse;
			}
			StrcpyS(szCount, cchSzLen, rgszArg[iszArg++]);
			fCount = fTrue;
		}
		
		/* Not a recognized parameter
		*/ 
		else {
			return fFalse;
		}			
	}

	/* Input combination checks
	*/
	if(!fDvc) {
		printf("Error: No Device specified\n");
		return fFalse;
	}
	if( fPutByte && !fByte ) {
		printf("Error: No byte value specified\n");
		return fFalse;
	}
	if( fPut && !fString ) {
		printf("Error: No string specified\n");
		return fFalse;
	}
	if( fGet && ( !fCount || !fByte )  ) {
		printf("Error: Count or byte was unspecified\n");
		return fFalse;
	}
		
	return fTrue;
	
}

/* ------------------------------------------------------------ */
/***	ShowUsage
**
**	Parameters:
**		szProgName	- name of program as called 
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

void ShowUsage(char* szProgName) {

	printf("Error invalid paramaters: \n");
	printf("Usage: %s <action> -d <device> [options] \n", szProgName);
	
	printf("\nActions:\t\t\t\t\n");
	printf("\t-pb	Put Byte\tRequires -b <byte>\n");
	printf("\t-p	Put     \tRequires -str <string>\n");
	printf("\t-g	Get     \tRequires -c <# bytes> and -b <fill byte>\n");
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
**		Disables Dspi, closes device, and exits the program
*/

void ErrorExit() {
	if( hif != hifInvalid ) {
		// DSPI API Call: DspiDisable
		DspiDisable(hif);

		// DMGR API Call: DmgrClose
		DmgrClose(hif);
	}
	exit(1);
}

/* ------------------------------------------------------------ */
/***	StrcpyS
**
**	Parameters:
**		szDst - pointer to the destination string
**		cchDst - size of destination string
**		szSrc - pointer to zero terminated source string
**
**	Return Value:
**		none
**
**	Errors:
**		none
**
**	Description:
**		Cross platform version of Windows function strcpy_s.
*/
void StrcpyS( char* szDst, size_t cchDst, const char* szSrc ) {
	
#if defined (WIN32)

	strcpy_s(szDst, cchDst, szSrc);

#else
	
	if ( 0 < cchDst ) {
		
		strncpy(szDst, szSrc, cchDst - 1);
		szDst[cchDst - 1] = '\0';
	}
	
#endif
}


/* ------------------------------------------------------------ */

/************************************************************************/

