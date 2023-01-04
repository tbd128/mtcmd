//	TestUnitReady.h
//	Part of Ophiuchus
//	
//	Copyright (c) 2002-2023 tbd128 Allright Reserved.
//	This project is licensed under the MIT License, see the LICENSE.txt file for details.
//	
//	Monday, March 11, 2002
//

#ifdef	TestUnitReady_h__
#else
#define	TestUnitReady_h__

IOReturn	opcs_waitUntilDeviceIdle( ScsiDevInfoRec *tgtDevInfoP, 
					  SCSITaskStatus *taskStatus );
IOReturn	opcs_testUnitReady( ScsiDevInfoRec *tgtDevInfoP, 
				    SCSITaskStatus *taskStatus );

#endif	//	TestUnitReady_h__
