//	DensityCode.c
//	Part of Ophiuchus
//	
//	Copyright (c) 2002-2023 tbd128 Allright Reserved.
//	This project is licensed under the MIT License, see the LICENSE.txt file for details.
//	
//	2.1   : Monday, May 07, 2002
//

#include	<stdio.h>
#include	<strings.h>

#include	<IOKit/IOKitLib.h>
#include	<IOKit/scsi/SCSITaskLib.h>
#include	<IOKit/scsi/SCSICommandOperationCodes.h>

#include	"ScsiDevInfo.h"
#include	"DebugLog.h"
#include	"DensityCode.h"

typedef	struct	DensityCodeRec	DensityCodeRec;
struct	DensityCodeRec
{
  UInt8	ProductID[kINQUIRY_PRODUCT_IDENTIFICATION_Length + 1];
  UInt8	DensityCode[kDensityCodeMax];
};

DensityCodeRec	DensityCodeTab[] = 
{
  //{ "ProductID" }		{def., low,  med., hi.,  comp}
  {
    { "EXB-8505" },		{0x00, 0x14, 0x15, 0x8c, 0x8c}
  },
  {
    { "" },			{0x00, 0x00, 0x00, 0x00, 0x00 }
  }
};

Boolean	opcs_getDensityCode( ScsiDevInfoRec *tgtDevInfoP, 
			     int densityType, UInt8 *densityCode )
{
  UInt8		*tgtProductID;
  int		cmpLen;
  int		i;
  Boolean	found = false;
  
  if( (densityType < kDensityCodeDefault)
      || (kDensityCodeMax <= densityType) )
    {
      opcs_debugLog( "densityType is out of range : %d\n", densityType );
      return false;
    }
  
  tgtProductID = tgtDevInfoP->DeviceInquiryData.ProductID;
  
  for( i = 0 ; DensityCodeTab[i].ProductID[0] != '\0' ; i++ )
    {
      cmpLen = strlen( (char *)(DensityCodeTab[i].ProductID) );
      if( strncmp( (char *)(DensityCodeTab[i].ProductID), 
		(char *)tgtProductID, cmpLen ) == 0 )
	{
	  opcs_debugLog( "ProductID is matched : %s\n", 
			 DensityCodeTab[i].ProductID );
	  *densityCode = DensityCodeTab[i].DensityCode[densityType];
	  opcs_debugLog( "*densityCode = %d\n", *densityCode );
	  found = true;
	  break;
	}
    }
  return found;
}
