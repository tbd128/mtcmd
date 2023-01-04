//	CheckSenseStat.c
//	Part of Ophiuchus
//	
//	Copyright (c) 2002-2023 tbd128 Allright Reserved.
//	This project is licensed under the MIT License, see the LICENSE.txt file for details.
//	
//	2.0   : Monday, March 11, 2002
//	2.0.1 : Sunday, March 24, 2002
//

#include	<stdio.h>
#include	<unistd.h>

#include	<IOKit/IOKitLib.h>
#include	<IOKit/scsi/SCSITaskLib.h>
#include	<IOKit/scsi/SCSICommandOperationCodes.h>

#include	"ScsiDevInfo.h"
#include	"CheckSenseStat.h"


int	opcs_analyzeSenseData( const SCSI_Sense_Data *sense, 
			       opcs_SenseDataRec *senseData )
{
  int	result = 0;
  
  memset( senseData, 0, sizeof(opcs_SenseDataRec) );
  
  senseData->Valid = ((sense->VALID_RESPONSE_CODE) >> 7) & 0x1;
  senseData->ErrorClass = ((sense->VALID_RESPONSE_CODE) >> 4) & 0x7;
  if( senseData->ErrorClass != 7 )
    {
      senseData->ErrorCode = (sense->VALID_RESPONSE_CODE) & 0xf;
    }
  else
    {
      // class 7
      senseData->ErrorCode = (sense->VALID_RESPONSE_CODE) & 0xf;
      senseData->SegNo = (sense->SEGMENT_NUMBER);
      senseData->FileMark = ((sense->SENSE_KEY) >> 7) & 0x1;
      senseData->EOM = ((sense->SENSE_KEY) >> 6) & 0x1;
      senseData->ILI = ((sense->SENSE_KEY) >> 5) & 0x1;
      senseData->key = sense->SENSE_KEY & 0x0F;
      senseData->Info = ((sense->INFORMATION_1) << 24)
	| ((sense->INFORMATION_2) << 16)
	| ((sense->INFORMATION_3) << 8)
	| ((sense->INFORMATION_4) << 0);
      senseData->ASC = sense->ADDITIONAL_SENSE_CODE;
      senseData->ASCQ = sense->ADDITIONAL_SENSE_CODE_QUALIFIER;
    }
  
  if( /*(0 <= senseData.ErrorClass) &&*/ (senseData->ErrorClass <= 6) )
    {
      // Something happened. Print the sense string
      //err = senseData.ErrorCode;
      //PrintSenseString( &(param.senseData) );
      if( senseData->ErrorClass != 0 )
	{
	  fprintf( stderr, "mtcmd : RequestSense - ErrorCode : %d\n", 
		   senseData->ErrorCode );
	}
    }
  else
    {
      if( senseData->key != senseKey_NoSense )
	{
	  result |= (1 << (senseData->key - 1));
	}
    }
  return result;
}


void	opcs_printSenseKeyMsg( const opcs_SenseDataRec *senseData )
{
  switch( senseData->key )
    {
    case senseKey_NoSense:		// 0x0
      break;
    case senseKey_RecoveredError:	// 0x1
      break;
    case senseKey_NotReady:		// 0x2
      fprintf( stderr, "mtcmd : Device not ready\n" );
      break;
    case senseKey_MediumError:		// 0x3
      fprintf( stderr, "mtcmd : Medium error detected.\n" );
      break;
    case senseKey_HardwareError:	// 0x4
      fprintf( stderr, "mtcmd : Hardware error detected.\n" );
      break;
    case senseKey_IllegalRequest:	// 0x5
      fprintf( stderr, "mtcmd : Illegal command request error detected.\n" );
      break;
    case senseKey_UnitAttention:	// 0x6
      fprintf( stderr, "mtcmd : Unit attention detected.\n" );
      break;
    case senseKey_DataProtect:		// 0x7
      fprintf( stderr, "mtcmd : Device(medium) read only.\n" );
      break;
    case senseKey_BlankCheck:		// 0x8
      fprintf( stderr, "mtcmd : Blank area or end of data detected.\n" );
      break;
    case senseKey_VendorUniq:		// 0x9
      fprintf( stderr, "mtcmd : An error (Vendor Uniq) detected. \n" );
      break;
    case senseKey_CopyAborted:		// 0xa
      fprintf( stderr, 
	       "mtcmd : Copy, Compare or CopyAndVerify error detected.\n" );
      break;
    case senseKey_AbortedCommand:	// 0xb
      fprintf( stderr, "mtcmd : Command aborted by target side.\n" );
      break;
    case senseKey_Equal:		// 0xc
      break;
    case senseKey_VolumeOverflow:	// 0xd
      fprintf( stderr, "mtcmd : end-of-partition detected.\n" );
      fprintf( stderr, "        some data was not written.\n" );
      break;
    case senseKey_Miscompare:		// 0xe
      fprintf( stderr, "mtcmd : Data difference detected.\n" );
      break;
    case senseKey_Reserved:		// 0xf
      break;
    default:
      break;
    }
  printf("        (Key=%02x, ASC=%02x, ASCQ=%02x)\n", senseData->key, senseData->ASC, senseData->ASCQ);
}


void	opcs_printSenseString( const SCSI_Sense_Data *sense )
{
  char		str[256];
  UInt8		Valid, ErrorClass, ErrorCode, SegNo;
  UInt8		FileMark, EOM, ILI;
  UInt32	Info;
  UInt8		key, ASC, ASCQ;
  
  fprintf( stderr, "----------- Sense Data Begin -----------\n" );
  Valid = ((sense->VALID_RESPONSE_CODE) >> 7) & 0x1;
  ErrorClass = ((sense->VALID_RESPONSE_CODE) >> 4) & 0x7;
  if( ErrorClass != 7 )
    {
      ErrorCode = (sense->VALID_RESPONSE_CODE) & 0xf;
      fprintf( stderr, "Valid = 0x%x\n", Valid );
      fprintf( stderr, "ErrorClass = 0x%x\n", ErrorClass );
      fprintf( stderr, "ErrorCode = 0x%x\n", ErrorCode );
    }
  else
    {
      // class 7
      ErrorCode = (sense->VALID_RESPONSE_CODE) & 0xf;
      SegNo = (sense->SEGMENT_NUMBER);
      FileMark = ((sense->SENSE_KEY) >> 7) & 0x1;
      EOM = ((sense->SENSE_KEY) >> 6) & 0x1;
      ILI = ((sense->SENSE_KEY) >> 5) & 0x1;
      key = sense->SENSE_KEY & 0x0F;
      Info = ((sense->INFORMATION_1) << 24)
	| ((sense->INFORMATION_2) << 16)
	| ((sense->INFORMATION_3) << 8)
	| ((sense->INFORMATION_4) << 0);
      ASC = sense->ADDITIONAL_SENSE_CODE;
      ASCQ = sense->ADDITIONAL_SENSE_CODE_QUALIFIER;
      
      fprintf( stderr, "Valid = 0x%x\n", Valid );
      fprintf( stderr, "ErrorClass = 0x%x\n", ErrorClass );
      fprintf( stderr, "ErrorCode = 0x%x (always 0 because ErrorClass == 7)\n", 
	      ErrorCode );
      fprintf( stderr, "SegNo = 0x%x\n", SegNo );
      fprintf( stderr, "FileMark = %s\n", FileMark ? "Yes" : "No" );
      fprintf( stderr, "EOM = %s\n", EOM ? "Yes" : "No" );
      fprintf( stderr, "ILI = %s\n", ILI ? "Yes" : "No" );
      sprintf( str, "Key: 0x%02lx, ASC: 0x%02lx, ASCQ: 0x%02lx ", 
	       (long int)key, (long int)ASC, (long int)ASCQ );
      fprintf( stderr, "Info = %d\n", Info );
      fprintf( stderr, "%s\n", str );
    }
  fprintf( stderr, "----------- Sense Data End -----------\n" );
}
