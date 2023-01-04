//	ModeSenseSelect.h
//	Part of Ophiuchus
//	
//	Copyright (c) 2002-2023 tbd128 Allright Reserved.
//	This project is licensed under the MIT License, see the LICENSE.txt file for details.
//	
//	2.0   : Monday, March 11, 2002
//	2.0.1 : Sunday, March 24, 2002
//	2.1   : Monday, May 07, 2002
//

#ifdef	ModeSenseSelect_h__
#else
#define	ModeSenseSelect_h__

typedef	enum
{
  NonPageFormat = 0,
  PageFormat
}ModeSelectPageType;

IOReturn	opcs_modeSense( ScsiDevInfoRec *tgtDevInfoP, 
				int pageCode, SCSITaskStatus *taskStatus );
void	opcs_printModeSense( const ScsiDevInfoRec *tgtDevInfoP );
IOReturn	opcs_modeSelect( ScsiDevInfoRec *tgtDevInfoP, 
				 ModeSelectPageType PageFormat, 
				 SCSITaskStatus *taskStatus );
IOReturn	opcs_setTransferBlockSize( ScsiDevInfoRec *tgtDevInfoP, 
					   int bSize, 
					   SCSITaskStatus *taskStatus );
IOReturn	opcs_setDensityCode( ScsiDevInfoRec *tgtDevInfoP, 
				     int densityType, 
				     SCSITaskStatus *taskStatus );

IOReturn	opcs_setCompressionMode( ScsiDevInfoRec *tgtDevInfoP, 
					 Boolean compressMode, 
					 SCSITaskStatus *taskStatus );

#endif
