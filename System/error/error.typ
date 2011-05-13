(********************************************************************
 * COPYRIGHT --  
 ********************************************************************
 * Program: errors
 * File: errors.typ
 * Author: charles
 * Created: June 30, 2008
 ********************************************************************
 * Local data types of program errors
 ********************************************************************)

TYPE
	ErrorDisplay_typ : 	STRUCT 
		uiErrInfoIndex : ARRAY[0..conMaxDisplayErrors] OF UINT;
		strErrorDisplay : ARRAY[0..conMaxDisplayErrors] OF STRING[90];
	END_STRUCT;
END_TYPE
