//	DebugLog.c
//	Part of Ophiuchus
//	
//	Copyright (c) 2002-2023 tbd128 Allright Reserved.
//	This project is licensed under the MIT License, see the LICENSE.txt file for details.
//	
//	2.0   : Monday, March 11, 2002
//	2.0.1 : Sunday, March 24, 2002
//

#include	<stdio.h>
#include	<stdarg.h>
#include	"DebugLog.h"

int	sg_debugLevel;


void	opcs_setDebugLevel( int debugLevel )
{
  sg_debugLevel = debugLevel;
}

int	opcs_getDebugLevel( void )
{
  return sg_debugLevel;
}

void	opcs_debugLog( const char *fmt, ... )
{
  va_list	ap;
  
  va_start( ap, fmt );
  if( 0 < sg_debugLevel )
    {
      vfprintf( stderr, fmt, ap );
    }
  va_end( ap );
}

