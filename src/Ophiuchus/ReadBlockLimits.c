//	ReadBlockLimits.c
//	Part of Ophiuchus
//	
//	Copyright (c) 2002-2023 tbd128 Allright Reserved.
//	This project is licensed under the MIT License, see the LICENSE.txt file for details.
//	
//	2.0   : Monday, March 11, 2002
//	2.0.1 : Sunday, March 24, 2002
//	2.1   : Monday, May 07, 2002
//

#include	<stdio.h>
#include	<unistd.h>

#include	<IOKit/IOKitLib.h>
#include	<IOKit/scsi/SCSITaskLib.h>
#include	<IOKit/scsi/SCSICommandOperationCodes.h>

#include	"ScsiDevInfo.h"
#include	"ScsiCmdExec.h"
#include	"CheckSenseStat.h"
#include	"ReadBlockLimits.h"
#include	"DebugLog.h"

#define	kBlckLimitsReplyDataLen	6

static	IOReturn	opcs_makeBlockLimitsData(UInt8 *data, 
						 ScsiDevInfoRec *tgtDevInfoP);

IOReturn	opcs_readBlockLimits( ScsiDevInfoRec *tgtDevInfoP, 
				      SCSITaskStatus *taskStatus )
{
  SCSICmdParam	param;
  UInt8		blockLimitsBuf[kBlckLimitsReplyDataLen];
  IOReturn	err = 0;
  
  memset( &param, 0, sizeof(SCSICmdParam) );
  memset( blockLimitsBuf, 0, kBlckLimitsReplyDataLen );
  
  param.task = tgtDevInfoP->tgtDevTask;
  
  param.TgtToInitiatorBufLen = kBlckLimitsReplyDataLen;
  param.TgtToInitiatorBufPtr = blockLimitsBuf;
  
  param.cdbSize = kSCSICDBSize_6Byte;
  param.cdb[0] = 0x05; //kSCSICmd_READ_BLOCK_LIMITS;//read block limits : 0x05
  
  err = opcs_execScsiCmd( &param );
  *taskStatus = param.taskStatus;
  if( err != kIOReturnSuccess )
    {
      return err;
    }
  
  if( param.taskStatus == kSCSITaskStatus_CHECK_CONDITION )
    {
      // retry
      opcs_debugLog( "Retry to ReadBlockLimits...\n", param.taskStatus );
      err = opcs_execScsiCmd( &param );
      *taskStatus = param.taskStatus;
      if( err != kIOReturnSuccess )
	{
	  return err;
	}
    }
  
  //	error resolve
  if( param.taskStatus == kSCSITaskStatus_CHECK_CONDITION )
    {
      opcs_SenseDataRec	senseData;
      int		stat;
      int		mask;
      
      if( opcs_getDebugLevel() != 0 )
	{
	  opcs_printSenseString( &(param.senseData) );
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
	  mask = senseKey_HardwareError | senseKey_IllegalRequest 
	    | senseKey_AbortedCommand;
	  if( (stat & mask) == 0 )
	    {
	      opcs_printSenseKeyMsg( &senseData );
	      err = kIOReturnSuccess;
	    }
	  else
	    {
	      err = -1;
	    }
	  // ignore other sense key
	}
    }
  if( err != kIOReturnSuccess )
    {
      return err;
    }
  
  err = opcs_makeBlockLimitsData( blockLimitsBuf, tgtDevInfoP );
  if( err != 0 )
    {
      return err;
    }
  
  return param.taskStatus;
}


static	IOReturn	opcs_makeBlockLimitsData( UInt8 *data, 
						  ScsiDevInfoRec *tgtDevInfoP )
{
  BlockLimitsRec	*BlockLimitsP = &(tgtDevInfoP->BlockLimits);
  
  BlockLimitsP->MaxBlockLen = data[1] * 0x10000 + data[2] * 0x100 + data[3];
  BlockLimitsP->MinBlockLen = data[4] * 0x100 + data[5];
  if( BlockLimitsP->MaxBlockLen == 0 )
    {
      fprintf( stderr, "unexpected error occured .. Min Block Length == 0\n" );
      return -1;
    }
  if( BlockLimitsP->MinBlockLen == 0 )
    {
      fprintf( stderr, "unexpected error occured .. Max Block Length == 0\n" );
      return -1;
    }
  
  if( BlockLimitsP->MaxBlockLen == BlockLimitsP->MinBlockLen )
    {
      BlockLimitsP->isSupportVariableBlockLength = false;
    }
  else
    {
      BlockLimitsP->isSupportVariableBlockLength = true;
    }
  return 0;
}


void	opcs_printBlockLimits( BlockLimitsRec *BlockLimitsP )
{
  fprintf( stderr, "----------- BlockLimits Data Begin -----------\n" );
  fprintf( stderr, "\tMax Block Limit Length = %d\n", 
	   BlockLimitsP->MaxBlockLen );
  fprintf( stderr, "\tMin Block Limit Length = %d\n", 
	   BlockLimitsP->MinBlockLen );
  
  if( (BlockLimitsP->MaxBlockLen == 0) || (BlockLimitsP->MinBlockLen == 0) )
    {
      fprintf( stderr, "\t -> Max or Min block limit is 0.\n" );
      fprintf( stderr, "\t    Maybe, meadia does not present...\n" );
    }
  else if( BlockLimitsP->isSupportVariableBlockLength == false )
    {
      fprintf(stderr, "\t -> This device only supports fixed block length.\n");
    }
  else
    {
      fprintf( stderr, "\t -> This device supports variable block length.\n" );
    }
  fprintf( stderr, "----------- BlockLimits Data End -----------\n" );
  
  return;
}
