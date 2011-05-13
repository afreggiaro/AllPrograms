/********************************************************************
 * COPYRIGHT --  
 ********************************************************************
 * Program: errors
 * File: errors.c
 * Author: charles
 * Created: June 30, 2008
 ********************************************************************
 * Implementation of program errors
 ********************************************************************/

#include <bur/plctypes.h>

#ifdef _DEFAULT_INCLUDES
 #include <AsDefault.h>
#endif
#include "header.h"


BOOL 	fncParseErrLine(USINT *usiLineData,UINT uiIndex);
USINT  	fncValidErrNumCharacter(USINT usiChar);
USINT  	fncValidErrTypeCharacter(USINT usiChar);
USINT   fncValidErrTextCharacter(USINT usiChar);
USINT   fncValidMachTypeCharacter(USINT usiChar);
void 	fncErrCheckModules(void);
void	fncModulesStructInit(void);
void 	fncErrCheckDrives(void);
void	fncDrivesStructInit(void);
void	fncShowErrorHelp(void);
void	fncWriteErr2History(STRING *strError);
void	fncClearErrHistory(void);
void 	fncRefreshErrHistDisplay(void);
USINT	*ptrErrFileData;
void	fncWriteHist2File(void);

added line 1

added another line

void _INIT errorINIT( void )
{
BOOL	blnLoop;
UINT	status_tmpalloc, i;
USINT	usiErrLineData[conMaxErrLineLen+10],usiGetLineResponse;

	udiSizeOfPrgStatus = sizeof(stPrgStatus[0]);/*Get Size of PrgStatus Structure Used to memcopy individual status structures to master structure*/

	uiErrBoxIndex = 0;
	uiMaxDisplayIndex = conMaxDisplayErrors -1;
	uiNumDisplayErrors = 0;
	uiErrDisplayIndex = 0;
	blnErrorsOk = conFalse;
	uiLocalLastMenu = 0;
	udiErrPtr 	=	(UDINT)	&Err[0];
	uiCurErrHistDisplayIndex	= uiCurErrHistIndex;
	
	for(i=0;i<conMaxNumErrors;i++)
	{
		Err[i] = conFalse;
		ErrOld[i] = conFalse;
	}

	for(i=0;i < conMaxDisplayErrors;i++)
	{
		/*Clear out the Display Error Array */
		strcpy(stErrorDisplay.strErrorDisplay[i], "SYSTEM OK");
		stErrorDisplay.uiErrInfoIndex[i]	= 0;
	}

	for(i=0;i<conNumDisplayHistErrors;i++)
		strcpy(strErrHistDisplay[i], "");

/*START: Open file and allocate temporary memmory */
	uiTestNumber 	= 0;
	blnLoop 		= conTrue;
	FOpen.enable 	= conTrue;
	FOpen.pDevice   = (UDINT) conErrorDevice;
	FOpen.pFile		= (UDINT) conErrorFileName;
	FOpen.mode 		= fiREAD_ONLY;

	while(blnLoop)
	{
		FileOpen(&FOpen);
		if (FOpen.status == conFubDone)
		{
			blnFileOpenOk = conTrue;
			blnLoop = conFalse;
			status_tmpalloc = TMP_alloc(FOpen.filelen, (void**)&ptrErrFileData);
		}
		else if(FOpen.status != conFubBusy)
		{
			/*Error openning Errors.csv file */
			if(!blnErr0Active)
			{
				Err[0] = conTrue;
				blnErr0Active = conTrue;
				strcpy(strErr0,"File System Err:Error Opening Error.csv");
			}
			fncSetSysErr(conTrue,FOpen.status, conBrErr,conFalse,conFalse,conTrue,conNoString);
			blnFileOpenOk = conFalse;
			blnLoop = conFalse;
		}
	}
/*END: Open File */

/*START: Read file  */
	if((blnFileOpenOk == conTrue) && (status_tmpalloc == 0))
	{
		FRead.enable	= conTrue;
		FRead.ident     = FOpen.ident;
		FRead.offset    = 0;
		FRead.pDest     = (UDINT)ptrErrFileData;
		FRead.len       = FOpen.filelen;
		blnLoop 		= conTrue;
	}
	while(blnLoop)
	{
		FileRead(&FRead);
		if(FRead.status == conFubDone)
		{
			blnLoop = conFalse;
			blnFileReadOk = conTrue;
		}
		else if(FRead.status != conFubBusy)
		{
			/*Error Reading UserGp file */
			if(!blnErr0Active)
			{
				Err[0] = conTrue;
				blnErr0Active = conTrue;
				strcpy(strErr0,"File System Err:Reading Error.csv");
			}
			blnLoop = conFalse;
			fncSetSysErr(conTrue,FRead.status, conBrErr,conFalse,conFalse,conTrue,conNoString);
			blnErrFileReadFailed = conTrue;
		}
	}
/*END: Read File */


/*START: Close file  */
	if(blnFileOpenOk == conTrue)
	{
	 	FClose.enable	= conTrue;
		FClose.ident 	= FOpen.ident;
		FileClose(&FClose);
		while(FClose.status == conFubBusy)
			FileClose(&FClose);
	}
	if(blnFileReadOk == conTrue)
	{
		blnLoop = conTrue;
		uiErrIndex = conNumSysErrors;
		udiCharCounter = 0;
		uiMaxErrIndex = 0;
	}
/*End: Close file  */

/*START DECODE ERROR FILE DATA AND LOAD the ErrInfo */
	
	while(blnLoop == conTrue)
	{

		usiGetLineResponse = fncGetLine(ptrErrFileData,usiErrLineData,&udiCharCounter,conMaxErrLineLen,FOpen.filelen);
		if(usiGetLineResponse == conGotLine)
		{
			if(uiErrIndex > (conMaxNumErrors-1))
			{
				blnLoop = conFalse;
				if(!blnErr0Active)
				{
					Err[0] = conTrue;
					blnErr0Active = conTrue;
					strcpy(strErr0,"Error System:1 Too Many Errors");
				}
				blnErrFileError[1] = conTrue;
				blnErrDecodeFailed = conTrue;
			}
			else
			{
				blnErrLineValid = fncParseErrLine(&usiErrLineData[0],uiErrIndex);
			}
			if(blnErrLineValid)
				uiErrIndex++;
			else
			{
				blnErrDecodeFailed = conTrue;
				blnLoop = conFalse;
			}
		}
		else if(usiGetLineResponse == conGotLineError)
		{
			blnLoop = conFalse;
			blnErrDecodeFailed = conTrue;
		}
		else if(usiGetLineResponse == conGotEOF)
		{
			blnLoop = conFalse;
			if(blnErrLineValid == conTrue)
				blnErrorsOk = conTrue;
		}
		for(i= 0; i <= uiMaxErrIndex;i++)
			KillErr[i] = ErrInfo[i].blnKill;
		
	}
	for(i=0;i<=conNumSysErrors;i++)
	{
		ErrInfo[i].blnBrError = conFalse;
		ErrInfo[i].blnDisplay = conTrue;
		ErrInfo[i].blnHistory = conFalse;
		ErrInfo[i].blnKill = conFalse;
		ErrInfo[i].blnMenu = conFalse;
		ErrInfo[i].blnMessage = conFalse;
		ErrInfo[i].blnTimed = conFalse;
		ErrInfo[i].blnWarning = conFalse;
		strcpy(ErrInfo[i].strMachine,"___");
		strcpy(ErrInfo[i].strErrNum,"___");
		strcpy(ErrInfo[i].strError,"___");
	}
	
	/*END DECODE ERROR FILE DATA AND LOAD the ErrInfo with data */
	
	/*Initialize module status to all OK*/
	fncModulesStructInit();	
	
	/*Initialize drives status to all OK*/
	fncDrivesStructInit();

	/*Get all text from the visualization text groups*/
	fncGetTxtByTxtGrp.blnQVGA		= QVGA;
	
	/*Start with getting the hardware error strings from txtMartinSysErrCommon text group*/
	fncGetTxtByTxtGrp.udiTxtGrpId	= 20;
	fncGetTxtByTxtGrp.uiReadLen		= 40;
	i		= 0;
	blnLoop	= conTrue;
	while(blnLoop == conTrue)
	{
		fncGetTxtByTxtGrp.udiTxtIndex	= i;
		fncGetTxtByTxtGrp.strText		= (UDINT) &strHdwrErrorStrings[i];
		fncGetTextByTextGroup(&fncGetTxtByTxtGrp);
		if(fncGetTxtByTxtGrp.uiStatus != conFubBusy);
			i++;
		if(conNumHdwrErrStrings < i)	
			blnLoop	= conFalse;
	}	

	/*Get the all strings from the Martin Errors text group*/
	fncGetTxtByTxtGrp.udiTxtGrpId	= 19;
	fncGetTxtByTxtGrp.uiReadLen		= 80;
	i		= 0;
	blnLoop	= conTrue;
	while(blnLoop == conTrue)
	{
		fncGetTxtByTxtGrp.udiTxtIndex	= i;
		fncGetTxtByTxtGrp.strText		= (UDINT) &strMartinErrors[i];
		fncGetTextByTextGroup(&fncGetTxtByTxtGrp);
		if(fncGetTxtByTxtGrp.uiStatus != conFubBusy);
			i++;
		if(conNumMartinErrors < i)	
			blnLoop	= conFalse;
	}

}

void _CYCLIC errorCYCLIC( void )
{
	DTGetTime_typ 						stCurrentTime;
	INT									i;
	DATE_AND_TIME 						dtErrorDateNTime;
	STRING								strDateString[25];

	if(blnErrorsOk == conFalse)/*if errors or not ok exit routine*/
		return;

	if(uiCurrentMenu == menStatus && blnErrorHelpRq)
	{
		fncShowErrorHelp();
		blnErrorHelpRq	= conFalse;
	}
	
	if(blnClearErrHistRq)
	{
		fncClearErrHistory();
		blnClearErrHistRq	= conFalse;
	}

	if(Err[0] == conFalse)
		blnErr0Active = 0;
		
	if(uiLocalLastMenu != uiCurrentMenu)
	{
		blnSystemMenuChange = conTrue;
		uiLocalLastMenu = uiCurrentMenu;
		uiErrBoxIndex = 0;
	}
	else
		blnSystemMenuChange = conFalse;

	if(uiCurrentMenu == menHistory)
	{		
		if(blnSystemMenuChange)
			blnHistTop	= conTrue;
		fncRefreshErrHistDisplay();
	}
	
    /*Get the Current Time*/
    stCurrentTime.enable = conTrue;
    DTGetTime(&stCurrentTime); /*Call Function to Get The Current Time*/
    dtErrorDateNTime = stCurrentTime.DT1;
	ascDT(dtErrorDateNTime,(UDINT) &strDateString, 25);

	/*Reset only drive errors if a request is made*/
	if(blnResetDriveErrsRq)
	{
		blnResetDriveErrsRq	= conFalse;
		for(i=0;i<=conNumSysErrors;i++)
		{
			if(ErrInfo[i].usiType == conInvDriveErr)
				Err[i]		= conFalse;
		}
	}
	
	if(blnMasterErrReset)
		blnMasterErrReset = conFalse;
	if(blnResetErrorsRq == conTrue)      /* if Reset Request set all errors to 0 */
	{
		for(i = 0;i <= conNumSysErrors; i++)
			Err[i] = conFalse;
		blnResetErrorsRq = conFalse;
		blnMasterErrReset = conTrue;
	}

	/*Only add new errors if not making the error history file*/
	if(!blnWriteErrHistBusy)
	{
		for(i=0;i < conMaxDisplayErrors;i++)
		{
			/*Clear out the Display Error Array */
			strcpy(stErrorDisplay.strErrorDisplay[i], "SYSTEM OK");
			stErrorDisplay.uiErrInfoIndex[i]	= 0;
		}
	
		/*Navigation code for active errors*/
		if(blnNextErrorSet || blnPrevErrorSet)
			intDisplayErrStartIndex	+= (INT)(blnNextErrorSet*conMaxDisplayErrors) - (INT)(blnPrevErrorSet*conMaxDisplayErrors);
		if((intDisplayErrStartIndex + conMaxDisplayErrors) > uiTotalNumOfActiveErrors)
			intDisplayErrStartIndex	= (INT)(uiTotalNumOfActiveErrors) - (INT)(conMaxDisplayErrors);/*Show up to the last active error*/
		if(intDisplayErrStartIndex < 0)
			intDisplayErrStartIndex	= 0;
		blnNextErrorSet				= conFalse;
		blnPrevErrorSet				= conFalse;
		/***********************************/
	
		uiTotalNumOfActiveErrors	= 0;/*Reset total number of active errors*/
		blnGotNewTimedErr 			= conFalse;
		uiNumDisplayErrors			= 0;
	
		for(i= 0; i <= uiMaxErrIndex;i++)
		{
				
			blnNewError = conFalse;
			if(Err[i] == conTrue )
			{
				intI = i;
				if((i == 0) && (Err[0] == conTrue))
					strcpy(ErrInfo[0].strError,strErr0);

				if( (ErrInfo[i].blnTimed == conTrue) && (blnClearTimedErrors == conTrue) ) /* Check to See if need To Reset Timed Error */
					Err[i] = conFalse;
				else if( (blnSystemMenuChange == conTrue) && (ErrInfo[i].blnMenu == conTrue))
					Err[i] = conFalse;
				else
				{
					if((ErrInfo[i].blnDisplay) || (i <= conNumSysErrors))
					{
						uiTotalNumOfActiveErrors++;/*Count all displayable errors*/

						if((intDisplayErrStartIndex + 1 <= uiTotalNumOfActiveErrors) && (intDisplayErrStartIndex + conMaxDisplayErrors >= uiTotalNumOfActiveErrors))
						{
							if(uiNumDisplayErrors < conMaxDisplayErrors) /* Put Error into Display Array if Appropriate */
							{
								strcpy(stErrorDisplay.strErrorDisplay[uiNumDisplayErrors], ErrInfo[i].strError);
								stErrorDisplay.uiErrInfoIndex[uiNumDisplayErrors]	= i;
								uiNumDisplayErrors++;
							}
						}
					}

					if(ErrOld[i] == conFalse)
						blnNewError = conTrue;
					if((blnNewError == conTrue) && 	(ErrInfo[i].blnTimed == conTrue))
					{
						blnGotNewTimedErr = conTrue;
						blnTimedErrorRq = conTrue;
					}
					if((blnNewError == conTrue) && (ErrInfo[i].blnHistory == conTrue))
					{
						strcpy(strTempHistErr,strDateString);
						strcat(strTempHistErr,"->");
						strcat(strTempHistErr,ErrInfo[i].strError);
						fncWriteErr2History(strTempHistErr);
						if(blnWriteHist2FileRq)
							break;/*Stop putting errors in the list and wait for the write to finish*/
					}
				}
			}
			ErrOld[i] = Err[i];
		}
	}
	
	blnClearTimedErrors = conFalse;
	ErrTimer.PT = Gp.tmTimedErrorTime;
	ErrTimer.IN = (blnTimedErrorRq && !blnGotNewTimedErr);
	TON(&ErrTimer);
	if(ErrTimer.Q)
	{
		blnTimedErrorRq = conFalse;
		blnClearTimedErrors = conTrue;
	}

	if(uiNumDisplayErrors <= 1) /* ROTATE THROUGH THE CURRENT ERRORS*/
	{
		uiErrDisplayIndex = 0;
		ErrCycleTimer.IN = conFalse;
		TON(&ErrCycleTimer);
	}
	else
	{
		ErrCycleTimer.PT = Gp.tmErrDisplayTime;
		ErrCycleTimer.IN = conTrue;
		TON(&ErrCycleTimer);
		if(ErrCycleTimer.Q == conTrue)
		{
			ErrCycleTimer.IN = conFalse;
			TON(&ErrCycleTimer);
			uiErrDisplayIndex++;
		}
		if(uiErrDisplayIndex >= uiNumDisplayErrors)
			uiErrDisplayIndex = 0;
	}
	
	/*Error check modules and I/O*/
	fncErrCheckModules();	
	
	/*Error check drives*/
	fncErrCheckDrives();
	
	/*Writes error history to file if a write request has been made*/	
	fncWriteHist2File();		
	
}
/********************************************************* END CYCLIC ROUTINE ************************************************/
/********************************************************* END CYCLIC ROUTINE ************************************************/

BOOL fncParseErrLine(USINT *usiLineData,UINT uiIndex)
{


	BOOL 	blnLoopLocal,blnMachTypeOk,blnErrTextOk,blnErrMenuOk,blnErrHistoryOk,blnErrTimedOk,blnErrKillOk,blnErrDisplayOk,blnErrTypeOk,blnErrNumOk;
	BOOL	blnKill,blnTimed,blnHistory,blnMenu,blnBrError,blnWarning,blnMessage,blnDisplay;
	STRING	strErrNum[10],strErrType[10],strErrDescription[conMaxErrDescriptionLen+2],strMachType[10];
	USINT 	usiErrChar,usiResponse;
	INT		i,j;

	blnErrNumOk		= conFalse;
	blnErrTypeOk	= conFalse;
	blnErrDisplayOk	= conFalse;
	blnErrKillOk	= conFalse;
	blnErrTimedOk	= conFalse;
	blnErrHistoryOk	= conFalse;
	blnErrMenuOk	= conFalse;
	blnErrTextOk	= conFalse;
	
	blnKill			= conFalse;
	blnTimed		= conFalse;
	blnHistory		= conFalse;
	blnDisplay		= conFalse;
	blnMenu			= conFalse;
	blnBrError		= conFalse;
	blnWarning		= conFalse;
	blnMessage		= conFalse;
	i=0;
	j=0;
	blnLoopLocal = conTrue;

	while(blnLoopLocal)
	{
		usiErrChar = usiLineData[i];
		usiResponse = fncValidErrNumCharacter(usiErrChar);
		if(usiResponse == conValid)
		{
			strErrNum[j] = usiErrChar;
			i++;
			j++;
			if((j > conMaxErrNumLen) || (i>conMaxErrLineLen))
			{
				blnLoopLocal = conFalse;
				if(!blnErr0Active)
				{
					Err[0] = conTrue;
					blnErr0Active = conTrue;
					strcpy(strErr0,"Error System:3 Err Number to Long");
				}
				blnErrFileError[3] = conTrue;
				return(conFalse);
			}
		}
		else if(usiResponse == conGotSeperator)
		{
			/*strErrNum[j] = conAsciiColon;*/
			strErrNum[j] = conAsciiNull;
			blnErrNumOk = conTrue;
			blnLoopLocal = conFalse;
			i++;
			j = 0;
			uiCurErrNum = uiIndex;	
			if(uiIndex >= conMaxNumErrors)
			{
				blnErrNumOk = conFalse;
				if(!blnErr0Active)
				{
					Err[0] = conTrue;
					blnErr0Active = conTrue;
					strcpy(strErr0,"Error System:19 Err Number Out of Range ");
				}
				blnErrFileError[19] = conTrue;
				return(conFalse);
				
			}
		}
		else /*InValid*/
		{
			blnLoopLocal = conFalse;
				if(!blnErr0Active)
				{
					Err[0] = 1;
					blnErr0Active = conTrue;
					strcpy(strErr0,"Error System:4 ErrNum Invalid Char");
				}
				blnErrFileError[4] = conTrue;
		}
	}
	if(blnErrNumOk)
		blnLoopLocal = conTrue;

	while(blnLoopLocal)
	{
		usiErrChar = usiLineData[i];
		usiResponse = fncValidErrTypeCharacter(usiErrChar);
		if(usiResponse == conValid)
		{
			strErrType[j] = usiErrChar;
			i++;
			j++;
			if((j > conMaxErrTypeLen)||(i > conMaxErrLineLen))
			{ /*Err to Long or Max Error Line Length Reached*/
				blnLoopLocal = conFalse;
				if(!blnErr0Active)
				{
					Err[0] = 1;
					blnErr0Active = conTrue;
					strcpy(strErr0,"Error System:5 ErrType Too Long");
				}
				blnErrFileError[5] = conTrue;
				return(conFalse);
			}
		}
		else if(usiResponse == conGotSeperator)
		{
			strErrType[j] = conAsciiNull;
			blnBrError = conFalse;
			blnWarning = conFalse;
			blnMessage = conFalse;
			if(strncmp("BR",strErrType,strlen("BR")) == 0)
				blnBrError = conTrue;
			else if(strncmp("WRN",strErrType,strlen("WRN")) == 0)
				blnWarning = conTrue;
			else if(strncmp("MSG",strErrType,strlen("MSG")) == 0)
				blnMessage = conTrue;
			blnErrTypeOk = conTrue;
			j=0;
			blnLoopLocal = conFalse;
			i++;
		}
		else /*InValid*/
		{
			blnLoopLocal = conFalse;
			if(!blnErr0Active)
			{
				Err[0] = 1;
				blnErr0Active = conTrue;
				strcpy(strErr0,"Error System:6 ErrType Invalid Char");
			}
			blnErrFileError[6] = conTrue;
		}
	}
	if(blnErrTypeOk)
		blnLoopLocal = conTrue;
	while(blnLoopLocal)
	{
		usiErrChar = usiLineData[i];
		usiResponse = fncValidErrNumCharacter(usiErrChar);
		if(usiResponse == conValid)
		{
			i++;
			j++;
			if(usiErrChar == '1')
				blnDisplay = conTrue;
			else
				blnDisplay = conFalse;
			if((j > 1)||(i > conMaxErrLineLen))
			{ /*Err to Long or Max Error Line Length Reached*/
				blnLoopLocal = conFalse;
				if(!blnErr0Active)
				{
					Err[0] = 1;
					blnErr0Active = conTrue;
					strcpy(strErr0,"Error System:7 blnDisplay Too Long");
				}
				blnErrFileError[7] = conTrue;
				return(conFalse);
			}
		}
		else if(usiResponse == conGotSeperator)
		{
			i++;
			j=0;
			blnErrDisplayOk = conTrue;
			blnLoopLocal = conFalse;
		}
		else /*InValid*/
		{
			blnLoopLocal = conFalse;
			if(!blnErr0Active)
			{
				Err[0] = 1;
				blnErr0Active = conTrue;
				strcpy(strErr0,"Error System:8 blnDisplay Invalid Char");
			}
			blnErrFileError[8] = conTrue;
		}
	}
	if(blnErrDisplayOk)
		blnLoopLocal = conTrue;
	while(blnLoopLocal)
	{
		usiErrChar = usiLineData[i];
		usiResponse = fncValidErrNumCharacter(usiErrChar);
		if(usiResponse == conValid)
		{
			i++;
			j++;
			if(usiErrChar == '1')
				blnKill = conTrue;
			else
				blnKill = conFalse;
			if((j > 1)||(i > conMaxErrLineLen))
			{ /*Err to Long or Max Error Line Length Reached*/
				blnLoopLocal = conFalse;
				if(!blnErr0Active)
				{
					Err[0] = 1;
					blnErr0Active = conTrue;
					strcpy(strErr0,"Error System:9 blnKill Too Long");
				}
				blnErrFileError[9] = conTrue;
				return(conFalse);
			}
		}
		else if(usiResponse == conGotSeperator)
		{
			i++;
			j=0;
			blnErrKillOk = conTrue;
			blnLoopLocal = conFalse;
		}
		else /*InValid*/
		{
			blnLoopLocal = conFalse;
			if(!blnErr0Active)
			{
				Err[0] = 1;
				blnErr0Active = conTrue;
				strcpy(strErr0,"Error System:10 blnKill Invalid char");
			}
			blnErrFileError[10] = conTrue;
		}
	}
	if(blnErrKillOk)
		blnLoopLocal = conTrue;

	while(blnLoopLocal)
	{
		usiErrChar = usiLineData[i];
		usiResponse = fncValidErrNumCharacter(usiErrChar);
		if(usiResponse == conValid)
		{
			i++;
			j++;
			if(usiErrChar == '1')
				blnTimed = conTrue;
			else
				blnTimed = conFalse;
			if((j > 1)||(i > conMaxErrLineLen))
			{ /*Err to Long or Max Error Line Length Reached*/
				blnLoopLocal = conFalse;
				if(!blnErr0Active)
				{
					Err[0] = 1;
					blnErr0Active = conTrue;
					strcpy(strErr0,"Error System:11 blnTimed Too Long");
				}
				blnErrFileError[11] = conTrue;
				return(conFalse);
			}
		}
		else if(usiResponse == conGotSeperator)
		{
			i++;
			j=0;
			blnErrTimedOk = conTrue;
			blnLoopLocal = conFalse;
		}
		else /*InValid*/
		{
			blnLoopLocal = conFalse;
			if(!blnErr0Active)
			{
				Err[0] = 1;
				blnErr0Active = conTrue;
				strcpy(strErr0,"Error System:12 blnTimed Invalid Char");
			}
			blnErrFileError[12] = conTrue;
		}
	}
	if(blnErrTimedOk)
		blnLoopLocal = conTrue;
	while(blnLoopLocal)
	{
		usiErrChar = usiLineData[i];
		usiResponse = fncValidErrNumCharacter(usiErrChar);
		if(usiResponse == conValid)
		{
			i++;
			j++;
			if(usiErrChar == '1')
				blnHistory = conTrue;
			else
				blnHistory = conFalse;
			if((j > 1)||(i > conMaxErrLineLen))
			{ /*Err to Long or Max Error Line Length Reached*/
				blnLoopLocal = conFalse;
				if(!blnErr0Active)
				{
					Err[0] = 1;
					blnErr0Active = conTrue;
					strcpy(strErr0,"Error System:13 blnHistory Too Long");
				}
				blnErrFileError[13] = conTrue;
				return(conFalse);
			}
		}
		else if(usiResponse == conGotSeperator)
		{
			i++;
			j=0;
			blnErrHistoryOk = conTrue;
			blnLoopLocal = conFalse;
		}
		else /*InValid*/
		{
			blnLoopLocal = conFalse;
			if(!blnErr0Active)
			{
				Err[0] = 1;
				blnErr0Active = conTrue;
				strcpy(strErr0,"Error System:14 blnHistory Invalid Char");
			}
			blnErrFileError[14] = conTrue;
		}
	}
	if(blnErrHistoryOk)
		blnLoopLocal = conTrue;
	while(blnLoopLocal)
	{
		usiErrChar = usiLineData[i];
		usiResponse = fncValidErrNumCharacter(usiErrChar);
		if(usiResponse == conValid)
		{
			i++;
			j++;
			if(usiErrChar == '1')
				blnMenu = conTrue;
			else
				blnMenu = conFalse;
			if((j > 1)||(i > conMaxErrLineLen))
			{ /*Err to Long or Max Error Line Length Reached*/
				blnLoopLocal = conFalse;
				if(!blnErr0Active)
				{
					Err[0] = 1;
					blnErr0Active = conTrue;
					strcpy(strErr0,"Error System:15 blnMenu Too Long");
				}
				blnErrFileError[15] = conTrue;
				return(conFalse);
			}
		}
		else if(usiResponse == conGotSeperator)
		{
			i++;
			j=0;
			blnErrMenuOk = conTrue;
			blnLoopLocal = conFalse;
		}
		else /*InValid*/
		{
			blnLoopLocal = conFalse;
			if(!blnErr0Active)
			{
				Err[0] = 1;
				blnErr0Active = conTrue;
				strcpy(strErr0,"Error System:16 blnMenu Invlaid Char");
			}
			blnErrFileError[16] = conTrue;
		}
	}
	if(blnErrMenuOk)
		blnLoopLocal = conTrue;

	while(blnLoopLocal)
	{
		usiErrChar = usiLineData[i];
		usiResponse = fncValidMachTypeCharacter(usiErrChar);
		if(usiResponse == conValid)
		{
			strMachType[j] = usiErrChar;
			i++;
			j++;
			if((j > conMaxMachTypeLen)||(i > conMaxErrLineLen))
			{ /*Err to Long or Max Error Line Length Reached*/
				blnLoopLocal = conFalse;
				if(!blnErr0Active)
				{
					Err[0] = 1;
					blnErr0Active = conTrue;
					strcpy(strErr0,"Error System:19 MachType Too Long");
				}
				blnErrFileError[19] = conTrue;
				return(conFalse);
			}
		}
		else if(usiResponse == conGotSeperator)
		{
 			/*strMachType[j] = conAsciiUnderScore;*/
			strMachType[j] = conAsciiNull;
			i++;
			j=0;
			blnMachTypeOk = conTrue;
			blnLoopLocal = conFalse;
		}
		else /*InValid*/
		{
			usiBadChar = usiErrChar;
			blnLoopLocal = conFalse;
			if(!blnErr0Active)
			{
				Err[0] = 1;
				blnErr0Active = conTrue;
				strcpy(strErr0,"Error System:20 MachType Invlaid Char");
			}
			blnErrFileError[20] = conTrue;
		}
	}
	if(blnMachTypeOk)
		blnLoopLocal = conTrue;

	while(blnLoopLocal)
	{
		usiErrChar = usiLineData[i];
		usiResponse = fncValidErrTextCharacter(usiErrChar);
		if(usiResponse == conValid)
		{
			strErrDescription[j] = usiErrChar;
			i++;
			j++;
			if((j > conMaxErrDescriptionLen)||(i > conMaxErrLineLen))
			{ /*Err to Long or Max Error Line Length Reached*/
				blnLoopLocal = conFalse;
				if(!blnErr0Active)
				{
					Err[0] = 1;
					blnErr0Active = conTrue;
					strcpy(strErr0,"Error System:17 Err Description too Long");
				}
				blnErrFileError[17] = conTrue;
				return(conFalse);
			}
		}
		else if(usiResponse == conGotSeperator)
		{
			strErrDescription[j] = conAsciiNull;
			blnErrTextOk = conTrue;
			blnLoopLocal = conFalse;
			i++;
			j=0;
		}
		else /*InValid*/
		{
			blnLoopLocal = conFalse;
			if(!blnErr0Active)
			{
				Err[0] = 1;
				blnErr0Active = conTrue;
				strcpy(strErr0,"Error System:18 Err Description Invalid Char");
			}
			blnErrFileError[18] = conTrue;
		}
	}
	
	if(blnErrTextOk)
	{
		if(uiIndex > uiMaxErrIndex)
			uiMaxErrIndex = uiIndex;
		ErrInfo[uiIndex].blnBrError = blnBrError;
		ErrInfo[uiIndex].blnDisplay = blnDisplay;
		ErrInfo[uiIndex].blnHistory = blnHistory;
		ErrInfo[uiIndex].blnKill = blnKill;
		ErrInfo[uiIndex].blnMenu = blnMenu;
		ErrInfo[uiIndex].blnMessage = blnMessage;
		ErrInfo[uiIndex].blnTimed = blnTimed;
		ErrInfo[uiIndex].blnWarning = blnWarning;
		strcpy(ErrInfo[uiIndex].strMachine,strMachType);
		strcpy(ErrInfo[uiIndex].strErrNum,strErrNum);
		strcpy(ErrInfo[uiIndex].strError,strMachType);
		strcat(ErrInfo[uiIndex].strError,"_");
		if(blnBrError)
			strcat(ErrInfo[uiIndex].strError,"BR_");
		else if(blnWarning)
			strcat(ErrInfo[uiIndex].strError,"WRN_");
		else if(blnMessage)
			strcat(ErrInfo[uiIndex].strError,"MSG_");
		else
			strcat(ErrInfo[uiIndex].strError,"ERR_");

		strcat(ErrInfo[uiIndex].strError,strErrNum);
		strcat(ErrInfo[uiIndex].strError,":");
		strcat(ErrInfo[uiIndex].strError,strErrDescription);
		return(conTrue);
	}
	return(conFalse);
}
/*************** End Function fncParseGpLine ******************/

/*************** Function fncValidErrTextCharater***********************/
/* Returns Valid,Invalid or Seperator								*/
/****************************************************************/

USINT  fncValidErrTextCharacter(USINT usiChar)
{
	if(usiChar == conAsciiSeperator)
		return(conGotSeperator);
	if(usiChar == conAsciiNull)
		return(conGotSeperator);
	return(conValid);
}
/*************** Function fncValidErrNumCharater***********************/
/* Returns Valid,Invalid or Seperator								*/
/****************************************************************/

USINT  fncValidErrNumCharacter(USINT usiChar)
{
	if((usiChar >= conAsciiZero)&&(usiChar <= conAsciiNine))
		return(conValid);
	if(usiChar == conAsciiSeperator)
		return(conGotSeperator);
	return(conInvalid);
}
/*************** Function fncErrTypeCharater***********************/
/* Returns Valid,Invalid or Seperator								*/
/****************************************************************/

USINT  fncValidErrTypeCharacter(USINT usiChar)
{
	if((usiChar >= conAsciia)&&(usiChar <= conAsciiz))
		return(conValid);
	if((usiChar >= conAsciiA)&&(usiChar <= conAsciiZ))
		return(conValid);
	if( (usiChar == conAsciiSeperator))
		return(conGotSeperator);
	return(conInvalid);
}
USINT  fncValidMachTypeCharacter(USINT usiChar)
{
	if((usiChar >= conAsciia)&&(usiChar <= conAsciiz))
		return(conValid);
	if((usiChar >= conAsciiA)&&(usiChar <= conAsciiZ))
		return(conValid);
	if((usiChar >= conAsciiZero)&&(usiChar <= conAsciiNine))
		return(conValid);
	if( (usiChar == conAsciiSpace))
		return(conValid);
	if( (usiChar == conAsciiSeperator))
		return(conGotSeperator);
	return(conInvalid);
}

void fncShowErrorHelp(void)
{	
	if(fncStrCmpI(stErrorDisplay.strErrorDisplay[uiSelectedDisErrIndex], "SYSTEM OK") == 0)
		return;
	
	uiErrInfoIndex	= stErrorDisplay.uiErrInfoIndex[uiSelectedDisErrIndex];
		
	strcpy(strMachType, ErrInfo[uiErrInfoIndex].strMachine);
	fncString2Upper(strMachType);
	strcpy(strErrNum, ErrInfo[uiErrInfoIndex].strErrNum);
	fncGetLanguageFolderName(uiCurLanguage, strLanguageCode);
	strcpy(strCurHelpHtmlFileLocation, "file://HELP:/");
	strcat(strCurHelpHtmlFileLocation, strLanguageCode);
	strcat(strCurHelpHtmlFileLocation, "/ERRORS/");
	
	if(ErrInfo[uiErrInfoIndex].usiType == 0)
	{
		strcat(strCurHelpHtmlFileLocation, "MACHINES/");
		strcat(strCurHelpHtmlFileLocation, strMachType);
		strcat(strCurHelpHtmlFileLocation, "/");
		strcat(strCurHelpHtmlFileLocation, strErrNum);	
	}
	else if(ErrInfo[uiErrInfoIndex].usiType == conBrErr)
		strcat(strCurHelpHtmlFileLocation, "BR/BRERRORS");
	else if(ErrInfo[uiErrInfoIndex].usiType == conSysErr)
		strcat(strCurHelpHtmlFileLocation, "SYS/SYSERRORS");
	else if(ErrInfo[uiErrInfoIndex].usiType == conHardwareErr)
		strcat(strCurHelpHtmlFileLocation, "HDWR/HDWRERRORS");
	else if(ErrInfo[uiErrInfoIndex].usiType == conServoErr)
		strcat(strCurHelpHtmlFileLocation, "SRV/SRVERRORS");
	else if(ErrInfo[uiErrInfoIndex].usiType == conMartinErr)
		strcat(strCurHelpHtmlFileLocation, "MARTIN/MARTINERRORS");
	else
		strcpy(strCurHelpHtmlFileLocation, "NOTFOUND");

	strcat(strCurHelpHtmlFileLocation, ".html");
	uiChangeMenu	= menHelp;
}

void fncModulesStructInit(void)
{
	INT i, j;
	
	/*Initialize modules structure to all OK*/
	for(i=0;i<=conNumberOfModules;i++)
	{
		stModule[i].stStatus.blnModuleOK			= conTrue;
		stModule[i].stStatus.blnBusPowerOk			= conTrue;
		stModule[i].stStatus.blnEncPowerSupplyOk	= conTrue;
		stModule[i].stStatus.blnIOPowerOk			= conTrue;
		stModule[i].stStatus.blnStaleData			= conFalse;
		for(j=0;j<=conMaxDigOutPerModule;j++)
			stModule[i].stDoup[j].blnStatus			= conFalse;
		for(j=0;j<=conMaxAnlgInPerModule;j++)
			stModule[i].stAinp[j].usiStatus			= 0;
		for(j=0;j<=conMaxEncodersPerModule;j++)
			stModule[i].stEnc[j].usiStatus			= 0;
	}
	
}

void fncErrCheckModules(void)
{
	STRING	strErrPre[19], strErrMsg[80], strModNum[10], strIONum[2];
	INT		i, j;
	
	/*NOTE
		strHdwrErrorStrings index to string. These values are taken from the txtMartinSysErrCommon text group. 
		Use the conNumHdwrErrStrings constant to set the upper limit of this array. This value should match the highest text group index number,
		and there should be no gaps in the index numbers.
		0 - Module
		1 - Digital Out
		2 - Analog In
		3 - Not OK
		4 - Analog In Error 1 - Below lower limit
		5 - Analog In Error 10 - Above upper limit
		6 - Analog In Error 11 - Wire break
		7 - Drive
		8 - Drive fault
		9 - Go to status screen for help
		10 - System error
		11 - Bus power low
		12 - IO power low
		13 - Encoder power low
	*/
	
	for(i=0;i<=conNumberOfModules;i++)
	{		
		
		itoa(i, strModNum);
		strcpy(strErrPre, strHdwrErrorStrings[0]);
		strcat(strErrPre, " #");
		strcat(strErrPre, strModNum);
		strcat(strErrPre, " ");
		
		if(!stModule[i].stStatus.blnModuleOK)	
		{
			/*Set module not ok error*/
			strcpy(strErrMsg, strErrPre);
			strcat(strErrMsg, strHdwrErrorStrings[3]);
			fncSetSysErr(conTrue,2, conHardwareErr, conFalse, conFalse, conFalse, strErrMsg);
		}
		else 
		{
			if(!stModule[i].stStatus.blnBusPowerOk)
			{
				/*Set module bus power not ok error*/
				strcpy(strErrMsg, strErrPre);
				strcat(strErrMsg, strHdwrErrorStrings[11]);
				fncSetSysErr(conTrue,3, conHardwareErr, conFalse, conFalse, conFalse, strErrMsg);	
			}
			if(!stModule[i].stStatus.blnIOPowerOk)
			{
				/*Set module IO power not ok error*/
				strcpy(strErrMsg, strErrPre);
				strcat(strErrMsg, strHdwrErrorStrings[12]);
				fncSetSysErr(conTrue,4, conHardwareErr, conFalse, conFalse, conFalse, strErrMsg);
			}
			if(!stModule[i].stStatus.blnEncPowerSupplyOk)
			{
				/*Set module encoder power not ok error*/
				strcpy(strErrMsg, strErrPre);
				strcat(strErrMsg, strHdwrErrorStrings[13]);
				fncSetSysErr(conTrue,5, conHardwareErr, conFalse, conFalse, conFalse, strErrMsg);
			}
		
			for(j=0;j<=conMaxDigOutPerModule;j++)
			{
				if(stModule[i].stDoup[j].blnStatus)
				{
					/*Set digital out not ok error*/
					itoa(j, strIONum);
					strcpy(strErrMsg, strErrPre);
					strcat(strErrMsg, strHdwrErrorStrings[1]);
					strcat(strErrMsg, " #");
					strcat(strErrMsg, strIONum);
					strcat(strErrMsg, " ");
					strcat(strErrMsg, strHdwrErrorStrings[3]);
					fncSetSysErr(conTrue,6, conHardwareErr, conFalse, conFalse, conFalse, strErrMsg);
				}
			}

			for(j=0;j<=conMaxAnlgInPerModule;j++)
			{
				if(stModule[i].stAinp[j].usiStatus != 0)
				{
					/*Set analog in error*/	
					itoa(j, strIONum);
					strcpy(strErrMsg, strErrPre);
					strcat(strErrMsg, strHdwrErrorStrings[2]);
					strcat(strErrMsg, " #");
					strcat(strErrMsg, strIONum);
					strcat(strErrMsg, " ");
			
					if(stModule[i].stAinp[j].usiStatus == 1)
						strcat(strErrMsg, strHdwrErrorStrings[4]);
					else if(stModule[i].stAinp[j].usiStatus == 10)
						strcat(strErrMsg,  strHdwrErrorStrings[5]);
					else if(stModule[i].stAinp[j].usiStatus == 11)
						strcat(strErrMsg,  strHdwrErrorStrings[6]);
					else
						strcat(strErrMsg,  strHdwrErrorStrings[3]);
				
					fncSetSysErr(conTrue,stModule[i].stAinp[j].usiStatus, conHardwareErr, conFalse, conFalse, conFalse, strErrMsg);
				}
			}
		}
	}
}

void fncWriteErr2History(STRING *strError)
{
	UINT uiLen	= strlen(strError);
	
	memset(strErrHist[uiCurErrHistIndex],0,conErrHistRecordLength+1);/*Clear the string*/
	if(uiLen > (conErrHistRecordLength-3))
		fncSubString(strError, 0, (conErrHistRecordLength-3));/*Shorten to the length of the record*/
	strcpy(strErrHist[uiCurErrHistIndex], strError);/*Copy to the history*/
	strErrHist[uiCurErrHistIndex][conErrHistRecordLength-2]	= conAsciiCR;
	strErrHist[uiCurErrHistIndex][conErrHistRecordLength-1]	= conAsciiLF;
	strErrHist[uiCurErrHistIndex][conErrHistRecordLength]	= conAsciiNull;
	
	if(uiCurErrHistIndex < conMaxNumErrHistRecords)
		uiCurErrHistIndex++;
	else
	{
		/*Write current histor data to file*/
		blnWriteHist2FileRq	= conTrue;
		blnErrHistFull		= conTrue;
		uiCurErrHistIndex	= 0;
	}
	
	/*Increase the display index*/
	if(uiCurErrHistDisplayIndex != ((INT)(uiCurErrHistIndex) - 1))
	{
		uiCurErrHistDisplayIndex++;
		if(uiCurErrHistDisplayIndex > conMaxNumErrHistRecords)
			uiCurErrHistDisplayIndex	= 0;
	}
}
	
void fncClearErrHistory(void)
{
	INT i;
	for(i=0;i<=conMaxNumErrHistRecords;i++)
		strcpy(strErrHist[i], "");
	uiCurErrHistIndex			= 0;
	uiCurErrHistDisplayIndex	= 0;
	blnErrHistFull				= conFalse;
}

void fncRefreshErrHistDisplay(void)
{
	INT i;
	
	blnJumped	= conFalse;
	if(blnHistNextPage || blnHistBottom)
	{			
		if(blnErrHistFull)
		{
			if(blnHistBottom)
				uiCurErrHistDisplayIndex	= uiCurErrHistIndex;
				
			/*We are trying to view older errors and the history array is full*/
			/*Check if we are jumping the max/0 error index gap*/
			if(uiCurErrHistDisplayIndex < conNumDisplayHistErrors)
			{
				uiNewDisplayIndex	= conMaxNumErrHistRecords - (conNumDisplayHistErrors - uiCurErrHistDisplayIndex) + 1;
				/*Check if we are jumping the old/new gap*/
				if((uiNewDisplayIndex <= uiCurErrHistIndex) || (uiCurErrHistDisplayIndex >= uiCurErrHistIndex) || ((uiCurErrHistIndex + conNumDisplayHistErrors - 1) > uiNewDisplayIndex))
					blnJumped	= conTrue;
			}
			else
			{
				uiNewDisplayIndex	= uiCurErrHistDisplayIndex - conNumDisplayHistErrors;
				/*Check if we jumping the old/new gap*/
				if((uiNewDisplayIndex < uiCurErrHistIndex) && (uiCurErrHistDisplayIndex >= uiCurErrHistIndex))
					blnJumped	= conTrue;
			}
			
			/*We jumped over the old/new gap*/
			if(blnJumped)
			{
				uiNewDisplayIndex	= uiCurErrHistIndex + conNumDisplayHistErrors - 1;
				if(uiNewDisplayIndex > conMaxNumErrHistRecords)
					uiNewDisplayIndex	= uiNewDisplayIndex - conMaxNumErrHistRecords - 1;
			}
		}
		else
		{		
			/*The error array is not full and we are trying to see older errors*/
			if(uiCurErrHistIndex > conNumDisplayHistErrors)
			{
				if(blnHistBottom)
					uiNewDisplayIndex	= conNumDisplayHistErrors - 1;	
				else if(uiCurErrHistDisplayIndex < conNumDisplayHistErrors)
					uiNewDisplayIndex	= conNumDisplayHistErrors - 1;
				else if((uiCurErrHistDisplayIndex - conNumDisplayHistErrors) < conNumDisplayHistErrors)
					uiNewDisplayIndex	= conNumDisplayHistErrors - 1;
				else
					uiNewDisplayIndex	= uiCurErrHistDisplayIndex - conNumDisplayHistErrors;
			}
			else
			{
				if(uiCurErrHistIndex != 0)
					uiNewDisplayIndex	= uiCurErrHistIndex - 1;	
				else
					uiNewDisplayIndex	= 0;
			}
		}
		uiCurErrHistDisplayIndex	= uiNewDisplayIndex;
		blnHistNextPage				= conFalse;
		blnHistBottom				= conFalse;
	}
	else if(blnHistPrevPage)
	{
		if(blnErrHistFull)
		{

			/*We are trying to view newer errors and the history array is full*/
			/*Check if we are jumping the max/0 error index gap*/
			if((uiCurErrHistDisplayIndex + conNumDisplayHistErrors) > conMaxNumErrHistRecords)
			{
				/*We are jumping the max/0 index gap*/
				uiNewDisplayIndex	= uiCurErrHistDisplayIndex + conNumDisplayHistErrors - conMaxNumErrHistRecords - 1;
				/*Check if we are jumping the old/new gap*/
				if((uiNewDisplayIndex >= uiCurErrHistIndex) || (uiCurErrHistDisplayIndex < uiCurErrHistIndex))
					blnJumped	= conTrue;
			}
			else
			{
				uiNewDisplayIndex	= uiCurErrHistDisplayIndex + conNumDisplayHistErrors;
				/*Check if we jumping the old/new gap*/
				if((uiNewDisplayIndex >= uiCurErrHistIndex) && (uiCurErrHistDisplayIndex < uiCurErrHistIndex))
					blnJumped	= conTrue;
			}			
			
			/*We jumped over the old/new gap*/
			if(blnJumped)
			{
				if(uiCurErrHistIndex == 0)
					uiNewDisplayIndex	= conMaxNumErrHistRecords;
				else
					uiNewDisplayIndex	= uiCurErrHistIndex - 1;
			}
		}
		else
		{
			if(uiCurErrHistDisplayIndex + conNumDisplayHistErrors >= uiCurErrHistIndex)
			{
				if(uiCurErrHistIndex != 0)
					uiNewDisplayIndex	= uiCurErrHistIndex - 1;
				else
					uiNewDisplayIndex	= 0;	
			}
			else
				uiNewDisplayIndex	= uiCurErrHistDisplayIndex + conNumDisplayHistErrors;					
		}
		uiCurErrHistDisplayIndex	= uiNewDisplayIndex;
		blnHistPrevPage	= conFalse;
	}
	else if(blnHistTop)
	{
		if(blnErrHistFull && (uiCurErrHistIndex == 0))
			uiNewDisplayIndex	= conMaxNumErrHistRecords;
		else if(uiCurErrHistIndex != 0)
			uiNewDisplayIndex	= uiCurErrHistIndex - 1;
		else
			uiNewDisplayIndex	= 0;
		uiCurErrHistDisplayIndex	= uiNewDisplayIndex;
		blnHistTop	= conFalse;
	}

	/*Reset display error string array*/
	for(i=0;i<conNumDisplayHistErrors;i++)
	{		
		if(uiCurErrHistDisplayIndex < i)
			uiErrHistIndex		= conMaxNumErrHistRecords - (i - uiCurErrHistDisplayIndex) + 1;
		else
			uiErrHistIndex		= uiCurErrHistDisplayIndex - i;	
			
		if((fncStrCmpI(strErrHist[uiErrHistIndex], "") == 0) || (!blnErrHistFull && (uiCurErrHistDisplayIndex < i)))
			strcpy(strErrHistDisplay[i], "");
		else
			strcpy(strErrHistDisplay[i], strErrHist[uiErrHistIndex]);
	}
}

void fncDrivesStructInit(void)
{
	INT i;
	
	/*Initialize drives status to OK*/
	for(i=0;i<=conMaxNumOfDrives;i++)
		stDrive[i].blnModuleOk	= conTrue;
		
}

void fncErrCheckDrives(void)
{
	STRING	strErrMsg[80], strDrvNum[10];
	INT		i;
	
	for(i=0;i<=conMaxNumOfDrives;i++)
	{		
		if(!stDrive[i].blnModuleOk)	
		{
			/*Set module not ok error*/
			itoa(i, strDrvNum);
			strcpy(strErrMsg, strHdwrErrorStrings[7]);
			strcat(strErrMsg, " #");
			strcat(strErrMsg, strDrvNum);
			strcat(strErrMsg, " ");
			strcat(strErrMsg, strHdwrErrorStrings[3]);
			fncSetSysErr(conTrue,0, conInvDriveErr, conFalse, conFalse, conFalse, strErrMsg);
		}
	}
}

#define STATE_WAIT				0
#define STATE_CHECK_EXISTANCE	1
#define STATE_DELETE_FILE		2
#define STATE_CREATE_FILE		3
#define STATE_WRITE_2_FILE		4
#define STATE_CLOSE_FILE		5

void fncWriteHist2File(void)
{
	
	blnErrHistFileDone					= conFalse;
	
	switch(usiErrHistState)
	{
		case STATE_WAIT:
			if(blnWriteHist2FileRq)
			{
				blnWriteHist2FileRq		= conFalse;
				itoa(usiCurErrHistFileNum, strFileNum);
				strcpy(strErrHistFileName, "ErrLog_");
				strcat(strErrHistFileName, strFileNum);
				strcat(strErrHistFileName, ".txt");	
				FHistInfo.enable		= conTrue;
				FHistInfo.pDevice		= conErrHistDevice;
				FHistInfo.pInfo			= &stErrHistFileInfo;
				FHistInfo.pName			= (UDINT)strErrHistFileName;
				blnWriteErrHistBusy		= conTrue;
				usiErrHistState			= STATE_CHECK_EXISTANCE;
			}
		break;
		case STATE_CHECK_EXISTANCE:
			if(FHistInfo.status == 20708)/*Status: File not found */
			{
				/*File not found*/	
				FHistInfo.enable		= conFalse;	
				FHistCreate.enable		= conTrue;
				FHistCreate.pDevice		= (UDINT)conErrHistDevice;
				FHistCreate.pFile		= (UDINT)strErrHistFileName;
				usiErrHistState			= STATE_CREATE_FILE;			
			}
			else if(FHistInfo.status != 65535)/*Status: Not busy*/
			{
				/*Found the file*/
				FHistInfo.enable		= conFalse;			
				FHistDelete.enable		= conTrue;
				FHistDelete.pDevice		= (UDINT)conErrHistDevice;
				FHistDelete.pName		= (UDINT)strErrHistFileName;
				usiErrHistState			= STATE_DELETE_FILE;	
			}
		break;
		case STATE_DELETE_FILE:
			if(FHistDelete.status == 0)
			{
				/*Successfully deleted the file, create a new file*/
				FHistDelete.enable		= conFalse;
				FHistCreate.enable		= conTrue;
				FHistCreate.pDevice		= (UDINT)conErrHistDevice;
				FHistCreate.pFile		= (UDINT)strErrHistFileName;
				usiErrHistState			= STATE_CREATE_FILE;
			}
			else if(FHistDelete.status != 65535)
			{
				/*Error*/	
				FHistDelete.enable		= conFalse;
				blnErrHistFileDone		= conTrue;
				blnWriteErrHistBusy		= conFalse;
				usiErrHistState			= STATE_WAIT;
			}
		break;
		case STATE_CREATE_FILE:
			if(FHistCreate.status == 0)
			{
				/*Successfully created the file, open it up*/
				FHistCreate.enable		= conFalse;
				FHistWrite.enable		= conTrue;
				FHistWrite.ident		= FHistCreate.ident;
				FHistWrite.len			= sizeof(strErrHist);
				FHistWrite.offset		= 0;
				FHistWrite.pSrc			= (UDINT)strErrHist;
				usiErrHistState			= STATE_WRITE_2_FILE;
			}
			else if(FHistCreate.status != 65535)
			{
				/*Error creating the file*/	
				FHistCreate.enable		= conFalse;
				blnErrHistFileDone		= conTrue;
				blnWriteErrHistBusy		= conFalse;
				usiErrHistState			= STATE_WAIT;
			}
		break;
		case STATE_WRITE_2_FILE:
			if(FHistWrite.status != 65535)
			{
				/*Done*/
				FHistWrite.enable		= conFalse;
				FHistClose.enable		= conTrue;
				FHistClose.ident		= FHistWrite.ident;
				usiErrHistState			= STATE_CLOSE_FILE;
			}
		break;
		case STATE_CLOSE_FILE:
			if(FHistClose.status != 65535)
			{
				/*Increase the current err history file number since we successfully made a new file*/
				usiCurErrHistFileNum++;
				if(usiCurErrHistFileNum > conMaxNumErrHistFiles)
					usiCurErrHistFileNum	= 0;
				FHistClose.enable			= conFalse;
				blnErrHistFileDone			= conTrue;
				blnWriteErrHistBusy			= conFalse;
				usiErrHistState				= STATE_WAIT;
			}
		break;
	}
	
	if(blnErrHistFileDone || blnWriteErrHistBusy)
	{
		FileInfo(&FHistInfo);
		FileDelete(&FHistDelete);
		FileCreate(&FHistCreate);
		FileWrite(&FHistWrite);
		FileClose(&FHistClose);
	}
	
}
