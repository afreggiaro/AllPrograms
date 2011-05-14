/********************************************************************
 * Program: prgGenericMach
 * File: prgGenericMach.c
 * Author: TonyF
 * Created: September 30, 2010
 ********************************************************************
 * Implementation of program prgGenericMach
 ********************************************************************/

#include <bur/plctypes.h>

#ifdef _DEFAULT_INCLUDES
 #include <AsDefault.h>
#endif
#include "header.h"
#include "Bcs.h"

void _INIT prgGenericMachINIT( void )
{
	usiGenericMachPrgIndex		= usiPrgNum;
	blnTooManyPrograms 			= fncGetProgramIndex();
}

void _CYCLIC prgGenericMachCYCLIC( void )
{
	
	if(!blnPlcSystemOk)
		return;

	usiGenericMachIndex	= stGenericMachMap.In.usiGenericMachIndex;
	itoa(usiGenericMachIndex, strGenericMachIndex);
		
	usiThisMachIndex	= Gp.Bcs.GenericMach[usiGenericMachIndex].usiMachIndex;
	usiEntryMachIndex	= Gp.Bcs.GenericMach[usiGenericMachIndex].usiEntryMachIndex;
	usiExitMachIndex	= Gp.Bcs.GenericMach[usiGenericMachIndex].usiExitMachIndex;
	usiThisLineNumber	= Gp.Bcs.GenericMach[usiGenericMachIndex].usiLineNumber;
	
	for(i=1;i<=conMaxNumDrivesPerMach;i++)
		stMachInterface[usiThisMachIndex].Out.blnDriveFault[i]	= conFalse;
	stMachInterface[usiThisMachIndex].Out.blnReady									= IO.Bcs.GenericMach[usiGenericMachIndex].Dinp.Ready.blnVal;
	stMachInterface[usiThisMachIndex].Out.blnZoneReady[0]							= stMachInterface[usiThisMachIndex].Out.blnReady;
	stMachInterface[usiThisMachIndex].Out.usiNumOfZones								= 1;
	stMachInterface[usiThisMachIndex].Out.usiMachType								= conMachTypeGeneric;
	stMachInterface[usiThisMachIndex].Out.usiLineNumber								= usiThisLineNumber;
	stMachInterface[usiThisMachIndex].Out.usiNumBundlesAcross[usiThisLineNumber]	= stMachInterface[usiEntryMachIndex].Out.usiNumBundlesAcross[usiThisLineNumber];
	strcpy(stMachInterface[usiThisMachIndex].Out.strMachTypeAndSN, Gp.Bcs.GenericMach[usiGenericMachIndex].strMachName);
	
	if(IO.Bcs.Dinp.BlcEye[Gp.Bcs.GenericMach[usiGenericMachIndex].usiWakeUpEyeIndex].blnVal)
		IO.Bcs.GenericMach[usiGenericMachIndex].Doup.WakeUp.blnOn					= conTrue;
	else
		IO.Bcs.GenericMach[usiGenericMachIndex].Doup.WakeUp.blnOff					= conTrue;
	
	if(blnAllLinesOn || !stMachInterface[usiEntryMachIndex].Out.blnLineOff[usiThisLineNumber])
	{
		stMachInterface[usiThisMachIndex].Out.blnLineOff[usiThisLineNumber]			= conFalse;
		if(!IO.Bcs.GenericMach[usiGenericMachIndex].Doup.LineOn.blnVal)
			IO.Bcs.GenericMach[usiGenericMachIndex].Doup.LineOn.blnOn				= conTrue;
		strcpy(stStatus.strCurrentState, "LINE ON");
		stStatus.blnActive	= conTrue;
	}
	else
	{
		stMachInterface[usiThisMachIndex].Out.blnLineOff[usiThisLineNumber]			= conTrue;
		if(IO.Bcs.GenericMach[usiGenericMachIndex].Doup.LineOn.blnVal)
			IO.Bcs.GenericMach[usiGenericMachIndex].Doup.LineOn.blnOff				= conTrue;
		strcpy(stStatus.strCurrentState, "LINE OFF");
		stStatus.blnActive	= conFalse;
	}
			
	if(Gp.Bcs.GenericMach[usiGenericMachIndex].blnBundleRotatedAtExit + stMachInterface[usiEntryMachIndex].Out.blnRotateBundle[usiThisLineNumber] == 1)
		stMachInterface[usiThisMachIndex].Out.blnRotateBundle[usiThisLineNumber]	= conTrue;
	else
		stMachInterface[usiThisMachIndex].Out.blnRotateBundle[usiThisLineNumber]	= conFalse;
	
	if(stMachInterface[usiExitMachIndex].Out.blnReady && !IO.Bcs.GenericMach[usiGenericMachIndex].Doup.DsReady.blnVal)
		IO.Bcs.GenericMach[usiGenericMachIndex].Doup.DsReady.blnOn					= conTrue;
	else if(!stMachInterface[usiExitMachIndex].Out.blnReady && IO.Bcs.GenericMach[usiGenericMachIndex].Doup.DsReady.blnVal)
		IO.Bcs.GenericMach[usiGenericMachIndex].Doup.DsReady.blnOff					= conTrue;
		
	if(blnLineHold[usiThisLineNumber] && !IO.Bcs.GenericMach[usiGenericMachIndex].Doup.HoldEn.blnVal)
		IO.Bcs.GenericMach[usiGenericMachIndex].Doup.HoldEn.blnOn					= conTrue;
	else if(!blnLineHold[usiThisLineNumber] && IO.Bcs.GenericMach[usiGenericMachIndex].Doup.HoldEn.blnVal)
		IO.Bcs.GenericMach[usiGenericMachIndex].Doup.HoldEn.blnOff					= conTrue;
	
	/*Update status structure*/
	strcpy(stStatus.strProgramName, "prgGenericMach_");
	strcat(stStatus.strProgramName, strGenericMachIndex);
	stStatus.tmrActive.IN		= stStatus.blnActive;
	stStatus.tmrActive.PT		= conMaxTime;
	TON(&stStatus.tmrActive);
	stStatus.tmRunTime			= stStatus.tmrActive.ET;
	stStatus.osfActive.CLK		= stStatus.blnActive;
	F_TRIG(&stStatus.osfActive);	
	
	memcpy(&stPrgStatus[usiGenericMachPrgIndex], &stStatus, sizeof(prgStatus_typ));
	
}
