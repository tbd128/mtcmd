//	Read.c
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
#include	"ScsiCmdExec.h"
#include	"CheckSenseStat.h"
#include	"TestUnitReady.h"
#include	"Read.h"
#include	"DebugLog.h"

IOReturn	opcs_read( ScsiDevInfoRec *tgtDevInfoP, FILE *file,
			   SCSITaskStatus *taskStatus )
{
  UInt8			*buf;
  UInt64		readLen;
  UInt64		readQtm;
  IOReturn		err = kIOReturnSuccess;
  
  readQtm = tgtDevInfoP->opts.bSize * tgtDevInfoP->opts.bFactor;
  
  buf = malloc( readQtm );
  if( buf == NULL )
    {
      return -1;
    }
  
  (void)opcs_waitUntilDeviceIdle( tgtDevInfoP, taskStatus );
  *taskStatus = kSCSITaskStatus_GOOD;
  while( (err == kIOReturnSuccess) && (*taskStatus == kSCSITaskStatus_GOOD) )
    {
      memset( buf, 0, readQtm );
      readLen = readQtm;
      
      (void)opcs_waitUntilDeviceIdle( tgtDevInfoP, taskStatus );
      err = opcs_SCSIRead( tgtDevInfoP, buf, &readLen, taskStatus );
      if( err == kIOReturnSuccess )
	{
	  readLen = fwrite( buf, 1, readLen, file );
	}
    }
  
  (void)opcs_waitUntilDeviceIdle( tgtDevInfoP, taskStatus );
  
  free( buf );
  
  if( err == kSCSITaskStatus_CHECK_CONDITION )
    {
      err = kIOReturnSuccess;
    }
  return err;
}


IOReturn	opcs_SCSIRead( ScsiDevInfoRec *tgtDevInfoP, 
			       UInt8 *buf, UInt64 *lenByte, 
			       SCSITaskStatus *taskStatus )
{
  SCSICmdParam	param;
  UInt64	lenBlock;
  IOReturn	err = 0;
  
  memset( &param, 0, sizeof(SCSICmdParam) );
  memset( buf, 0, *lenByte );
  
  param.task = tgtDevInfoP->tgtDevTask;
  param.TgtToInitiatorBufLen = *lenByte;
  param.TgtToInitiatorBufPtr = buf;
  param.cdbSize = kSCSICDBSize_6Byte;
  param.cdb[0] = kSCSICmd_READ_6; // read : 0x08
  if( tgtDevInfoP->BlockLimits.isSupportVariableBlockLength == false )
    {
      int	BlockLength = tgtDevInfoP->opts.bSize;
      lenBlock = (*lenByte + BlockLength - 1) / BlockLength;
      // Fixed Length
      param.cdb[1] = 0x01;
      opcs_debugLog( "Read on Fixed Block : " );
    }
  else
    {
      lenBlock = *lenByte;
      // Variable Length and Suppress Incorrect Length Indicator
      param.cdb[1] = 0x02;
      opcs_debugLog( "Read on Variable Block : " );
    }
  opcs_debugLog( "Block Count = %lld\n", lenBlock );
  // set block count
  param.cdb[2] = (lenBlock >> 16) & 0xff;
  param.cdb[3] = (lenBlock >> 8) & 0xff;
  param.cdb[4] = lenBlock & 0xff;
  
  opcs_debugLog( "Request len = %lld\n", param.TgtToInitiatorBufLen );
  err = opcs_execScsiCmd( &param );
  *lenByte = param.transferCount;
  opcs_debugLog( "Read len = %lld\n", param.transferCount );
  *taskStatus = param.taskStatus;
  if( err != kIOReturnSuccess )
    {
      opcs_debugLog
	( "opcs_execScsiCmd() in opcs_SCSIRead() failed. err = %d\n", err );
      return err;
    }
  
  if( param.taskStatus != kSCSITaskStatus_GOOD )
    {
      if( param.taskStatus == kSCSITaskStatus_CHECK_CONDITION )
	{
	  opcs_SenseDataRec	senseData;
	  int			stat = 0;
	  
	  memset( &senseData, 0, sizeof(opcs_SenseDataRec) );
	  stat = opcs_analyzeSenseData( &(param.senseData), &senseData );
	  if( senseData.FileMark != 0 )
	    {
	      // end of data : normal end
	      if( opcs_getDebugLevel() != 0 )
		{
		  opcs_debugLog( "Detect CheckCondition but EOF is true\n" );
		}
	      param.taskStatus = kSCSITaskStatus_GOOD;
	    }
	  else
	    {
	      opcs_printSenseKeyMsg( &senseData );
	    }
	}
      opcs_debugLog( "taskStatus = 0x%08x\n", param.taskStatus );
      if( opcs_getDebugLevel() != 0 )
	{
	  opcs_printSenseString( &(param.senseData) );
	}
    }
  
  return param.taskStatus;
}


/*
  *endOfData = 0;
  
  //	error resolve
  if( param.taskStatus == kSCSITaskStatus_CHECK_CONDITION )
    {
      opcs_SenseDataRec	senseData;
      int		stat;
      int		mask;
      
      if( 0 < tgtDevInfoP->opts.debugLevel )
	{
	  PrintSenseString( &(param.senseData) );
	}
      
      memset( &senseData, 0, sizeof(opcs_SenseDataRec) );
      stat = opcs_analyzeSenseData( &(param.senseData), &senseData );
      if( senseData.ErrorClass != 7 )
	{
	  if( senseData.ErrorClass == 0 )
	    {
	      err = kIOReturnSuccess;
	    }
	  // ignore Error Class 0-6
	}
      else
	{
	  mask = senseErrBit_NoSense | senseErrBit_RecoveredError;
	  if( (stat & mask) == 0 )
	    {
	      if( senseData.Valid != 0 )
		{
		  int	*endOfData = 0;
		  if( senseData.FileMark != 0 )
		    {
		      opcs_debugLog( "FileMark or SetMark detected.\n" );
		      *endOfData = 1;
		    }  // end of if( senseData.FileMark != 0 )
		  else if( senseData.EOM != 0 )
		    {
		      opcs_debugLog( "Early Warning detected.\n" );
		      *endOfData = 1;
		    }
		  if( *endOfData == 1 )
		    {
		      if( (param.cdb[1] & 0x01) != 0 )
			{
			  // fixed block
			  opcs_debugLog( "read block count = " );
			}
		      else
			{
			  // fixed block
			  opcs_debugLog( "read byte length = " );
			}
		      opcs_debugLog( "%d\n", senseData.Info );
		    }
		} // end of if( senseData.Valid != 0 )
	    } // end of if( (stat & mask) == 0 )
	  else
	    {
	      opcs_printSenseKeyMsg( &senseData );
	      err = -1;
	    }
	} // end of if( senseData.ErrorClass != 7 ) - else
    } // end of if( param.taskStatus == kSCSITaskStatus_CHECK_CONDITION )
  if( err != kIOReturnSuccess )
    {
      return err;
    }
*/
