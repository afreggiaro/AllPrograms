/********************************************************************
 * COPYRIGHT --  
 ********************************************************************
 * Program: prgLfTable
 * File: prgLfTable.c
 * Author: TonyF
 * Created: August 04, 2010
 ********************************************************************
 * Implementation of program prgLfTable
 ********************************************************************/

#include <bur/plctypes.h>

#ifdef _DEFAULT_INCLUDES
 #include <AsDefault.h>
#endif
#include "header.h"
#include "bcs.h"

void _INIT prgLfTableINIT( void )
{
	stStatus.usiCurrentState	= 0;
	stStatus.usiLastState		= 0;
	usiLfTablePrgIndex			= usiPrgNum;
	blnTooManyPrograms 			= fncGetProgramIndex();
}

void _CYCLIC prgLfTableCYCLIC( void )
{
	if(!blnPlcSystemOk)
		return;
	
	/*Get the control station index*/
	usiLfTableIndex				= stLfTableMap.In.usiLfTableIndex;
	itoa(usiLfTableIndex, strLfTableIndex);
	
	usiThisMachIndex			= Gp.Bcs.LoadFormerTable[usiLfTableIndex].usiMachIndex;	
	
	/*Clear the state string*/
	strcpy(stStatus.strCurrentState, "");	
		
	/*Process machine interface outputs*/
	tmrReadyDelay.IN	= !IO.Bcs.LoadFormerTable[usiLfTableIndex].Dinp.BusyEye.blnVal;/*Ready when the busy eye is not blocked*/
	tmrReadyDelay.PT	= Gp.Bcs.LoadFormerTable[usiLfTableIndex].tmReadyDelay;
	TON(&tmrReadyDelay);
	stMachInterface[usiThisMachIndex].Out.blnReady			= tmrReadyDelay.Q;
	stMachInterface[usiThisMachIndex].Out.blnZoneReady[0]	= stMachInterface[usiThisMachIndex].Out.blnReady;/*First zone is ready when machine is ready*/
	
	for(i=1;i<=conMaxNumDrivesPerMach;i++)
		stMachInterface[usiThisMachIndex].Out.blnDriveFault[i]	= conFalse;
		
	if(Gp.Bcs.LoadFormerTable[usiLfTableIndex].uiSerialNumber != 0)
	{
		stMachInterface[usiThisMachIndex].Out.uiSerialNumber	= Gp.Bcs.LoadFormerTable[usiLfTableIndex].uiSerialNumber;
		stMachInterface[usiThisMachIndex].Out.usiMachType		= conMachTypeAt1;
	}
	else
	{
		stMachInterface[usiThisMachIndex].Out.uiSerialNumber	= usiLfTableIndex;
		stMachInterface[usiThisMachIndex].Out.usiMachType		= conMachTypeAirTable;
	}
	stMachInterface[usiThisMachIndex].Out.usiMachTypeIndex	= usiLfTableIndex;
	stMachInterface[usiThisMachIndex].Out.usiNumOfZones		= 1;
	fncMachTypeAndSN(usiThisMachIndex, stMachInterface[usiThisMachIndex].Out.strMachTypeAndSN);
	
	if(stMachInterface[usiThisMachIndex].Out.blnReady)
	{
		stStatus.blnActive		= conFalse;
		strcat(stStatus.strCurrentState, "BUSY EYE OFF");
	}
	else
	{
		stStatus.blnActive		= conTrue;
		strcat(stStatus.strCurrentState, "BUSY EYE ON");
	}
		
	/*Update status structure*/
	strcpy(stStatus.strProgramName, "prgLfTable_");
	strcat(stStatus.strProgramName, strLfTableIndex);
	stStatus.tmrActive.IN		= stStatus.blnActive;
	stStatus.tmrActive.PT		= conMaxTime;
	TON(&stStatus.tmrActive);
	stStatus.tmRunTime			= stStatus.tmrActive.ET;
	stStatus.osfActive.CLK		= stStatus.blnActive;
	F_TRIG(&stStatus.osfActive);	
	
	memcpy(&stPrgStatus[usiLfTablePrgIndex], &stStatus, sizeof(prgStatus_typ));
	
}
