//	ReadBlockLimits.h
//	Part of Ophiuchus
//	
//	Copyright (c) 2002-2023 tbd128 Allright Reserved.
//	This project is licensed under the MIT License, see the LICENSE.txt file for details.
//	
//	Monday, March 11, 2002
//

#ifdef	ReadBlockLimits_h__
#else
#define	ReadBlockLimits_h__

IOReturn	opcs_readBlockLimits( ScsiDevInfoRec *tgtDevInfoP, 
				      SCSITaskStatus *taskStatus );
void	opcs_printBlockLimits( BlockLimitsRec *BlockLimitsP );

#endif	//	ReadBlockLimits_h__
