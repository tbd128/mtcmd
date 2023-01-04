//	Space.h
//	Part of Ophiuchus
//	
//	Copyright (c) 2002-2023 tbd128 Allright Reserved.
//	This project is licensed under the MIT License, see the LICENSE.txt file for details.
//	
//	2.0   : Monday, March 11, 2002
//	2.1   : Monday, May 07, 2002
//

#ifdef	Space_h__
#else
#define	Space_h__

IOReturn	opcs_bkwdSpace( ScsiDevInfoRec *tgtDevInfoP, 
				SInt32 fileMarkCount, 
				SCSITaskStatus *taskStatus );
IOReturn	opcs_fwdSpace( ScsiDevInfoRec *tgtDevInfoP, 
			       SInt32 fileMarkCount, 
			       SCSITaskStatus *taskStatus );

#endif	// Space_h__
