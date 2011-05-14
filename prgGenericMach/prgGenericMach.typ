(********************************************************************
 * COPYRIGHT --  
 ********************************************************************
 * Program: prgGenericMach
 * File: prgGenericMach.typ
 * Author: afreggiaro
 * Created: October 21, 2010
 ********************************************************************
 * Local data types of program prgGenericMach
 ********************************************************************)

TYPE
	GenericMachMapInput_typ : 	STRUCT 
		usiGenericMachIndex : USINT;
	END_STRUCT;
	GenericMachMap_typ : 	STRUCT 
		In : GenericMachMapInput_typ;
	END_STRUCT;
END_TYPE
