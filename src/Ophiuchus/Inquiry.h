//	Inquiry.h
//	Part of Ophiuchus
//	
//	Copyright (c) 2002-2023 tbd128 Allright Reserved.
//	This project is licensed under the MIT License, see the LICENSE.txt file for details.
//	
//	Monday, March 11, 2002
//

#ifdef	Inquiry_h__
#else
#define	Inquiry_h__

IOReturn	opcs_inquiry( ScsiDevInfoRec *tgtDevInfoP, 
			      SCSITaskStatus *taskStatus );
void	opcs_printInquiryDataSummary(DeviceInquiryDataRec *DeviceInquiryData);
void	opcs_printInquiryData( DeviceInquiryDataRec *DeviceInquiryDataP );

#endif
