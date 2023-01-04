//	ModeSenseSelect.c
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
#include	"DensityCode.h"
#include	"ModeSenseSelect.h"
#include	"DebugLog.h"

static  void    opcs_deCompositeModePage( ScsiDevInfoRec *tgtDevInfoP, 
					  int pageCode );
#if 0
static  void    opcs_compositeModePage( UInt8 *data, UInt64 *len, 
					ScsiDevInfoRec *tgtDevInfoP );
#endif

//////////////////////////////////////////////////////////////////////////
//	mode sense and select interface
//////////////////////////////////////////////////////////////////////////

IOReturn	opcs_modeSense( ScsiDevInfoRec *tgtDevInfoP, 
				int pageCode, SCSITaskStatus *taskStatus )
{
  SCSICmdParam	param;
  IOReturn	err = 0;
  UInt64	bufLenByte = 0;
  UInt8		*senseDataBuffer;
  
  senseDataBuffer = tgtDevInfoP->ModePage.ModePageHeader.SenseDataBuffer;
  
  memset( &param, 0, sizeof(SCSICmdParam) );
  memset( senseDataBuffer, 0, kMyModeSenseDataBufLen );
  
  param.task = tgtDevInfoP->tgtDevTask;
  
  param.TgtToInitiatorBufLen = kMyModeSenseDataBufLen;
  param.TgtToInitiatorBufPtr = senseDataBuffer;
  
  param.cdbSize = kSCSICDBSize_6Byte;
  param.cdb[0] = kSCSICmd_MODE_SENSE_6;	// mode sense : 0x1a
  param.cdb[1] = 0x00;
  //param.cdb[2] = 0x00;
  param.cdb[2] = pageCode;	// get the current data of the page code
  param.cdb[3] = 0x00;
  param.cdb[4] = kMyModeSenseDataBufLen;
  
  err = opcs_execScsiCmd( &param );
  *taskStatus = param.taskStatus;
  if( err != kIOReturnSuccess )
    {
      return err;
    }
  bufLenByte = param.transferCount;
  
  if( param.taskStatus == kSCSITaskStatus_GOOD )
    {
      tgtDevInfoP->ModePage.ModePageHeader.SenseDataBufLen = bufLenByte;
      opcs_deCompositeModePage( tgtDevInfoP, pageCode );
    }
  else
    {
      if( param.taskStatus == kSCSITaskStatus_CHECK_CONDITION )
	{
	  opcs_SenseDataRec	senseData;
	  int			stat;
	  
	  memset( &senseData, 0, sizeof(opcs_SenseDataRec) );
	  stat = opcs_analyzeSenseData( &(param.senseData), &senseData );
	  if( pageCode == 0x00 )
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
  /*
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
  */
  return param.taskStatus;
}


IOReturn	opcs_modeSelect( ScsiDevInfoRec *tgtDevInfoP, 
				 ModeSelectPageType PageFormat, 
				 SCSITaskStatus *taskStatus )
{
  SCSICmdParam	param;
  UInt8		*senseDataBuffer;
  UInt64	bufLenByte;
  IOReturn	err = 0;
  
  senseDataBuffer = tgtDevInfoP->ModePage.ModePageHeader.SenseDataBuffer;
  bufLenByte = tgtDevInfoP->ModePage.ModePageHeader.SenseDataBufLen;
  
  memset( &param, 0, sizeof(SCSICmdParam) );
  
  param.task = tgtDevInfoP->tgtDevTask;
  
  param.InitiatorToTgtBufLen = bufLenByte;
  param.InitiatorToTgtBufPtr = senseDataBuffer;
  
  param.cdbSize = kSCSICDBSize_6Byte;
  param.cdb[0] = kSCSICmd_MODE_SELECT_6; // mode select : 0x15
  if( PageFormat == NonPageFormat )
    {
      param.cdb[1] = 0x00;	// non-page format
    }
  else
    {
      param.cdb[1] = 0x10;	// page format
    }
  param.cdb[2] = 0x00;
  param.cdb[3] = 0x00;
  param.cdb[4] = bufLenByte;
  
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


//////////////////////////////////////////////////////////////////////////
//	mode page data managiment
//////////////////////////////////////////////////////////////////////////
static  void    opcs_deCompositeModePage( ScsiDevInfoRec *tgtDevInfoP, 
					  int pageCode )
{
  ModePageRec		*ModePageP = &(tgtDevInfoP->ModePage);
  ModePageHeaderRec	*ModePageHeaderP = &(ModePageP->ModePageHeader);
  UInt8			*data;
  UInt64		len;
  UInt64		headerLen;
  CompressionPageRec	*cpDataP = &(ModePageP->CompressionPage);
  
  len = ModePageHeaderP->SenseDataBufLen;
  data = ModePageHeaderP->SenseDataBuffer;
  
  cpDataP->isSupportCompressionPage = false;
  
  //	each base data
  ModePageHeaderP->ModeParamLen = data[0];
  ModePageHeaderP->MediaType = data[1];
  ModePageHeaderP->isWriteProtected = (((data[2]) >> 7) & 0x1);
  ModePageHeaderP->BufferingMode = (((data[2]) >> 4) & 0x7);
  ModePageHeaderP->Speed = ((data[2]) & 0xf);
  ModePageHeaderP->BlockDescLen = data[3];
  //	block desciptor
  if( 0 < ModePageHeaderP->BlockDescLen )
    {
      ModePageHeaderP->isSupportBlockDesc = true;
      ModePageHeaderP->DensityCode = data[4];
      ModePageHeaderP->NumberOfBlocks = data[5] * 0x10000;
      ModePageHeaderP->NumberOfBlocks += data[6] * 0x100;
      ModePageHeaderP->NumberOfBlocks += data[7] * 0x1;
      ModePageHeaderP->reserved = data[8];
      ModePageHeaderP->BlockLength = data[9] * 0x10000;
      ModePageHeaderP->BlockLength += data[10] * 0x100;
      ModePageHeaderP->BlockLength += data[11] * 0x1;
    }
  else
    {
      ModePageHeaderP->isSupportBlockDesc = false;
    }
  headerLen = 4/*HeaderLen*/ + ModePageHeaderP->BlockDescLen;
  //	Additional Page Data
  ModePageHeaderP->AdditionalPageDataLen = len - headerLen;
  if( 0 < ModePageHeaderP->AdditionalPageDataLen )
    {
      memcpy( ModePageHeaderP->AdditionalPageData, &data[headerLen], 
	      ModePageHeaderP->AdditionalPageDataLen );
    }
  
  switch( pageCode )
    {
    case 0x0f:
	cpDataP->CompressionPageLen = data[headerLen + 1];
	cpDataP->DataCompressionEnable = ((data[headerLen + 2]) >> 7) & 0x1;
	cpDataP->DataCompressionCapable = ((data[headerLen + 2]) >> 6) & 0x1;
	cpDataP->reserved1 = data[headerLen + 2]  & 0x3f;
	cpDataP->DataDecompressionEnable = ((data[headerLen + 3]) >> 7) & 0x1;
	cpDataP->ReportExceptionOnDecompression 
	  = ((data[headerLen + 3]) >> 5) & 0x3;
	cpDataP->reserved2 = data[headerLen + 3] & 0x1f;
	cpDataP->CompressionAlgorithm = data[headerLen + 4] * 0x1000000;
	cpDataP->CompressionAlgorithm += data[headerLen + 5] * 0x10000;
	cpDataP->CompressionAlgorithm += data[headerLen + 6] * 0x100;
	cpDataP->CompressionAlgorithm += data[headerLen + 7] * 0x1;
	cpDataP->DecompressionAlgorithm = data[headerLen + 8] * 0x1000000;
	cpDataP->DecompressionAlgorithm += data[headerLen + 9] * 0x10000;
	cpDataP->DecompressionAlgorithm += data[headerLen + 10] * 0x100;
	cpDataP->DecompressionAlgorithm += data[headerLen + 11] * 0x1;
	cpDataP->AdditionalDataLen = cpDataP->CompressionPageLen + 2 - 12;
	memcpy( cpDataP->AdditionalData, &(data[headerLen + 12]), 
		cpDataP->AdditionalDataLen );
	
	cpDataP->isSupportCompressionPage = cpDataP->DataCompressionCapable;
      break;
      
    default:
      break;
    }
}

#if 0
static  void    opcs_compositeModePage( UInt8 *data, UInt64 *len, 
					ScsiDevInfoRec *tgtDevInfoP )
{
  ModePageRec		*ModePageP = &(tgtDevInfoP->ModePage);
  ModePageHeaderRec	*ModePageHeaderP = &(ModePageP->ModePageHeader);
  
  //memset( ModePageP, 0, sizeof(ModePageRec) );
  
  *len = ModePageHeaderP->SenseDataBufLen;
  
  //	each base data
  data[0] = ModePageHeaderP->ModeParamLen;
  data[1] = ModePageHeaderP->MediaType;
  data[2] = (ModePageHeaderP->isWriteProtected << 7)
    | (ModePageHeaderP->BufferingMode << 4)
    | ModePageHeaderP->Speed;
  data[3] = ModePageHeaderP->BlockDescLen;
  //	block desciptor
  if( ModePageHeaderP->isSupportBlockDesc == true )
    {
      data[4] = ModePageHeaderP->DensityCode;
      data[5] = ((ModePageHeaderP->NumberOfBlocks >> 16) & 0xff);
      data[6] = ((ModePageHeaderP->NumberOfBlocks >> 8) & 0xff);
      data[7] = (ModePageHeaderP->NumberOfBlocks & 0xff);
      data[8] = ModePageHeaderP->reserved;
      data[9] = ((ModePageHeaderP->BlockLength >> 16) & 0xff);
      data[10] = ((ModePageHeaderP->BlockLength >> 8) & 0xff);
      data[11] = (ModePageHeaderP->BlockLength & 0xff);
      //	Additional Page Data
      if( 0 < ModePageHeaderP->AdditionalPageDataLen )
	{
	  memcpy( &data[12], ModePageHeaderP->AdditionalPageData, 
		  ModePageHeaderP->AdditionalPageDataLen );
	}
    }
}
#endif

void	opcs_printModeSense( const ScsiDevInfoRec *tgtDevInfoP )
{
  const ModePageRec	*ModePageP = &(tgtDevInfoP->ModePage);
  const ModePageHeaderRec *ModePageHeaderP = &(ModePageP->ModePageHeader);
  UInt64		len = ModePageHeaderP->SenseDataBufLen;
  const	UInt8		*data = ModePageHeaderP->SenseDataBuffer;
  const CompressionPageRec *cpDataP = &(ModePageP->CompressionPage);
  int		i;
  
  fprintf( stderr, "----------- ModeSense Data Begin -----------\n" );
  if( len != 0 )
    {
      fprintf( stderr, "\tlen = %lld (0x%llx)\n", len, len );
      fprintf( stderr, "\t----------- Raw Data Begin -----------\n" );
      fprintf( stderr, "\t" );
      for( i = 0 ; i < len ; i++ )
	{
	  fprintf( stderr, "%02x ", data[i] );
	}
      fprintf( stderr, "\n" );
      fprintf( stderr, "\t----------- Raw Data End -----------\n" );
      fprintf( stderr, "\tMode Parameter Length = %d\n", 
	       ModePageHeaderP->ModeParamLen );
      fprintf( stderr, "\tMedia Type = %d\n", ModePageHeaderP->MediaType );
      fprintf( stderr, "\tWrite Protected = %s\n", 
	       ModePageHeaderP->isWriteProtected ? "Yes" : "No" );
      fprintf( stderr, "\tBuffering Mode = %d (0x%x)\n", 
	       ModePageHeaderP->BufferingMode, ModePageHeaderP->BufferingMode);
      fprintf( stderr, "\tSpeed = %d (0x%x)\n", 
	       ModePageHeaderP->Speed, ModePageHeaderP->Speed );
      fprintf( stderr, "\tBlock Descriptor Length = %d (0x%x)\n", 
	       ModePageHeaderP->BlockDescLen, ModePageHeaderP->BlockDescLen );
      if( ModePageHeaderP->isSupportBlockDesc == true )
	{
	  fprintf( stderr, "\tDensity Code = %d (0x%x)\n", 
		   ModePageHeaderP->DensityCode, ModePageHeaderP->DensityCode);
	  fprintf( stderr, "\tTotal Logical Blocks = %u (0x%x)\n", 
		   ModePageHeaderP->NumberOfBlocks, 
		   ModePageHeaderP->NumberOfBlocks );
	  fprintf( stderr, "\t(Reserved) = %d\n", ModePageHeaderP->reserved );
	  fprintf( stderr, "\tBlock Length = %u (0x%x)\n", 
		   ModePageHeaderP->BlockLength, ModePageHeaderP->BlockLength);
	  fprintf( stderr, "\tAdditional Page Data Length = %d\n", 
		   ModePageHeaderP->AdditionalPageDataLen );
	  if( 0 < ModePageHeaderP->AdditionalPageDataLen )
	    {
	      fprintf( stderr, "\tAdditional Page Data : " );
	      for( i = 0 ; i < ModePageHeaderP->AdditionalPageDataLen ; i++ )
		{
		  fprintf( stderr, "%02x ", data[i + 12] );
		}
	      fprintf( stderr, "\n" );
	    }
	}
      
      // compression page
      fprintf( stderr, "\t----------- Compression Page Begin -----------\n" );
      if( cpDataP->isSupportCompressionPage != false )
	{
	  fprintf( stderr, "\tThis device supports data compression\n" );
	  fprintf( stderr, "\tData Compression Page Length = %lld\n", 
		   cpDataP->CompressionPageLen + 1 );
	  fprintf( stderr, "\tData Compression Enable = %s\n", 
		   cpDataP->DataCompressionEnable ? "Yes" : "No" );
	  fprintf( stderr, "\tData Compression Capable = %s\n", 
		   cpDataP->DataCompressionCapable ? "Yes" : "No" );
	  fprintf( stderr, "\tReserved1 = %d\n", cpDataP->reserved1 );
	  fprintf( stderr, "\tData Decompression Enable = %s\n", 
		   cpDataP->DataDecompressionEnable ? "Yes" : "No" );
	  fprintf( stderr, "\tReport Exception on Decompression = %d\n", 
		   cpDataP->ReportExceptionOnDecompression );
	  fprintf( stderr, "\tReserved2 = %d\n", cpDataP->reserved2 );
	  fprintf( stderr, "\tCompression Algorithm = %d\n", 
		   cpDataP->CompressionAlgorithm );
	  fprintf( stderr, "\tDecompression Algorithm = %d\n", 
		   cpDataP->DecompressionAlgorithm );
	  if( 0 < cpDataP->AdditionalDataLen )
	    {
	      fprintf( stderr, "\tAdditional Page Data : " );
	      for( i = 0 ; i < cpDataP->AdditionalDataLen ; i++ )
		{
		  fprintf( stderr, "%02x ", cpDataP->AdditionalData[i] );
		}
	      fprintf( stderr, "\n" );
	    }
	}
      else
	{
	  fprintf(stderr,"\tThis device does not support data compression\n");
	}
      fprintf( stderr, "\t----------- Compression Page End -----------\n" );
    }
  else
    {
      fprintf( stderr, " -> ModeSense Data could not get.\n" );
      fprintf( stderr, "    Maybe, this device does not support ModeSense command.\n" );
      fprintf( stderr, "    Or, meadia does not present...\n" );
    }
  fprintf( stderr, "----------- ModeSense Data End -----------\n" );
  
  return;
}


//////////////////////////////////////////////////////////////////////////
//	block descriptor modifier
//////////////////////////////////////////////////////////////////////////
IOReturn	opcs_setTransferBlockSize( ScsiDevInfoRec *tgtDevInfoP, 
					   int bSize, 
					   SCSITaskStatus *taskStatus )
{
  // set block size in mode select header to 0
  UInt8		*senseDataBuffer;
  UInt64	bufLenByte;
  IOReturn	err = 0;
  
  opcs_debugLog( "set block size in mode select block desc to %d.\n", bSize );
  senseDataBuffer = tgtDevInfoP->ModePage.ModePageHeader.SenseDataBuffer;
  bufLenByte = tgtDevInfoP->ModePage.ModePageHeader.SenseDataBufLen;
  
  if( tgtDevInfoP->ModePage.ModePageHeader.isSupportBlockDesc == true )
    {
      opcs_debugLog( "Support BlockDesc...\n" );
      if( tgtDevInfoP->ModePage.ModePageHeader.BlockLength != bSize )
	{
	  opcs_debugLog( "Device BlockSize Configuration is defferent...\n" );
	  opcs_debugLog( "Re-Get the BlockSize.\n" );
	  err = opcs_modeSense( tgtDevInfoP, 0x00, taskStatus );
	  if( err != KERN_SUCCESS )
	    {
	      return err;
	    }
	  // suppress write disabled data
	  senseDataBuffer[0] = 0;
	  senseDataBuffer[1] = 0;
	  senseDataBuffer[2] &= 0x7f;
	  // set the block size
	  opcs_debugLog( "Set the BlockSize.\n" );
	  senseDataBuffer[9] = (UInt8)(bSize >> 16) & 0xff;
	  senseDataBuffer[10] = (UInt8)(bSize >> 8) & 0xff;
	  senseDataBuffer[11] = (UInt8)(bSize & 0xff);
	  
	  err = opcs_modeSelect( tgtDevInfoP, NonPageFormat, taskStatus );
	  if( err != kIOReturnSuccess )
	    {
	      opcs_debugLog( "failed to opcs_modeSelect : err = %d\n", err );
	      return err;
	    }
	  err = opcs_modeSense( tgtDevInfoP, 0x00, taskStatus );
	}
    }
  return err;
}


IOReturn	opcs_setDensityCode( ScsiDevInfoRec *tgtDevInfoP, 
				     int densityType, 
				     SCSITaskStatus *taskStatus )
{
  // set block size in mode select header to 0
  UInt8		*senseDataBuffer;
  UInt64	bufLenByte;
  UInt8		densityCode;
  IOReturn	err = 0;
  
  opcs_debugLog( "set density code in mode select block desc.\n" );
  senseDataBuffer = tgtDevInfoP->ModePage.ModePageHeader.SenseDataBuffer;
  bufLenByte = tgtDevInfoP->ModePage.ModePageHeader.SenseDataBufLen;
  
  if( tgtDevInfoP->ModePage.ModePageHeader.isSupportBlockDesc == true )
    {
      opcs_debugLog( "Support BlockDesc...\n" );
      //if( tgtDevInfoP->ModePage.ModePageHeader.BlockLength != bSize )
      if( opcs_getDensityCode( tgtDevInfoP, densityType, &densityCode ) 
	  != false )
	{
	  opcs_debugLog( "Found Density Code...\n" );
	  if( tgtDevInfoP->ModePage.ModePageHeader.DensityCode != densityCode )
	    {
	      opcs_debugLog( "Device Density Code is defferent...\n" );
	      opcs_debugLog( "Re-Get the BlockSize.\n" );
	      err = opcs_modeSense( tgtDevInfoP, 0x00, taskStatus );
	      if( err != KERN_SUCCESS )
		{
		  return err;
		}
	      // suppress write disabled data
	      senseDataBuffer[0] = 0;
	      senseDataBuffer[1] = 0;
	      senseDataBuffer[2] &= 0x7f;
	      // set the block size
	      opcs_debugLog( "Set the Density Code.\n" );
	      senseDataBuffer[4] = densityCode;
	      
	      err = opcs_modeSelect( tgtDevInfoP, NonPageFormat, taskStatus );
	      if( err != kIOReturnSuccess )
		{
		  opcs_debugLog( "failed to opcs_modeSelect : " );
		  opcs_debugLog( "err = %d\n", err );
		  return err;
		}
	      err = opcs_modeSense( tgtDevInfoP, 0x00, taskStatus );
	    }
	  else
	    {
	      opcs_debugLog( "Device is already set to request status\n" );
	    }
	}
      else
	{
	  fprintf( stderr, "mtcmd : It does not found the density code " );
	  fprintf( stderr, "for this device : \n        \"%s\"\n", 
		   tgtDevInfoP->DeviceInquiryData.ProductID );
	  fprintf( stderr, "        Density Code request is ignored.\n" );
	}
    }
  return err;
}



//////////////////////////////////////////////////////////////////////////
//	compression page (0x0f) modifier
//////////////////////////////////////////////////////////////////////////
IOReturn	opcs_setCompressionMode( ScsiDevInfoRec *tgtDevInfoP, 
					 Boolean compressMode, 
					 SCSITaskStatus *taskStatus )
{
  UInt8		*senseDataBuffer;
  UInt64	bufLenByte;
  UInt64	pageDataOffset;
  IOReturn	err = 0;
  
  if( tgtDevInfoP->ModePage.CompressionPage.isSupportCompressionPage != false )
    {
      // get compression page data
      opcs_debugLog( "get compression mode page.\n" );
      err = opcs_modeSense( tgtDevInfoP, 0x0f, taskStatus );
      if( err != KERN_SUCCESS )
	{
	  return err;
	}
      if( compressMode 
	  != tgtDevInfoP->ModePage.CompressionPage.DataCompressionEnable )
	{
	  senseDataBuffer 
	    = tgtDevInfoP->ModePage.ModePageHeader.SenseDataBuffer;
	  bufLenByte = kMyModeSenseDataBufLen;
	  pageDataOffset 
	    = 4 + tgtDevInfoP->ModePage.ModePageHeader.BlockDescLen;
	  
	  // suppress write disabled data
	  senseDataBuffer[0] = 0;
	  senseDataBuffer[1] = 0;
	  senseDataBuffer[2] &= 0x7f;
	  if( compressMode == false )
	    {
	      // clear DataCompressionEnable(DCE) bit
	      senseDataBuffer[pageDataOffset + 2] &= 0x7f;
	    }
	  else
	    {
	      // set DataCompressionEnable(DCE) bit
	      senseDataBuffer[pageDataOffset + 2] |= 0x80;
	    }
	  opcs_debugLog( "set compression mode page.\n" );
	  err = opcs_modeSelect( tgtDevInfoP, PageFormat, taskStatus );
	  if( err != kIOReturnSuccess )
	    {
	      opcs_debugLog("failed to opcs_modeSelect : err = %d\n",err);
	      return err;
	    }
	  err = opcs_modeSense( tgtDevInfoP, 0x0f, taskStatus );
	  if( err != KERN_SUCCESS )
	    {
	      return err;
	    }
	}// end of if( compressMode ... )
      else
	{
	  opcs_debugLog( "Device is already set to request status\n" );
	}
    }// end of if( tgtDevInfoP->ModePage.CompressionPage... )
  else
    {
      fprintf( stderr, "mtcmd : This device does not support hardware " );
      fprintf( stderr, "compression.\n" );
      fprintf( stderr, "        Compression request is ignored.\n" );
    }
  return err;
}
