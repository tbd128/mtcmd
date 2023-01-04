//	Write.c
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
#include	"WriteFilemarks.h"
#include	"Write.h"
#include	"TestUnitReady.h"
#include	"ModeSenseSelect.h"
#include	"DebugLog.h"

IOReturn	opcs_write( ScsiDevInfoRec *tgtDevInfoP, FILE *file,
			    SCSITaskStatus *taskStatus )
{
  UInt8			*buf;
  UInt64		len;
  UInt64		writeLen;
  UInt64		writeQtm;
  IOReturn		err = kIOReturnSuccess;
  
  writeQtm = tgtDevInfoP->opts.bSize * tgtDevInfoP->opts.bFactor;
  
  buf = malloc( writeQtm );
  if( buf == NULL )
    {
      return -1;
    }
  
  while( feof( file ) == 0 )
    {
      memset( buf, 0, writeQtm );
      len = fread( buf, 1, writeQtm, file );
      opcs_debugLog( "Read from fread() = %lld\n", len );
      if( 0 < len )
	{
	  (void)opcs_waitUntilDeviceIdle( tgtDevInfoP, taskStatus );
	  writeLen = len;
	  err = opcs_SCSIWrite( tgtDevInfoP, buf, &writeLen, taskStatus );
	  if( err != kIOReturnSuccess )
	    {
	      return err;
	    }
	}
    }
  
  (void)opcs_waitUntilDeviceIdle( tgtDevInfoP, taskStatus );
  opcs_debugLog( "write file mark\n" );
  err = opcs_writeFilemarks( tgtDevInfoP, 1, taskStatus );
  if( err != kIOReturnSuccess )
    {
      return err;
    }
  
  free( buf );
  return err;
}


IOReturn	opcs_SCSIWrite( ScsiDevInfoRec *tgtDevInfoP, 
				UInt8 *buf, UInt64 *lenByte, 
				SCSITaskStatus *taskStatus )
{
  SCSICmdParam	param;
  UInt64	lenBlock;
  IOReturn	err = 0;
  
  memset( &param, 0, sizeof(SCSICmdParam) );
  
  param.task = tgtDevInfoP->tgtDevTask;
  
  param.InitiatorToTgtBufLen = *lenByte;
  param.InitiatorToTgtBufPtr = buf;
  param.cdbSize = kSCSICDBSize_6Byte;
  param.cdb[0] = kSCSICmd_WRITE_6; // write
  if( tgtDevInfoP->BlockLimits.isSupportVariableBlockLength == false )
    {
      int	BlockLength = tgtDevInfoP->opts.bSize;
      lenBlock = (*lenByte + BlockLength - 1) / BlockLength;
      // Fixed Length
      param.cdb[1] = 0x01;
      opcs_debugLog( "Write on Fixed Block : " );
    }
  else
    {
      lenBlock = *lenByte;
      // Variable Length
      param.cdb[1] = 0x00;
      opcs_debugLog( "Write on Variable Block : " );
    }
  opcs_debugLog( "Block Count = %d\n", (int)lenBlock );
  // set block count
  param.cdb[2] = (lenBlock >> 16) & 0xff;
  param.cdb[3] = (lenBlock >> 8) & 0xff;
  param.cdb[4] = lenBlock & 0xff;
  
  opcs_debugLog( "Request len = %lld\n", param.InitiatorToTgtBufLen );
  err = opcs_execScsiCmd( &param );
  *lenByte = param.transferCount;
  opcs_debugLog( "Wrote len = %lld\n", param.transferCount );
  *taskStatus = param.taskStatus;
  if( err != kIOReturnSuccess )
    {
      opcs_debugLog
	( "opcs_execScsiCmd() in opcs_SCSIWrite() failed. err = %d\n", err );
      return err;
    }
  
  if( param.taskStatus != kSCSITaskStatus_GOOD )
    {
      if( param.taskStatus == kSCSITaskStatus_CHECK_CONDITION )
	{
	  opcs_SenseDataRec	senseData;
	  int			stat;
	  
	  memset( &senseData, 0, sizeof(opcs_SenseDataRec) );
	  stat = opcs_analyzeSenseData( &(param.senseData), &senseData );
	  opcs_printSenseKeyMsg( &senseData );
	}
      opcs_debugLog( "taskStatus = 0x%08x\n", param.taskStatus );
      if( opcs_getDebugLevel() != 0 )
	{
	  opcs_printSenseString( &(param.senseData) );
	}
    }
  
  return param.taskStatus;
}

