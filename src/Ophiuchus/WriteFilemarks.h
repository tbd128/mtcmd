//	WriteFilemarks.h
//	Part of Ophiuchus
//	
//	Copyright (c) 2002-2023 tbd128 Allright Reserved.
//	This project is licensed under the MIT License, see the LICENSE.txt file for details.
//	
//	Monday, March 11, 2002
//

#ifdef	WriteFilemarks_h__
#else
#define	WriteFilemarks_h__

IOReturn	opcs_writeFilemarks( ScsiDevInfoRec *tgtDevInfoP, 
				     UInt32 fileMarkCount,
				     SCSITaskStatus *taskStatus );

#endif	// WriteFilemarks_h__
