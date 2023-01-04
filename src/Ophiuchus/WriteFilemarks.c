//	WriteFilemarks.c
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
#include	"DebugLog.h"

IOReturn	opcs_writeFilemarks( ScsiDevInfoRec *tgtDevInfoP, 
				     UInt32 fileMarkCount,
				     SCSITaskStatus *taskStatus )
{
  SCSICmdParam	param;
  IOReturn	err = 0;
  
  memset( &param, 0, sizeof(SCSICmdParam) );
  
  param.task = tgtDevInfoP->tgtDevTask;
  
  param.cdbSize = kSCSICDBSize_6Byte;
  param.cdb[0] = 0x10; //kSCSICmd_WRITE_FILEMARKS; /* rewind : 0x10 */
  param.cdb[1] = 0x00; // write file mark and wait for complete.
  param.cdb[2] = (fileMarkCount >> 16) & 0xff;
  param.cdb[3] = (fileMarkCount >> 8) & 0xff;
  param.cdb[4] = fileMarkCount & 0xff;
  
  err = opcs_execScsiCmd( &param );
  *taskStatus = param.taskStatus;
  if( err != kIOReturnSuccess )
    {
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
