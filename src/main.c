//	main.c
//	An interface to Ophiuchus
//	
//	Copyright (c) 2002-2023 tbd128 Allright Reserved.
//	This project is licensed under the MIT License, see the LICENSE.txt file for details.
//	
//	1.0   : Monday, March 11, 2002
//	1.0.1 : Sunday, March 24, 2002
//	1.1   : Monday, May 07, 2002
//	1.1.1 : Saturday, Jun 15, 2002
//	1.1.3 : Tuesday, Jan 03, 2023
//

#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	<errno.h>
#include	"Ophiuchus/Ophiuchus.h"

const	static	char	*ver = "mtcmd ver. 1.1.1";
const	static	char	*date = "\tJun 15, 2002";
const	static	char	*vendor = "Albireo Software";
const	static	char	*helpMsg = 
"usage: mtcmd [option...] [cmd]\n"
" -h	display this message\n"
" -V	display version number\n"
" -i	display target device number. use this number for -t option argument\n"
" -I	display inquiry data, block limits, modepage\n"
" -t n	set the target device number to n\n"
" -bs n	set block size(length) to n (byte) -- default : 512byte\n"
" -bf n	set blocking factor to n -- default : 20\n"
" -if f	specify input file name as f. for write mode only\n"
" -of f	specify output file name as f. for read mode only\n"
" -c  m	set harware compress mode. m=y|n ; y=enable, n=disable\n"
" -dt t	set the density type. t=l|m|h|c\n"
" cmd	specify a command one of follows...\n"
"		read	read data from tape\n"
"		write	write data to tape\n"
"		fsf [n]	forward space n files. default : n = 1\n"
"		bsf [n]	backward space n files. default : n = 1\n"
"		weof [n] write n filemarks. default : n = 1\n"
"		smk [n]	write n setmarks (currently not implemented)\n"
"		rewind	rewind the tape\n"
"		offline	rewind the tape and place the tape unit off-line\n"
"		rewoffl	rewind the tape and place the tape unit off-line\n"
"		erase	erase the tape\n"
"		setopt	set block size and compression mode to device.\n";

char	*optList[] = 
{
  "-h", 
  "-V", 
  "-i", 
  "-I", 
  "-t", 
  "-bs", 
  "-bf", 
  "-if", 
  "-of", 
  "-c", 
  "-dt", 
  "-D", 
  NULL
};

char	*cmdList[] = 
{
  "help", 	//  0 cmd_showHelp
  "vers", 	//  1 cmd_showVers
  "info", 	//  2 cmd_info
  "fullinfo", 	//  3 cmd_fullInfo
  "read", 	//  4 cmd_read
  "write", 	//  5 cmd_write
  "fsf", 	//  6 cmd_fsf
  "bsf", 	//  7 cmd_bsf
  "weof", 	//  8 cmd_weof
  "smk", 	//  9 cmd_smk
  "rewind", 	// 10 cmd_rewind
  "offline", 	// 11 cmd_offline
  "rewoffl", 	// 12 cmd_rewoffl
  "erase", 	// 13 cmd_erase
  "setopt", 	// 14 cmd_erase
  NULL		
};


static	int	getCmdAndOption( int argc, char *argv[], opcs_OptRec *opts );
static	int	getOptionId( char *opt, char *tab[] );
static	void	opcs_OptRec_debugDump( opcs_OptRec *opts );


int	main( int argc, char *argv[] )
{
  opcs_OptRec	opts;
  int		err = 0;
  
  opcs_OptRec_setDefault( &opts );
  
  err = getCmdAndOption( argc, argv, &opts );
  if( 0 < opts.debugLevel )
    {
      opcs_OptRec_debugDump( &opts );
    }
  if( opts.cmd == cmd_showHelp )
    {
      fprintf( stderr, "%s", helpMsg );
    }
  
  if( err == 0 )
    {
      err = Ophiuchus( &opts );
      if( err == 1 )
	{
	  fprintf( stderr, "%s", helpMsg );
	}
    }
  
  return (err == 0) ? 0 : 1;
}


static	int	getCmdAndOption( int argc, char *argv[], opcs_OptRec *opts )
{
  int	cmdId;
  int	i;
  
  opts->cmd = cmd_notSpecified;
  
  for( i = 1 ; i < argc ; i++ )
    {
      switch( getOptionId( argv[i], optList ) ) 
	{
	case 0: // "-h"
	  opts->cmd = cmd_showHelp;
	  return 1;
	  break;
	case 1: // "-V"
	  opts->cmd = cmd_showVers;
	  fprintf( stderr, "%s\n%s; %s\n", ver, date, vendor );
	  break;
	case 2: // "-i"
	  opts->cmd = cmd_info;
	  break;
	case 3: // "-I"
	  opts->cmd = cmd_fullInfo;
	  break;
	case 4: // "-t"
	  if( (i + 1) < argc )
	    {
	      i++;
	      opts->targetNo = atoi( argv[i] );
	    }
	  else
	    {
	      fprintf(stderr, "mtcmd : need target number after -t option\n");
	      return -1;
	    }
	  break;
	case 5: // "-bs"
	  if( (i + 1) < argc )
	    {
	      opts->bSize = atoi( argv[i + 1] );
	      if( opts->bSize == 0 )
		{
		  if( strcmp( argv[i + 1], "0" ) != 0 )
		    {
		      fprintf( stderr, 
			       "mtcmd : need block size after -bs option\n" );
		      return -1;
		    }
		  else
		    {
		      i++;
		    }
		}
	      else
		{
		  i++;
		}
	    }
	  else
	    {
	      fprintf( stderr, "mtcmd : need block size after -bs option\n" );
	      return -1;
	    }
	  break;
	case 6: // "-bf"
	  if( (i + 1) < argc )
	    {
	      i++;
	      opts->bFactor = atoi( argv[i] );
	    }
	  else
	    {
	      fprintf( stderr, "mtcmd : need block count after -bf option\n" );
	      return -1;
	    }
	  break;
	case 7: // "-if"
	  if( (i + 1) < argc )
	    {
	      i++;
	      if( strlen( argv[i] ) < (FILENAME_MAX - 1) )
		{
		  strcpy( opts->inFileName, argv[i] );
		}
	      else
		{
		  fprintf( stderr, "-if : file name is too long.\n" );
		  return -1;
		}
	    }
	  else
	    {
	      fprintf( stderr, 
		       "mtcmd : need input file name after -if option\n" );
	      return -1;
	    }
	  break;
	case 8: // "-of"
	  if( (i + 1) < argc )
	    {
	      i++;
	      if( strlen( argv[i] ) < (FILENAME_MAX - 1) )
		{
		  strcpy( opts->outFileName, argv[i] );
		}
	      else
		{
		  fprintf( stderr, "-of : file name is too long.\n" );
		  return -1;
		}
	    }
	  else
	    {
	      fprintf( stderr, 
		       "mtcmd : need output file name after -if option\n" );
	      return -1;
	    }
	  break;
	case 9: // "-c"
	  if( (i + 1) < argc )
	    {
	      i++;
	      if( (strcmp("y", argv[i]) == 0) || (strcmp("n", argv[i]) == 0) )
		{
		  strcpy( opts->compressMode, argv[i] );
		}
	      else
		{
		  fprintf( stderr, "mtcmd : need \"y\" or \"n\" " );
		  fprintf( stderr, "after -c option\n" );
		  return -1;
		}
	    }
	  else
	    {
	      fprintf( stderr, "mtcmd : need \"y\" or \"n\" " );
	      fprintf( stderr, "after -c option\n" );
	      return -1;
	    }
	  break;
	case 10: // "-dt"
	  if( (i + 1) < argc )
	    {
	      i++;
	      if( (strcmp( "l", argv[i] ) == 0) 
		  || (strcmp( "m", argv[i] ) == 0)
		  || (strcmp( "h", argv[i] ) == 0) 
		  || (strcmp( "c", argv[i] ) == 0) )
		{
		  strcpy( opts->densityType, argv[i] );
		}
	      else
		{
		  fprintf( stderr, "mtcmd : need \"l\", \"m\", \"h\", " );
		  fprintf( stderr, "\"c\" after -dt option\n" );
		  return -1;
		}
	    }
	  else
	    {
	      fprintf( stderr, "mtcmd : need \"y\" or \"n\" " );
	      fprintf( stderr, "after -c option\n" );
	      return -1;
	    }
	  break;
	  
	case 11: // "-D"
	  if( (i + 1) < argc )
	    {
	      i++;
	      opts->debugLevel = atoi( argv[i] );
	    }
	  else
	    {
	      fprintf( stderr, "mtcmd : need debug level after -D option\n" );
	      return -1;
	    }
	  break;
	default:
	  cmdId = getOptionId( argv[i], cmdList );
	  if( cmdId != -1 )
	    {
	      opts->cmd = cmdId;
	      switch( cmdId )
		{
		case cmd_showHelp: // "help"
		  break;
		case cmd_showVers: // "vers"
		  break;
		case cmd_info: // "info"
		  break;
		case cmd_fullInfo: // "fullinfo"
		  break;
		case cmd_read: // "read"
		  break;
		case cmd_write: // "write"
		  break;
		case cmd_fsf: // "fsf"
		  if( (i + 1) < argc )
		    {
		      opts->cnt = atoi( argv[i + 1] );
		      if( opts->cnt != 0 )
			{
			  i++;
			}
		      else
			{
			  opts->cnt = 1;
			}
		    }
		  else
		    {
		      opts->cnt = 1;
		    }
		  break;
		case cmd_bsf: // "bsf"
		  if( (i + 1) < argc )
		    {
		      opts->cnt = atoi( argv[i + 1] );
		      if( opts->cnt != 0 )
			{
			  i++;
			}
		      else
			{
			  opts->cnt = 1;
			}
		    }
		  else
		    {
		      opts->cnt = 1;
		    }
		  break;
		case cmd_weof: // "weof"
		  if( (i + 1) < argc )
		    {
		      opts->cnt = atoi( argv[i + 1] );
		      if( opts->cnt != 0 )
			{
			  i++;
			}
		      else
			{
			  opts->cnt = 1;
			}
		    }
		  else
		    {
		      opts->cnt = 1;
		    }
		  break;
		case cmd_smk: // "smk"
		  if( (i + 1) < argc )
		    {
		      opts->cnt = atoi( argv[i + 1] );
		      if( opts->cnt != 0 )
			{
			  i++;
			}
		      else
			{
			  opts->cnt = 1;
			}
		    }
		  else
		    {
		      opts->cnt = 1;
		    }
		  break;
		case cmd_rewind: // "rewind"
		  break;
		case cmd_offline: // "offline"
		  break;
		case cmd_rewoffl: // "rewoffl"
		  break;
		case cmd_erase: // "erase"
		  break;
		case cmd_setopt: // "setopt"
		  break;
		default:
		  break;
		}
	    }
	  else
	    {
	      fprintf( stderr, "illegal option -- %s\n", argv[i] );
	      opts->cmd = cmd_showHelp;
	      return -1;
	    }
	  break;
	}
    }
  return 0;
}


static	int	getOptionId( char *opt, char *tab[] )
{
  int	i;
  int	foundId = -1;
  
  for( i = 0 ; tab[i] != NULL ; i++ )
    {
      if( strcmp( tab[i], opt ) == 0 )
	{
	  foundId = i;
	  break;
	}
    }
  return foundId;
}


static	void	opcs_OptRec_debugDump( opcs_OptRec *opts )
{
  fprintf( stderr, "---------- opcs_OptRec debug dump begin ----------\n" );
  fprintf( stderr, "targetNo = %d\n", opts->targetNo );
  fprintf( stderr, "cmd = %d\n", opts->cmd );
  fprintf( stderr, "bSize = %d\n", opts->bSize );
  fprintf( stderr, "bFactor = %d\n", opts->bFactor );
  fprintf( stderr, "cnt = %d\n", opts->cnt );
  fprintf( stderr, "compressMode = \"%s\"\n", opts->compressMode );
  fprintf( stderr, "densityType = \"%s\"\n", opts->densityType );
  fprintf( stderr, "inFileName = \"%s\"\n", opts->inFileName );
  fprintf( stderr, "outFileName = \"%s\"\n", opts->outFileName );
  fprintf( stderr, "debugLevel = %d\n", opts->debugLevel );
  fprintf( stderr, "----------- opcs_OptRec debug dump end -----------\n" );
}
