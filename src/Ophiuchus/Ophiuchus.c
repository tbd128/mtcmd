//	Ophiuchus.c
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
#include	<errno.h>

#include	<IOKit/IOKitLib.h>
#include	<IOKit/scsi/SCSITaskLib.h>
#include	<IOKit/scsi/SCSICommandOperationCodes.h>

#include	"Ophiuchus.h"
#include	"ScsiDevInfo.h"
#include	"ScsiCmdExec.h"
#include	"Inquiry.h"
#include	"ReadBlockLimits.h"
#include	"Write.h"
#include	"Read.h"
#include	"CheckSenseStat.h"
#include	"ModeSenseSelect.h"
#include	"Rewind.h"
#include	"TestUnitReady.h"
#include	"LoadUnload.h"
#include	"Erase.h"
#include	"Space.h"
#include	"WriteFilemarks.h"
#include	"ReserveRelease.h"
#include	"DebugLog.h"

static	IOReturn	opcsStb_printSummaryInfo( int targetNo );
static	IOReturn	opcsStb_printFullInfo( int targetNo );
static	IOReturn	opcsStb_read( const opcs_OptRec *opts );
static	IOReturn	opcsStb_write( const opcs_OptRec *opts );
static	void		opcsStb_decodeTransferOption(ScsiDevInfoRec *tgtDevInfoP,
						     const opcs_OptRec *opts );
static	IOReturn	opcsStb_setTransferOption( ScsiDevInfoRec *tgtDevInfoP );
static	IOReturn	opcsStb_fsf( const opcs_OptRec *opts );
static	IOReturn	opcsStb_bsf( const opcs_OptRec *opts );
static	IOReturn	opcsStb_weof( const opcs_OptRec *opts );
static	IOReturn	opcsStb_smk( const opcs_OptRec *opts );
static	IOReturn	opcsStb_rewind( const opcs_OptRec *opts );
static	IOReturn	opcsStb_offline( const opcs_OptRec *opts );
static	IOReturn	opcsStb_rewoffl( const opcs_OptRec *opts );
static	IOReturn	opcsStb_erase( const opcs_OptRec *opts );
static	IOReturn	opcsStb_setopt( const opcs_OptRec *opts );


void	opcs_OptRec_setDefault( opcs_OptRec *opts )
{
  opts->targetNo = 0;
  opts->cmd = cmd_notSpecified;
  //opts->bSize = kDefalutBlocSizeLen;
  opts->bSize = 0;
  opts->bFactor = kDefalutBlockingFactor;
  opts->cnt = 0;
  opts->compressMode[0] = '\0';	//kCompressionModePreserve;
  opts->densityType[0] = '\0';	//kDensityCodePreserve;
  opts->inFileName[0] = '\0';
  opts->outFileName[0] = '\0';
  opts->debugLevel = 0;
}

const	static	char	*ver = "Ophiuchus ver. 2.1";
const	static	char	*date = "\tMay 07, 2002";
const	static	char	*vendor = "tbd128";

int	Ophiuchus( const opcs_OptRec *opts )
{
  int	err = 0;
  
  opcs_setDebugLevel( opts->debugLevel );
  
  switch( opts->cmd )
    {
    case cmd_notSpecified:
      err = 1;
      break;
    case cmd_showHelp:	//  0 "help"
      err = 1;
      break;
    case cmd_showVers:	//  1 "vers"
      fprintf( stderr, "\n%s\n%s; %s\n\n", ver, date, vendor );
      break;
    case cmd_info:	//  2 "info"
      err = opcsStb_printSummaryInfo( opts->targetNo );
      break;
    case cmd_fullInfo:	//  3 "fullinfo"
      err = opcsStb_printFullInfo( opts->targetNo );
      break;
    case cmd_read:	//  4 "read"
      err = opcsStb_read( opts );
      break;
    case cmd_write:	//  5 "write"
      err = opcsStb_write( opts );
      break;
    case cmd_fsf:	//  6 "fsf"
      err = opcsStb_fsf( opts );
      break;
    case cmd_bsf:	//  7 "bsf"
      err = opcsStb_bsf( opts );
      break;
    case cmd_weof:	//  8 "weof"
      err = opcsStb_weof( opts );
      break;
    case cmd_smk:	//  9 "smk"
      err = opcsStb_smk( opts );
      break;
    case cmd_rewind:	// 10 "rewind"
      err = opcsStb_rewind( opts );
      break;
    case cmd_offline:	// 11 "offline"
      err = opcsStb_offline( opts );
      break;
    case cmd_rewoffl:	// 12 "rewoffl"
      err = opcsStb_rewoffl( opts );
      break;
    case cmd_erase:	// 13 "erase"
      err = opcsStb_erase( opts );
      break;
    case cmd_setopt:	// 14 "setopt"
      err = opcsStb_setopt( opts );
      break;
    default:
      err = -1;
    }
  return err;
}


//	print follow data for all tape device
//	if targetNo == 0, print all device info
//		o inquiry data : Vendor ID, ProductID, ProductVersion
static	IOReturn	opcsStb_printSummaryInfo( int targetNo )
{
  ScsiDevInfoRec	*tgtDevInfoP = NULL;
  kern_return_t		err = 0;
  int			i = 1;
  
  if( targetNo != 0 )
    {
      err = opcs_createScsiInterface( &tgtDevInfoP, targetNo );
      if( (err == KERN_SUCCESS) && (tgtDevInfoP != NULL) )
	{
	  fprintf( stderr, "mtcmd : target %d : ", targetNo );
	  opcs_printInquiryDataSummary( &(tgtDevInfoP->DeviceInquiryData) );
	}
      else
	{
	  fprintf( stderr, "mtcmd : No such target : %d\n", targetNo );
	}
      opcs_releaseScsiInterface( &tgtDevInfoP );
    }
  else
    {
      while( 1 )
	{
	  err = opcs_createScsiInterface( &tgtDevInfoP, i );
	  if( (err != KERN_SUCCESS) || (tgtDevInfoP == NULL) )
	    {
	      break;
	    }
	  fprintf( stderr, "mtcmd : target %d : ", i );
	  opcs_printInquiryDataSummary( &(tgtDevInfoP->DeviceInquiryData) );
	  opcs_releaseScsiInterface( &tgtDevInfoP );
	  i++;
	}
    }
  
  return 0;
}


//	print follow data for tape device
//	if targetNo == 0, print all device info
//		o full inquiry data
//		o block limits
//		o mode sense data (page 0)
static	IOReturn	opcsStb_printFullInfo( int targetNo )
{
  ScsiDevInfoRec	*tgtDevInfoP = NULL;
  kern_return_t		err = 0;
  int			i = 1;
  
  if( targetNo != 0 )
    {
      err = opcs_createScsiInterface( &tgtDevInfoP, targetNo );
      if( (err == KERN_SUCCESS) && (tgtDevInfoP != NULL) )
	{
	  fprintf( stderr, "========================= " );
	  fprintf( stderr, "SCSI target %d ", i );
	  fprintf( stderr, "=========================\n" );
	  opcs_printInquiryData( &(tgtDevInfoP->DeviceInquiryData) );
	  opcs_printBlockLimits( &(tgtDevInfoP->BlockLimits) );
	  opcs_printModeSense( tgtDevInfoP );
	}
      else
	{
	  fprintf( stderr, "mtcmd : No such target : %d\n", targetNo );
	}
      opcs_releaseScsiInterface( &tgtDevInfoP );
    }
  else
    {
      while( 1 )
	{
	  err = opcs_createScsiInterface( &tgtDevInfoP, i );
	  if( (err != KERN_SUCCESS) || (tgtDevInfoP == NULL) )
	    {
	      break;
	    }
	  fprintf( stderr, "========================= " );
	  fprintf( stderr, "SCSI target %d ", i );
	  fprintf( stderr, "=========================\n" );
	  opcs_printInquiryData( &(tgtDevInfoP->DeviceInquiryData) );
	  opcs_printBlockLimits( &(tgtDevInfoP->BlockLimits) );
	  opcs_printModeSense( tgtDevInfoP );
	  opcs_releaseScsiInterface( &tgtDevInfoP );
	  i++;
	}
    }
  return 0;
}


static	IOReturn	opcsStb_read( const opcs_OptRec *opts )
{
  ScsiDevInfoRec	*tgtDevInfoP = NULL;
  kern_return_t		err = 0;
  SCSITaskStatus	taskStatus;
  FILE			*file = 0;
  
  err = opcs_createScsiInterface( &tgtDevInfoP, opts->targetNo );
  if( err != KERN_SUCCESS )
    {
      return err;
    }
  else if( tgtDevInfoP == NULL )
    {
      fprintf( stderr, "mtcmd : failed to open target device : %d\n", 
	       opts->targetNo );
      err = -1;
    }
  else
    {
      // if output file was specified, open the file
      if( 0 < strlen( opts->outFileName ) )
	{
	  // open output file
	  errno = 0;
	  file = fopen( opts->outFileName, "w" );
	  err = errno;
	  if( errno )
	    {
	      perror( opts->outFileName );
	      opcs_debugLog( "the file could not open : %s\n", 
			     opts->outFileName );
	    }
	}
      else
	{
	  // output file name was not specified.
	  file = stdout;
	}
      if( err == 0 )
	{
	  tgtDevInfoP->opts.bSize = opts->bSize;
	  tgtDevInfoP->opts.bFactor = opts->bFactor;
	  tgtDevInfoP->opts.compressMode = kCompressionModePreserve;
	  tgtDevInfoP->opts.densityType = kDensityCodePreserve;
	  err = opcsStb_setTransferOption( tgtDevInfoP );
	  if( err == 0 )
	    {
	      // wait for idle
	      (void)opcs_waitUntilDeviceIdle( tgtDevInfoP, &taskStatus );
	      err = opcs_read( tgtDevInfoP, file, &taskStatus );
	      if( err != kIOReturnSuccess )
		{
		  fprintf( stderr, "mtcmd : opcs_read failed : err = 0x%x\n", 
			   err );
		}
	    }
	}
    }
  opcs_releaseScsiInterface( &tgtDevInfoP );
  return err;
}


static	IOReturn	opcsStb_write( const opcs_OptRec *opts )
{
  ScsiDevInfoRec	*tgtDevInfoP = NULL;
  kern_return_t		err = 0;
  SCSITaskStatus	taskStatus;
  FILE			*file;
  
  err = opcs_createScsiInterface( &tgtDevInfoP, opts->targetNo );
  if( err != KERN_SUCCESS )
    {
      return err;
    }
  else if( tgtDevInfoP == NULL )
    {
      fprintf( stderr, "mtcmd : failed to open target device : %d\n", 
	       opts->targetNo );
      err = -1;
    }
  else
    {
      // if input file was specified, open the file
      if( 0 < strlen( opts->inFileName ) )
	{
	  // open input file
	  errno = 0;
	  file = fopen( opts->inFileName, "r" );
	  err = errno;
	  if( errno )
	    {
	      perror( opts->inFileName );
	      opcs_debugLog( "the file could not open : %s\n", 
			     opts->inFileName );
	    }
	}
      else
	{
	  // input file name was not specified.
	  file = stdin;
	}
      if( err == 0 )
	{
	  opcsStb_decodeTransferOption( tgtDevInfoP, opts );
	  err = opcsStb_setTransferOption( tgtDevInfoP );
	  if( err == 0 )
	    {
	      // wait for idle
	      (void)opcs_waitUntilDeviceIdle( tgtDevInfoP, &taskStatus );
	      err = opcs_write( tgtDevInfoP, file, &taskStatus );
	      if( err != kIOReturnSuccess )
		{
		  fprintf( stderr, "mtcmd : opcs_write failed : err = 0x%x\n",
			   err );
		}
	    }
	}
    }
  opcs_releaseScsiInterface( &tgtDevInfoP );
  return err;
}


static	IOReturn	opcsStb_fsf( const opcs_OptRec *opts )
{
  ScsiDevInfoRec	*tgtDevInfoP = NULL;
  kern_return_t		err = 0;
  SCSITaskStatus	taskStatus;
  
  err = opcs_createScsiInterface( &tgtDevInfoP, opts->targetNo );
  if( err != KERN_SUCCESS )
    {
      return err;
    }
  else if( tgtDevInfoP == NULL )
    {
      fprintf( stderr, "failed to open target device : %d\n", opts->targetNo );
      err = -1;
    }
  else
    {
      err = opcs_fwdSpace( tgtDevInfoP, opts->cnt, &taskStatus );
    }
  opcs_releaseScsiInterface( &tgtDevInfoP );
  
  return err;
}


static	IOReturn	opcsStb_bsf( const opcs_OptRec *opts )
{
  ScsiDevInfoRec	*tgtDevInfoP = NULL;
  kern_return_t		err = 0;
  SCSITaskStatus	taskStatus;
  
  err = opcs_createScsiInterface( &tgtDevInfoP, opts->targetNo );
  if( err != KERN_SUCCESS )
    {
      return err;
    }
  else if( tgtDevInfoP == NULL )
    {
      fprintf( stderr, "failed to open target device : %d\n", opts->targetNo );
      err = -1;
    }
  else
    {
      err = opcs_bkwdSpace( tgtDevInfoP, opts->cnt, &taskStatus );
    }
  opcs_releaseScsiInterface( &tgtDevInfoP );
  
  return err;
}


static	IOReturn	opcsStb_weof( const opcs_OptRec *opts )
{
  ScsiDevInfoRec	*tgtDevInfoP = NULL;
  kern_return_t		err = 0;
  SCSITaskStatus	taskStatus;
  
  err = opcs_createScsiInterface( &tgtDevInfoP, opts->targetNo );
  if( err != KERN_SUCCESS )
    {
      return err;
    }
  else if( tgtDevInfoP == NULL )
    {
      fprintf( stderr, "failed to open target device : %d\n", opts->targetNo );
      err = -1;
    }
  else
    {
      err = opcs_writeFilemarks( tgtDevInfoP, opts->cnt, &taskStatus );
    }
  opcs_releaseScsiInterface( &tgtDevInfoP );
  
  return err;
}


static	IOReturn	opcsStb_smk( const opcs_OptRec *opts )
{
  /*
  ScsiDevInfoRec	*tgtDevInfoP = NULL;
  kern_return_t		err = 0;
  SCSITaskStatus	taskStatus;
  FILE			*file;
  
  err = opcs_createScsiInterface( &tgtDevInfoP, opts->targetNo );
  if( err != KERN_SUCCESS )
    {
      return err;
    }
  else if( tgtDevInfoP == NULL )
    {
      fprintf( stderr, "failed to open target device : %d\n", opts->targetNo );
      err = -1;
    }
  else
    {
      err = MySpace( tgtDevInfoP, opts->cnt, &taskStatus );
    }
  opcs_releaseScsiInterface( &tgtDevInfoP );
  
  return err;
  */
  return kIOReturnSuccess;
}


static	IOReturn	opcsStb_rewind( const opcs_OptRec *opts )
{
  ScsiDevInfoRec	*tgtDevInfoP = NULL;
  kern_return_t		err = 0;
  SCSITaskStatus	taskStatus;
  
  err = opcs_createScsiInterface( &tgtDevInfoP, opts->targetNo );
  if( err != KERN_SUCCESS )
    {
      return err;
    }
  else if( tgtDevInfoP == NULL )
    {
      fprintf( stderr, "failed to open target device : %d\n", opts->targetNo );
      err = -1;
    }
  else
    {
      err = opcs_rewind( tgtDevInfoP, &taskStatus );
    }
  opcs_releaseScsiInterface( &tgtDevInfoP );
  
  return err;
}


static	IOReturn	opcsStb_offline( const opcs_OptRec *opts )
{
  ScsiDevInfoRec	*tgtDevInfoP = NULL;
  kern_return_t		err = 0;
  SCSITaskStatus	taskStatus;
  
  err = opcs_createScsiInterface( &tgtDevInfoP, opts->targetNo );
  if( err != KERN_SUCCESS )
    {
      return err;
    }
  else if( tgtDevInfoP == NULL )
    {
      fprintf( stderr, "failed to open target device : %d\n", opts->targetNo );
      err = -1;
    }
  else
    {
      err = opcs_loadUnload( tgtDevInfoP, kUnloadTape, &taskStatus );
    }
  opcs_releaseScsiInterface( &tgtDevInfoP );
  
  return err;
}


static	IOReturn	opcsStb_rewoffl( const opcs_OptRec *opts )
{
  ScsiDevInfoRec	*tgtDevInfoP = NULL;
  kern_return_t		err = 0;
  SCSITaskStatus	taskStatus;
  
  err = opcs_createScsiInterface( &tgtDevInfoP, opts->targetNo );
  if( err != KERN_SUCCESS )
    {
      return err;
    }
  else if( tgtDevInfoP == NULL )
    {
      fprintf( stderr, "failed to open target device : %d\n", opts->targetNo );
      err = -1;
    }
  else
    {
      err = opcs_rewind( tgtDevInfoP, &taskStatus );
      if( err == kIOReturnSuccess )
	{
	  err = opcs_loadUnload( tgtDevInfoP, kUnloadTape, &taskStatus );
	}
    }
  opcs_releaseScsiInterface( &tgtDevInfoP );
  
  return err;
}


static	IOReturn	opcsStb_erase( const opcs_OptRec *opts )
{
  ScsiDevInfoRec	*tgtDevInfoP = NULL;
  kern_return_t		err = 0;
  SCSITaskStatus	taskStatus;
  
  err = opcs_createScsiInterface( &tgtDevInfoP, opts->targetNo );
  if( err != KERN_SUCCESS )
    {
      return err;
    }
  else if( tgtDevInfoP == NULL )
    {
      fprintf( stderr, "failed to open target device : %d\n", opts->targetNo );
      err = -1;
    }
  else
    {
      err = opcs_erase( tgtDevInfoP, &taskStatus );
      if( err == kIOReturnSuccess )
	{
	  err = opcs_loadUnload( tgtDevInfoP, kUnloadTape, &taskStatus );
	}
    }
  opcs_releaseScsiInterface( &tgtDevInfoP );
  
  return err;
}


static	IOReturn	opcsStb_setopt( const opcs_OptRec *opts )
{
  ScsiDevInfoRec	*tgtDevInfoP = NULL;
  kern_return_t		err = 0;
  //SCSITaskStatus	taskStatus;
  
  err = opcs_createScsiInterface( &tgtDevInfoP, opts->targetNo );
  if( err != KERN_SUCCESS )
    {
      return err;
    }
  else if( tgtDevInfoP == NULL )
    {
      fprintf( stderr, "failed to open target device : %d\n", opts->targetNo );
      err = -1;
    }
  else
    {
      opcsStb_decodeTransferOption( tgtDevInfoP, opts );
      err = opcsStb_setTransferOption( tgtDevInfoP );
    }
  opcs_releaseScsiInterface( &tgtDevInfoP );
  
  return err;
}


static	void	opcsStb_decodeTransferOption( ScsiDevInfoRec *tgtDevInfoP,
					      const opcs_OptRec *opts )
{
  tgtDevInfoP->opts.bSize = opts->bSize;
  tgtDevInfoP->opts.bFactor = opts->bFactor;
  // Compression Mode
  if( strcmp( "y", opts->compressMode ) == 0 )
    {
      tgtDevInfoP->opts.compressMode = kCompressionEnable;
    }
  else if( strcmp( "n", opts->compressMode ) == 0 )
    {
      tgtDevInfoP->opts.compressMode = kCompressionDisable;
    }
  else
    {
      tgtDevInfoP->opts.compressMode = kCompressionModePreserve;
    }
  // Density Type
  if( strcmp( "l", opts->densityType ) == 0 )
    {
      tgtDevInfoP->opts.densityType = kDensityCodeLow;
    }
  else if( strcmp( "m", opts->densityType ) == 0 )
    {
      tgtDevInfoP->opts.densityType = kDensityCodeMedium;
    }
  else if( strcmp( "h", opts->densityType ) == 0 )
    {
      tgtDevInfoP->opts.densityType = kDensityCodeHight;
    }
  else if( strcmp( "c", opts->densityType ) == 0 )
    {
      tgtDevInfoP->opts.densityType = kDensityCodeCompressed;
    }
  else
    {
      tgtDevInfoP->opts.densityType = kDensityCodePreserve;
    }
}

static	IOReturn	opcsStb_setTransferOption( ScsiDevInfoRec *tgtDevInfoP )
{
  IOReturn		err = kIOReturnSuccess;
  SCSITaskStatus	taskStatus;
  
  // Block Length -----------------------------------------------------
  if( tgtDevInfoP->opts.bSize == 0 )
    {
      // default operation
      if( tgtDevInfoP->BlockLimits.isSupportVariableBlockLength == false )
	{
	  // target device supports fixed block length only
	  tgtDevInfoP->opts.bSize = tgtDevInfoP->BlockLimits.MaxBlockLen;
	}
      else
	{
	  // target device supports variable block length
	  // set the BlockLength in the mode page header to 0
	  opcs_setTransferBlockSize( tgtDevInfoP, 0, &taskStatus );
	  tgtDevInfoP->opts.bSize = kDefalutBlocSizeLen;
	}
    }
  else
    {
      // force fixed block legth mode
      if( tgtDevInfoP->BlockLimits.isSupportVariableBlockLength == false )
	{
	  // target device supports fixed block length only
	  fprintf( stderr, "mtcmd : " );
	  fprintf( stderr, "This device supports fixed block length only.\n" );
	  fprintf( stderr, "        " );
	  fprintf( stderr, "-bs option is discarded.\n" );
	  tgtDevInfoP->opts.bSize = tgtDevInfoP->BlockLimits.MaxBlockLen;
	}
      else
	{
	  // check the specified block length is valid or not
	  if((tgtDevInfoP->opts.bSize < tgtDevInfoP->BlockLimits.MinBlockLen) 
	     ||(tgtDevInfoP->BlockLimits.MaxBlockLen<tgtDevInfoP->opts.bSize))
	    {
	      fprintf( stderr,
		       "Block size must be %d <= n <= %d for this device\n", 
		       tgtDevInfoP->BlockLimits.MinBlockLen, 
		       tgtDevInfoP->BlockLimits.MaxBlockLen );
	      return -1;
	    }
	  // target device supports variable block length
	  // set the BlockLength in the mode page header to 0
	  err = opcs_setTransferBlockSize( tgtDevInfoP, 
					   tgtDevInfoP->opts.bSize, 
					   &taskStatus );
	  tgtDevInfoP->opts.bSize = tgtDevInfoP->opts.bSize;
	  tgtDevInfoP->BlockLimits.isSupportVariableBlockLength = false;
	}
    }
  
  // Blocking Factor -----------------------------------------------------
  if( tgtDevInfoP->opts.bFactor == 0 )
    {
      tgtDevInfoP->opts.bFactor = kDefalutBlockingFactor;
    }
  else if( tgtDevInfoP->opts.bFactor < 0 )
    {
      fprintf( stderr, "Blocking factor must be 1 <= n.\n" );
      return -1;
    }
  
  // Density Code -----------------------------------------------------
  if( tgtDevInfoP->opts.densityType != kDensityCodePreserve )
    {
      err = opcs_setDensityCode( tgtDevInfoP, tgtDevInfoP->opts.densityType, 
				 &taskStatus );
      if( err != KERN_SUCCESS )
	{
	  return err;
	}
    }
  
  // Compression Type -----------------------------------------------------
  if( tgtDevInfoP->opts.compressMode != kCompressionModePreserve )
    {
      Boolean	compressMode;
      if( tgtDevInfoP->opts.compressMode == kCompressionEnable )
	{
	  compressMode = true;
	}
      else
	{
	  compressMode = false;
	}
      err = opcs_setCompressionMode( tgtDevInfoP, compressMode, &taskStatus );
      if( err != KERN_SUCCESS )
	{
	  return err;
	}
    }
  
  return err;
}


