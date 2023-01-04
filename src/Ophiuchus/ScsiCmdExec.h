//	ScsiCmdExec.h
//	Part of Ophiuchus
//	
//	Copyright (c) 2002-2023 tbd128 Allright Reserved.
//	This project is licensed under the MIT License, see the LICENSE.txt file for details.
//	
//	Monday, March 11, 2002
//

#ifdef	ScsiCmdExec_h__
#else
#define	ScsiCmdExec_h__

/* for execute SCSI command */

typedef	struct	SCSICmdParam	SCSICmdParam;
struct	SCSICmdParam
{
  //SCSITaskDeviceInterface	**interface;
  SCSITaskInterface		**task;
  SCSITaskStatus		taskStatus;
  SCSI_Sense_Data		senseData;
  SCSICommandDescriptorBlock	cdb;
  UInt8				cdbSize;
  UInt8				*InitiatorToTgtBufPtr;
  UInt64			InitiatorToTgtBufLen;
  UInt8				*TgtToInitiatorBufPtr;
  UInt64			TgtToInitiatorBufLen;
  UInt64			transferCount;
};

kern_return_t	opcs_createScsiInterface(ScsiDevInfoRec **tgtDevInfoP,int cnt);
void		opcs_releaseScsiInterface( ScsiDevInfoRec **tgtDevInfoListP );
IOReturn	opcs_execScsiCmd( SCSICmdParam *param );

#endif
