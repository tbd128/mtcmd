//	Rewind.h
//	Part of Ophiuchus
//	
//	Copyright (c) 2002-2023 tbd128 Allright Reserved.
//	This project is licensed under the MIT License, see the LICENSE.txt file for details.
//	
//	Monday, March 11, 2002
//

#ifdef	Rewind_h__
#else
#define	Rewind_h__

IOReturn	opcs_rewind( ScsiDevInfoRec *tgtDevInfoP, 
			     SCSITaskStatus *taskStatus );

#endif	//	Rewind_h__
