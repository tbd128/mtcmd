//	CheckSenseStat.h
//	Part of Ophiuchus
//	
//	Copyright (c) 2002-2023 tbd128 Allright Reserved.
//	This project is licensed under the MIT License, see the LICENSE.txt file for details.
//	
//	Monday, March 11, 2002
//

#ifdef	CheckSenseStat_h__
#else
#define	CheckSenseStat_h__

enum
{
  senseKey_NoSense,		// 0x0
  senseKey_RecoveredError,	// 0x1
  senseKey_NotReady,		// 0x2
  senseKey_MediumError,		// 0x3
  senseKey_HardwareError,	// 0x4
  senseKey_IllegalRequest,	// 0x5
  senseKey_UnitAttention,	// 0x6
  senseKey_DataProtect,		// 0x7
  senseKey_BlankCheck,		// 0x8
  senseKey_VendorUniq,		// 0x9
  senseKey_CopyAborted,		// 0xa
  senseKey_AbortedCommand,	// 0xb
  senseKey_Equal,		// 0xc
  senseKey_VolumeOverflow,	// 0xd
  senseKey_Miscompare,		// 0xe
  senseKey_Reserved		// 0xf
};

enum
{
  senseErrBit_NoSense		= senseKey_NoSense,			// 0x0
  senseErrBit_RecoveredError	= (1<<(senseKey_RecoveredError - 1)),	// 0x1
  senseErrBit_NotReadyBF	= (1<<(senseKey_NotReady - 1)),		// 0x2
  senseErrBit_MediumError	= (1<<(senseKey_MediumError - 1)),	// 0x3
  senseErrBit_HardwareError	= (1<<(senseKey_HardwareError - 1)),	// 0x4
  senseErrBit_IllegalRequest	= (1<<(senseKey_IllegalRequest - 1)),	// 0x5
  senseErrBit_UnitAttention	= (1<<(senseKey_UnitAttention - 1)),	// 0x6
  senseErrBit_DataProtect	= (1<<(senseKey_DataProtect - 1)),	// 0x7
  senseErrBit_BlankCheck	= (1<<(senseKey_BlankCheck - 1)),	// 0x8
  senseErrBit_VendorUniq	= (1<<(senseKey_VendorUniq - 1)),	// 0x9
  senseErrBit_CopyAborted	= (1<<(senseKey_CopyAborted - 1)),	// 0xa
  senseErrBit_AbortedCommand	= (1<<(senseKey_AbortedCommand - 1)),	// 0xb
  senseErrBit_Equal		= (1<<(senseKey_Equal - 1)),		// 0xc
  senseErrBit_VolumeOverflow	= (1<<(senseKey_VolumeOverflow - 1)),	// 0xd
  senseErrBit_Miscompare	= (1<<(senseKey_Miscompare - 1)),	// 0xe
  senseErrBit_Reserve		= (1<<(senseKey_Reserved - 1)) 		// 0xf
};

typedef	struct	opcs_SenseDataRec	opcs_SenseDataRec;
struct	opcs_SenseDataRec
{
  UInt8		Valid;
  UInt8		ErrorClass;
  UInt8		ErrorCode;
  UInt8		SegNo;
  UInt8		FileMark;
  UInt8		EOM;
  UInt8		ILI;
  UInt32	Info;
  UInt8		key;
  UInt8		ASC;
  UInt8		ASCQ;
};

int	opcs_analyzeSenseData( const SCSI_Sense_Data *sense, 
			       opcs_SenseDataRec *senseData );
void	opcs_printSenseKeyMsg( const opcs_SenseDataRec *senseData );
void	opcs_printSenseString( const SCSI_Sense_Data *sense );

#endif	// CheckSenseStat_h__
