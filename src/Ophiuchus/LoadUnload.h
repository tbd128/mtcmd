//	LoadUnload.h
//	Part of Ophiuchus
//	
//	Copyright (c) 2002-2023 tbd128 Allright Reserved.
//	This project is licensed under the MIT License, see the LICENSE.txt file for details.
//	
//	Monday, March 11, 2002
//

#ifdef	LoadUnload_h__
#else
#define	LoadUnload_h__

#define	kLoadTape	true
#define	kUnloadTape	false

IOReturn	opcs_loadUnload( ScsiDevInfoRec *tgtDevInfoP, Boolean load, 
				 SCSITaskStatus *taskStatus );

#endif
