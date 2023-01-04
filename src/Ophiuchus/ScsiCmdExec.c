//	ScsiCmdExec.c
//	Part of Ophiuchus
//	
//	Copyright (c) 2002-2023 tbd128 Allright Reserved.
//	This project is licensed under the MIT License, see the LICENSE.txt file for details.
//
//	2.0   : Monday, March 11, 2002
//	2.1   : Monday, May 07, 2002
//
//	This program is based on : 
//		Inside Mac OS X
//		Accessing Hardware From Applications (December 2001)
//		Chapter 4 : Working With SCSI Architecture Model Devices
//		Apple Computer Inc.
//

#include	<stdio.h>
#include	<unistd.h>

#include	<Kernel/mach/mach_port.h>
#include	<IOKit/IOKitLib.h>
#include	<IOKit/scsi/SCSITaskLib.h>
#include	<IOKit/scsi/SCSICommandOperationCodes.h>

#include	"ScsiDevInfo.h"
#include	"ScsiCmdExec.h"
#include	"Inquiry.h"
#include	"ReadBlockLimits.h"
#include	"ModeSenseSelect.h"

static	kern_return_t	opcs_testDevice( io_service_t service, 
					 ScsiDevInfoRec *tgtDevInfoP );

kern_return_t	opcs_createScsiInterface(ScsiDevInfoRec **tgtDevInfoP, int cnt)
{
  kern_return_t			err = KERN_SUCCESS;
  mach_port_t			masterPort = 0;
  CFMutableDictionaryRef	matchingDict;
  CFMutableDictionaryRef	subDict;
  io_iterator_t			iterator;
  io_service_t			obj = 0;
  int				i;
  
  if( cnt < 1 )
    {
      fprintf( stderr, "Target No. was not specified.\n" );
      fprintf( stderr, "You must specify target No.\n" );
    }
  
  err = IOMainPort( MACH_PORT_NULL, &masterPort );
  if( err != KERN_SUCCESS )
    {
      fprintf( stderr, "IOMasterPort failed : errcode = %d\n", err );
      return err;
    }
  
  matchingDict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, NULL, NULL);
  subDict = CFDictionaryCreateMutable( kCFAllocatorDefault, 0, NULL, NULL );
  
  CFDictionarySetValue( subDict, 
			CFSTR( kIOPropertySCSITaskDeviceCategory ), 
			CFSTR( kIOPropertySCSITaskUserClientDevice ) );
  
  CFDictionarySetValue( matchingDict, CFSTR( kIOPropertyMatchKey ), subDict );
  
  err = IOServiceGetMatchingServices( masterPort, matchingDict,  &iterator );
  if( err != KERN_SUCCESS )
    {
      fprintf( stderr, 
	       "IOServiceGetMatchingServices failed : errcode = %d\n", err );
      return err;
    }
  
  *tgtDevInfoP = NULL;
  for( i = 0 ; i < cnt ; i ++ )
    {
      obj = IOIteratorNext( iterator );
      if( obj == 0 )
	{
	  break;
	}
    }
  if( obj != 0 )
    {
      *tgtDevInfoP = malloc( sizeof(ScsiDevInfoRec) );
      if( *tgtDevInfoP == NULL )
	{
	  return -1;
	}
      memset( *tgtDevInfoP, 0, sizeof(ScsiDevInfoRec) );
      opcs_testDevice( obj, *tgtDevInfoP );
      err = IOObjectRelease( obj );
    }
  
  mach_port_deallocate( mach_task_self(), masterPort );
  
  return err;
}


void	opcs_releaseScsiInterface( ScsiDevInfoRec **tgtDevInfoP )
{
  SCSITaskDeviceInterface	**interface = NULL;
  SCSITaskInterface		**task = NULL;
  
  if( *tgtDevInfoP != NULL )
    {
      interface = (*tgtDevInfoP)->tgtDevInterface;
      task = (*tgtDevInfoP)->tgtDevTask;
      if( task != NULL )
	{
	  (*task)->Release( task );
	}
      if( interface != NULL )
	{
	  (*interface)->ReleaseExclusiveAccess( interface );
	  (*interface)->Release( interface );
	}
      free( *tgtDevInfoP );
    }
  *tgtDevInfoP = NULL;
}


static	kern_return_t	opcs_testDevice( io_service_t service, 
					 ScsiDevInfoRec *tgtDevInfoP )
{
  SInt32			score;
  HRESULT			herr;
  kern_return_t			err;
  IOCFPlugInInterface		**plugInInterface = NULL;
  SCSITaskDeviceInterface	**interface = NULL;
  SCSITaskInterface		**task = NULL;
  CFUUIDBytes			iid; /* 128bit struct contains the raw UUID */
  SCSITaskStatus		taskStatus;
  
  // Create the IOCFPlugIn interface so we can query it.
  err = IOCreatePlugInInterfaceForService( service,
					   kIOSCSITaskDeviceUserClientTypeID,
					   kIOCFPlugInInterfaceID,
					   &plugInInterface,
					   &score );
  if( err != KERN_SUCCESS )
    {
      fprintf( stderr, "IOCreatePlugInInterfaceForService returned 0x%x\n", err );
      return err;
    }
  // Query the interface for the MMCDeviceInterface.
  iid = CFUUIDGetUUIDBytes( kIOSCSITaskDeviceInterfaceID );
  herr = (*plugInInterface)->QueryInterface( plugInInterface,iid, 
					     (LPVOID)&interface );
  if( herr != S_OK )
    {
      fprintf( stderr, "QueryInterface returned 0x%x\n", herr );
      return herr;
    }
  if( interface == NULL )
    {
      fprintf( stderr, "GetSCSITaskDeviceInterface returned NULL\n" );
      return -1;
    }
  err = (*interface)->ObtainExclusiveAccess( interface );
  if( err != KERN_SUCCESS )
    {
      fprintf( stderr, "ObtainExclusiveAccess returned %d\n", err );
      return err;
    }
  
  (*plugInInterface)->Release( plugInInterface );
  
  tgtDevInfoP->tgtDevInterface = interface;
  
  // Create a task now that we have exclusive access
  task = (*interface)->CreateSCSITask( interface );
  if( task == NULL )
    {
      fprintf( stderr, "*** CreateSCSITask() faied ***\n\n");
      return -1;
    }
  tgtDevInfoP->tgtDevTask = task;
  
  // get target device inquiry data for future use.
  err = opcs_inquiry( tgtDevInfoP, &taskStatus );
  if( err != KERN_SUCCESS )
    {
      return err;
    }
  // get target device BlockLimits for future use.
  err = opcs_readBlockLimits( tgtDevInfoP, &taskStatus );
  if( err != KERN_SUCCESS )
    {
      return err;
    }
  
  // get mode sense data : compression page data for future use.
  err = opcs_modeSense( tgtDevInfoP, 0x0f, &taskStatus );
  if( err != KERN_SUCCESS )
    {
      // target device does not support compression page.
      // get header and block descriptor for future use.
      err = opcs_modeSense( tgtDevInfoP, 0x00, &taskStatus );
      if( err != KERN_SUCCESS )
	{
	  return err;
	}
    }
  
  return err;
}


IOReturn	opcs_execScsiCmd( SCSICmdParam *param )
{
  SCSITaskInterface		**task = NULL;
  IOReturn			err = 0;
  IOVirtualRange		*outRange = NULL;
  IOVirtualRange		*inRange = NULL;
  
  memset( &(param->senseData), 0, sizeof(param->senseData) );
  
  task = param->task;
  if( task == NULL )
    {
      fprintf( stderr, "Assertion failed... (task == NULL)\n");
      return -1;
    }
  
  memset( &(param->senseData), 0, sizeof(param->senseData) );
  
  if( param->InitiatorToTgtBufPtr != NULL )
    {
      outRange = (IOVirtualRange *)malloc( sizeof(IOVirtualRange) );
      if( outRange == NULL )
	{
	  fprintf( stderr, "*** ERROR Mallocing IOVirtualRange ***\n\n");
	  err = -1;
	  goto EXIT_POINT;
	}
      outRange->address = (IOVirtualAddress)(param->InitiatorToTgtBufPtr);
      outRange->length = param->InitiatorToTgtBufLen;
    }
  if( param->TgtToInitiatorBufPtr != NULL )
    {
      inRange = (IOVirtualRange *)malloc( sizeof(IOVirtualRange) );
      if( inRange == NULL )
	{
	  fprintf( stderr, "*** ERROR Mallocing IOVirtualRange ***\n\n");
	  err = -1;
	  goto EXIT_POINT;
	}
      inRange->address = (IOVirtualAddress)(param->TgtToInitiatorBufPtr);
      inRange->length = param->TgtToInitiatorBufLen;
    }
  
  // Set the actual CDB in the task
  err = (*task)->SetCommandDescriptorBlock( task, param->cdb, param->cdbSize );
  if( err != kIOReturnSuccess )
    {
      fprintf(stderr,"(*task)->SetCommandDescriptorBlock() failed. err = %d\n",
	      err);
      goto EXIT_POINT;
    }
  
  // Set the scatter-gather entry in the task
  if( outRange != NULL )
    {
      err = (*task)->SetScatterGatherEntries( task, outRange, 1, 
					      param->InitiatorToTgtBufLen,
					      kSCSIDataTransfer_FromInitiatorToTarget );
      if( err != kIOReturnSuccess )
	{
	  fprintf( stderr, "(*task)->SetScatterGatherEntries() failed. err = %d\n", err );
	  goto EXIT_POINT;
	}
    }
  if( inRange != NULL )
    {
      err = (*task)->SetScatterGatherEntries( task, inRange, 1, 
					      param->TgtToInitiatorBufLen, 
					      kSCSIDataTransfer_FromTargetToInitiator );
      if( err != kIOReturnSuccess )
	{
	  fprintf( stderr, "(*task)->SetScatterGatherEntries() failed. err = %d\n", err );
	  goto EXIT_POINT;
	}
    }
  
  // Set the timeout in the task
  // timeout is 1 hour. 
  // (60[mimutes] * 60[seconds] * 1000[milli-seconds])
  err = (*task)->SetTimeoutDuration( task, (60 * 60 * 1000) );
  if( err != kIOReturnSuccess )
    {
      fprintf(stderr, "(*task)->SetTimeoutDuration() failed. err = %d\n", err);
      goto EXIT_POINT;
    }
  
  // Execute the command
  err = (*task)->ExecuteTaskSync( task, &(param->senseData), 
				  &(param->taskStatus), 
				  &(param->transferCount) );
  if( err != kIOReturnSuccess )
    {
      fprintf(stderr, "(*task)->ExecuteTaskSync() failed. err = %d\n", err);
      goto EXIT_POINT;
    }
  
   EXIT_POINT:
  // post process
  if( outRange != NULL )
    {
      free( outRange );
    }
  if( inRange != NULL )
    {
      free( inRange );
    }
  
  return err;
}
