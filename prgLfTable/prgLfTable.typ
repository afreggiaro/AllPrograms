(********************************************************************
 * COPYRIGHT --  
 ********************************************************************
 * Program: prgLfTable
 * File: prgLfTable.typ
 * Author: afreggiaro
 * Created: August 05, 2010
 ********************************************************************
 * Local data types of program prgLfTable
 ********************************************************************)

TYPE
	LfTableMapIn_typ : 	STRUCT 
		usiLfTableIndex : USINT;
	END_STRUCT;
	LfTableMap_typ : 	STRUCT 
		In : LfTableMapIn_typ;
	END_STRUCT;
END_TYPE
