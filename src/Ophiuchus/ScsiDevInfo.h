//	ScsiDevInfo.h
//	Part of Ophiuchus
//	
//	Copyright (c) 2002-2023 tbd128 Allright Reserved.
//	This project is licensed under the MIT License, see the LICENSE.txt file for details.
//	
//	2.0   : Monday, March 11, 2002
//	2.0.1 : Sunday, March 24, 2002
//	2.1   : Monday, May 07, 2002
//


#ifdef	ScsiDevInfo_h__
#else
#define	ScsiDevInfo_h__

#include	<IOKit/scsi/SCSICmds_INQUIRY_Definitions.h>

#define	kDefalutBlocSizeLen	512
#define	kDefalutBlockingFactor	20

#define	kMyMaxSCSIStdInqDataLen		255
#define	kMyMinSCSIStdInqDataLen		36
#define	kMyFirstVendorSpecificLen	20
#define	kMySecondVendorSpecific		96
#define	kMySecondVendorSpecificLen	(kMyMaxSCSIStdInqDataLen - kMySecondVendorSpecific)

#define	kMyModeSenseDataBufLen	255

// Compression Type
typedef	enum
{
  kCompressionModePreserve = 0, 
  kCompressionEnable, 
  kCompressionDisable
}CompressionType;

// Density Type
typedef	enum
{
  kDensityCodePreserve = -1, 
  kDensityCodeDefault = 0,
  kDensityCodeLow,
  kDensityCodeMedium,
  kDensityCodeHight,
  kDensityCodeCompressed,
  kDensityCodeMax
}DensityType;

typedef	struct	DeviceInquiryDataRec	DeviceInquiryDataRec;
typedef	struct	BlockLimitsRec		BlockLimitsRec;
typedef	struct	ModePageRec		ModePageRec;
typedef	struct	ModePageHeaderRec	ModePageHeaderRec;
typedef	struct	CompressionPageRec	CompressionPageRec;
typedef	struct	TransferOption		TransferOption;
typedef	struct	ScsiDevInfoRec		ScsiDevInfoRec;

struct	DeviceInquiryDataRec
{
  UInt64	size;
  UInt8		PeripheralQualifier;
  UInt8		PeripheralDeviceType;
  Boolean	isRemovable;
  UInt8		DeviceTypeModifier;
  UInt8		IsoVersion;
  UInt8		EcmaVersion;
  UInt8		AnsiVersion;
  Boolean	isSupportAenc;
  Boolean	isSupportTerminateIOProcess;
  UInt8		Reserved1;
  UInt8		ResponseDataType;
  int		AdditionalDataLength;
  UInt8		Reserved2;
  UInt8		Reserved3;
  Boolean	isSupportRlativeAddress;
  Boolean	isSupport32bitWideBus;
  Boolean	isSupport16bitWideBus;
  Boolean	isSupportSyncronousTransfer;
  Boolean	isSupportLinkedCommand;
  UInt8		Reserved4;
  Boolean	isSupportTaggedCommadQueing;
  Boolean	isSupportSoftResetCondition;
  UInt8		VendorID[kINQUIRY_VENDOR_IDENTIFICATION_Length + 1];
  UInt8		ProductID[kINQUIRY_PRODUCT_IDENTIFICATION_Length + 1];
  UInt8		ProductVersion[kINQUIRY_PRODUCT_REVISION_LEVEL_Length + 1];
  UInt8		VendorSpecific1[kMyFirstVendorSpecificLen + 1];
  UInt8		VendorSpecific2[kMySecondVendorSpecificLen + 1];
};


struct	BlockLimitsRec
{
  int		MaxBlockLen;
  int		MinBlockLen;
  Boolean	isSupportVariableBlockLength;
};

struct	ModePageHeaderRec
{
  //	raw data
  UInt8		SenseDataBuffer[kMyModeSenseDataBufLen];
  UInt64	SenseDataBufLen;
  //	each base data
  UInt8		ModeParamLen;
  UInt8		MediaType;
  Boolean	isWriteProtected;
  UInt8		BufferingMode;
  UInt8		Speed;
  UInt8		BlockDescLen;
  //	block desciptor
  Boolean	isSupportBlockDesc;
  UInt8		DensityCode;
  UInt32	NumberOfBlocks;
  UInt8		reserved;
  UInt32	BlockLength;
  //	vendor uniq params for non-page mode
  UInt8		AdditionalPageData[kMyModeSenseDataBufLen];
  UInt8		AdditionalPageDataLen;
};

struct	CompressionPageRec
{
  Boolean	isSupportCompressionPage;
  //	hardware info
  UInt64	CompressionPageLen;
  Boolean	DataCompressionEnable;		// DCE
  Boolean	DataCompressionCapable;		// DCC
  UInt8		reserved1;
  Boolean	DataDecompressionEnable;	// DDE
  UInt8		ReportExceptionOnDecompression;	// RED
  UInt8		reserved2;
  UInt32	CompressionAlgorithm;
  UInt32	DecompressionAlgorithm;
  UInt8		AdditionalData[kMyModeSenseDataBufLen];
  UInt8		AdditionalDataLen;
};

struct	ModePageRec
{
  ModePageHeaderRec	ModePageHeader;
  CompressionPageRec	CompressionPage;
};

struct	TransferOption
{
  int	bSize;
  int	bFactor;
  int	compressMode;
  int	densityType;
};

struct	ScsiDevInfoRec
{
  SCSITaskDeviceInterface	**tgtDevInterface;
  SCSITaskInterface		**tgtDevTask;
  DeviceInquiryDataRec		DeviceInquiryData;
  BlockLimitsRec		BlockLimits;
  ModePageRec			ModePage;
  TransferOption		opts;
};

#endif
