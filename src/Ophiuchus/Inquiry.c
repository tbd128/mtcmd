//	Inquiry.c
//	Part of Ophiuchus
//	
//	Copyright (c) 2002-2023 tbd128 Allright Reserved.
//	This project is licensed under the MIT License, see the LICENSE.txt file for details.
//	
//	2.0   : Monday, March 11, 2002
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
#include	"Inquiry.h"
#include	"DebugLog.h"

static	void	opcs_makeInquiryData( UInt8 *data, UInt64 size, 
				      ScsiDevInfoRec *tgtDevInfoP );

IOReturn	opcs_inquiry( ScsiDevInfoRec *tgtDevInfoP, 
			      SCSITaskStatus *taskStatus )
{
  SCSICmdParam	param;
  UInt8		inqBuffer[kMyMaxSCSIStdInqDataLen];
  IOReturn	err = 0;
  
  memset( &param, 0, sizeof(SCSICmdParam) );
  memset( inqBuffer, 0, sizeof(kMyMaxSCSIStdInqDataLen) );
  
  param.task = tgtDevInfoP->tgtDevTask;
  
  param.TgtToInitiatorBufLen = kMyMaxSCSIStdInqDataLen;
  param.TgtToInitiatorBufPtr = inqBuffer;
  
  param.cdbSize = kSCSICDBSize_6Byte;
  param.cdb[0] = kSCSICmd_INQUIRY; // inquiry : 0x12
  param.cdb[4] = kMyMaxSCSIStdInqDataLen;
  
  err = opcs_execScsiCmd( &param );
  *taskStatus = param.taskStatus;
  if( err != kIOReturnSuccess )
    {
      return err;
    }
  
  if( param.taskStatus == kSCSITaskStatus_GOOD )
    {
      opcs_makeInquiryData( inqBuffer, param.transferCount, tgtDevInfoP );
    }
  else if( param.taskStatus == kSCSITaskStatus_CHECK_CONDITION )
    {
      // Something happened. Print the sense string
      opcs_printSenseString( &(param.senseData) );
    }
  else if( param.taskStatus != kSCSITaskStatus_GOOD )
    {
      opcs_debugLog( "taskStatus = 0x%08x\n", param.taskStatus );
    }
  
  return param.taskStatus;
}


static	void	opcs_makeInquiryData( UInt8 *data, UInt64 size, 
				      ScsiDevInfoRec *tgtDevInfoP )
{
  UInt8		str[256];
  UInt64	len;
  DeviceInquiryDataRec	*DeviceInquiryData = &(tgtDevInfoP->DeviceInquiryData);
  
  DeviceInquiryData->size = size;
  DeviceInquiryData->PeripheralQualifier = ((data[0]) >> 5) & 0x07;
  DeviceInquiryData->PeripheralDeviceType = (data[0]) & 0x31;
  DeviceInquiryData->isRemovable = (((data[1]) >> 7) & 0x1);
  DeviceInquiryData->DeviceTypeModifier = (data[1]) & 0x7f;
  DeviceInquiryData->IsoVersion = ((data[2]) >> 6) & 0x3;
  DeviceInquiryData->EcmaVersion = ((data[2]) >> 3) & 0x7;
  DeviceInquiryData->AnsiVersion = (data[2]) & 0x7;
  DeviceInquiryData->isSupportAenc = (((data[3]) >> 7) & 0x1);
  DeviceInquiryData->isSupportTerminateIOProcess = (((data[3]) >> 6) & 0x1);
  DeviceInquiryData->Reserved1 = ((data[3]) >> 4) & 0x3;
  DeviceInquiryData->ResponseDataType = ((data[3]) & 0xf);
  DeviceInquiryData->AdditionalDataLength = data[4];
  DeviceInquiryData->Reserved2 = data[5];
  DeviceInquiryData->Reserved3 = data[6];
  DeviceInquiryData->isSupportRlativeAddress = ((data[7] >> 7) & 0x1);
  DeviceInquiryData->isSupport32bitWideBus = ((data[7] >> 6) & 0x1);
  DeviceInquiryData->isSupport16bitWideBus = ((data[7] >> 5) & 0x1);
  DeviceInquiryData->isSupportSyncronousTransfer = ((data[7] >> 4) & 0x1);
  DeviceInquiryData->isSupportLinkedCommand = ((data[7] >> 3) & 0x1);
  DeviceInquiryData->Reserved4 = (data[7] >> 2) & 0x1;
  DeviceInquiryData->isSupportTaggedCommadQueing = ((data[7] >> 1) & 0x1);
  DeviceInquiryData->isSupportSoftResetCondition = ((data[7] >> 0) & 0x1);
  
  memmove( DeviceInquiryData->VendorID, &data[8], 
	   kINQUIRY_VENDOR_IDENTIFICATION_Length );
  str[kINQUIRY_VENDOR_IDENTIFICATION_Length] = '\0';
  
  memmove( DeviceInquiryData->ProductID, &data[16], 
	   kINQUIRY_PRODUCT_IDENTIFICATION_Length );
  str[kINQUIRY_PRODUCT_IDENTIFICATION_Length] = '\0';

  memmove( DeviceInquiryData->ProductVersion, &data[32], 
	   kINQUIRY_PRODUCT_REVISION_LEVEL_Length );
  str[kINQUIRY_PRODUCT_REVISION_LEVEL_Length] = '\0';
  
  if( 0 < (SInt64)(size - kMyMinSCSIStdInqDataLen) )
    {
      len = size - kMyMinSCSIStdInqDataLen;
      if( kMyFirstVendorSpecificLen < len )
	{
	  len = kMyFirstVendorSpecificLen;
	}
      memmove( DeviceInquiryData->VendorSpecific1, 
	       &data[kMyMinSCSIStdInqDataLen], len );
      DeviceInquiryData->VendorSpecific1[len] = '\0';
    }
  else
    {
      DeviceInquiryData->VendorSpecific1[0] = '\0';
    }
  
  if( 0 < (SInt64)(size - kMySecondVendorSpecific) )
    {
      len = size - kMySecondVendorSpecific;
      if( kMySecondVendorSpecificLen < len )
	{
	  len = kMySecondVendorSpecificLen;
	}
      memmove( DeviceInquiryData->VendorSpecific2, 
	       &data[kMySecondVendorSpecific], len );
      DeviceInquiryData->VendorSpecific2[len] = '\0';
    }
  else
    {
      DeviceInquiryData->VendorSpecific2[0] = '\0';
    }
  
  return;
}


void	opcs_printInquiryDataSummary( DeviceInquiryDataRec *DeviceInquiryData )
{
  fprintf( stderr, "%s%s%s\n", DeviceInquiryData->VendorID, 
	   DeviceInquiryData->ProductID, DeviceInquiryData->ProductVersion );
}


void	opcs_printInquiryData( DeviceInquiryDataRec *DeviceInquiryData )
{
  fprintf( stderr, "----------- Inquiry Data Begin -----------\n" );
  fprintf( stderr, "\tInquiry Data Size = 0x%llx (= %lld)\n", 
	   DeviceInquiryData->size, DeviceInquiryData->size );
  fprintf( stderr, "\tPeripheral Qualifier = 0x%x\n", 
	  DeviceInquiryData->PeripheralQualifier );
  fprintf( stderr, "\tPeripheral Device Type = 0x%x\n", 
	  DeviceInquiryData->PeripheralDeviceType );
  fprintf( stderr, "\tisRemovable = %s\n", 
	   DeviceInquiryData->isRemovable ? "Yes" : "No");
  fprintf( stderr, "\tDevice-type Modifier = 0x%x\n", 
	   DeviceInquiryData->DeviceTypeModifier );
  //fprintf( stderr, "Device-type Modifier = 0x%x, ", (data[1]) & 0x7f );
  //fprintf( stderr, "(%s)\n", ((data[1]) & 0x7f) == 0 ? "SCSI-2" : "SCSI-1" );
  fprintf( stderr, "\tISO Version = 0x%x\n", DeviceInquiryData->IsoVersion );
  fprintf( stderr, "\tECMA Version = 0x%x\n", DeviceInquiryData->EcmaVersion );
  fprintf( stderr, "\tANSI Version = 0x%x, ", DeviceInquiryData->AnsiVersion );
  switch( DeviceInquiryData->AnsiVersion )
    {
    case 0:
      fprintf( stderr, "It does not matched ANSI SCSI standard.\n" );
      break;
    case 1:
      fprintf( stderr, "SCSI-1 (ANSI X3.131-1986)\n" );
      break;
    case 2:
      fprintf( stderr, "SCSI-2 (ANSI X3.131-1994)\n" );
      break;
    default:
      fprintf( stderr, "(Reserved)\n" );
      break;
    }
  fprintf( stderr, "\tSupport AENC = %s\n", 
	   DeviceInquiryData->isSupportAenc ? "Yes" : "No" );
  fprintf( stderr, "\tSupport Terminate I/O Process = %s\n", 
	   DeviceInquiryData->isSupportTerminateIOProcess ? "Yes" : "No" );
  fprintf( stderr, "\t(Reserved) = 0x%x\n", DeviceInquiryData->Reserved1 );
  fprintf( stderr, "\tResponse Data Type = 0x%x, ", 
	   DeviceInquiryData->ResponseDataType );
  switch( DeviceInquiryData->ResponseDataType )
    {	
    case 0:
      fprintf( stderr, "SCSI-1 (ANSI X3.131-1986)\n" );
      break;
    case 1:
      fprintf( stderr, "CCS (ANSI X3T9.2/85-52)\n" );
      break;
    case 2:
      fprintf( stderr, "SCSI-2 (ANSI X3.131-1994)\n" );
      break;
    default:
      fprintf( stderr, "(Reserved)\n" );
      break;
    }
  fprintf( stderr, "\t(Additional Data Length) = 0x%x\n", 
	   DeviceInquiryData->AdditionalDataLength );
  fprintf( stderr, "\t(Reserved) = 0x%x\n", DeviceInquiryData->Reserved2 );
  fprintf( stderr, "\t(Reserved) = 0x%x\n", DeviceInquiryData->Reserved3 );
  fprintf( stderr, "\tSupport Rlative Address = %s\n", 
	   DeviceInquiryData->isSupportRlativeAddress ? "Yes" : "No" );
  fprintf( stderr, "\tSupport 32bit Wide Bus = %s\n", 
	   DeviceInquiryData->isSupport32bitWideBus ? "Yes" : "No" );
  fprintf( stderr, "\tSupport 16bit Wide Bus = %s\n", 
	   DeviceInquiryData->isSupport16bitWideBus ? "Yes" : "No" );
  fprintf( stderr, "\tSupport Syncronous Transfer = %s\n", 
	   DeviceInquiryData->isSupportSyncronousTransfer ? "Yes" : "No" );
  fprintf( stderr, "\tSupport Linked Command = %s\n", 
	   DeviceInquiryData->isSupportLinkedCommand ? "Yes" : "No" );
  fprintf( stderr, "\t(Reserved)  = 0x%x\n", DeviceInquiryData->Reserved3 );
  fprintf( stderr, "\tSupport Tagged Commad Queing = %s\n", 
	   DeviceInquiryData->isSupportTaggedCommadQueing ? "Yes" : "No" );
  fprintf( stderr, "\tSupport Soft Reset Condition = %s\n", 
	   DeviceInquiryData->isSupportSoftResetCondition ? "Yes" : "No" );
  fprintf( stderr, "\tVendor ID = \"%s\"\n", DeviceInquiryData->VendorID );
  fprintf( stderr, "\tProduct ID = \"%s\"\n", DeviceInquiryData->ProductID );
  fprintf( stderr, "\tProduct Version = \"%s\"\n", 
	   DeviceInquiryData->ProductVersion );
  fprintf( stderr, "\tVendor Specific 1 = " );
  if( 0 < strlen( (char *)DeviceInquiryData->VendorSpecific1 ) )
    {
      fprintf( stderr, "\"%s\"\n", DeviceInquiryData->VendorSpecific1 );
    }
  else
    {
      fprintf( stderr, "N/A\n" );
    }
  fprintf( stderr, "\tVendor Specific 2 = " );
  if( 0 < strlen( (char *)DeviceInquiryData->VendorSpecific2 ) )
    {
      fprintf( stderr, "\"%s\"\n", DeviceInquiryData->VendorSpecific2 );
    }
  else
    {
      fprintf( stderr, "N/A\n" );
    }
  fprintf( stderr, "----------- Inquiry Data End -----------\n" );
  
  return;
}
