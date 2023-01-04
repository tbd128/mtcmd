//	ReserveRelease.h
//	Part of Ophiuchus
//	
//	Copyright (c) 2002-2023 tbd128 Allright Reserved.
//	This project is licensed under the MIT License, see the LICENSE.txt file for details.
//	
//	Monday, March 11, 2002
//

#ifdef	ReserveRelease_h__
#else
#define	ReserveRelease_h__

IOReturn	opcs_reserveUnit( ScsiDevInfoRec *tgtDevInfoP, 
				  SCSITaskStatus *taskStatus );
IOReturn	opcs_releaseUnit( ScsiDevInfoRec *tgtDevInfoP, 
				  SCSITaskStatus *taskStatus );

#endif	//	ReserveRelease_h__
