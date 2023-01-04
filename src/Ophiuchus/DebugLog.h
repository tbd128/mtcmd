//	DebugLog.h
//	Part of Ophiuchus
//	
//	Copyright (c) 2002-2023 tbd128 Allright Reserved.
//	This project is licensed under the MIT License, see the LICENSE.txt file for details.
//	
//	2.0   : Monday, March 11, 2002
//	2.0.1 : Sunday, March 24, 2002
//

//	DebugLog.h

#ifdef	DebugLog_h__
#else
#define	DebugLog_h__

void	opcs_setDebugLevel( int debugLevel );
int	opcs_getDebugLevel( void );
void	opcs_debugLog( const char *fmt, ... );

#endif	// DebugLog_h__
