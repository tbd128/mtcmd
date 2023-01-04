//	Write.h
//	Part of Ophiuchus
//	
//	Copyright (c) 2002-2023 tbd128 Allright Reserved.
//	This project is licensed under the MIT License, see the LICENSE.txt file for details.
//	
//	Monday, March 11, 2002
//

#ifdef	Write_h__
#else
#define	Write_h__

IOReturn	opcs_write( ScsiDevInfoRec *tgtDevInfoP, FILE *file,
			    SCSITaskStatus *taskStatus );
IOReturn	opcs_SCSIWrite( ScsiDevInfoRec *tgtDevInfoP, 
				UInt8 *buf, UInt64 *lenByte, 
				SCSITaskStatus *taskStatus );

#endif
