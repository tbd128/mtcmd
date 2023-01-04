//	Erase.c
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
#include	"Erase.h"
#include	"DebugLog.h"

IOReturn	opcs_erase( ScsiDevInfoRec *tgtDevInfoP, 
			    SCSITaskStatus *taskStatus )
{
  SCSICmdParam	param;
  IOReturn	err = 0;
  
  memset( &param, 0, sizeof(SCSICmdParam) );
  
  param.task = tgtDevInfoP->tgtDevTask;
  
  param.cdbSize = kSCSICDBSize_6Byte;
  param.cdb[0] = 0x19; //kSCSICmd_ERASE
  param.cdb[1] = 0x01; // wait to complete and erase after cur.pos.
  
  err = opcs_execScsiCmd( &param );
  *taskStatus = param.taskStatus;
  if( err != kIOReturnSuccess )
    {
      opcs_debugLog("MyExecScsiCmd() in MySCSIRead() failed. err = %d\n",err);
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


/*
  //	error resolve
  if( param.taskStatus == kSCSITaskStatus_CHECK_CONDITION )
    {
      opcs_SenseDataRec	senseData;
      int		stat;
      int		mask;
      
      if( opcs_getDebugLevel() != 0 )
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
*/
