//	DensityCode.h
//	Part of Ophiuchus
//	
//	Copyright (c) 2002-2023 tbd128 Allright Reserved.
//	This project is licensed under the MIT License, see the LICENSE.txt file for details.
//	
//	2.1   : Monday, May 07, 2002
//

#ifdef	DensityCode_h__
#else
#define	DensityCode_h__

Boolean	opcs_getDensityCode( ScsiDevInfoRec *tgtDevInfoP, 
			     int densityType, UInt8 *densityCode );

#endif
