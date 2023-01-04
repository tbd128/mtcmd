//	Ophiuchus.h
//	Part of Ophiuchus
//	
//	Copyright (c) 2002-2023 tbd128 Allright Reserved.
//	This project is licensed under the MIT License, see the LICENSE.txt file for details.
//	
//	2.0   : Monday, March 11, 2002
//	2.1   : Monday, May 07, 2002
//

#ifdef	Ophiuchus_h__
#else
#define	Ophiuchus_h__

typedef	enum
{
  cmd_notSpecified = -1,
  cmd_showHelp = 0,	//  0 "help"
  cmd_showVers,		//  1 "vers"
  cmd_info,		//  2 "info"
  cmd_fullInfo,		//  3 "fullinfo"
  cmd_read,		//  4 "read"
  cmd_write,		//  5 "write"
  cmd_fsf,		//  6 "fsf"
  cmd_bsf,		//  7 "bsf"
  cmd_weof,		//  8 "weof"
  cmd_smk,		//  9 "smk"
  cmd_rewind,		// 10 "rewind"
  cmd_offline,		// 11 "offline"
  cmd_rewoffl,		// 12 "rewoffl"
  cmd_erase,		// 13 "erase"
  cmd_setopt		// 14 "setopt"
} opcs_CommandId;


typedef	struct	opcs_OptRec	opcs_OptRec;
struct	opcs_OptRec
{
  int	targetNo;
  int	cmd;
  int	bSize;
  int	bFactor;
  int	cnt;
  char	compressMode[4];	// "y" or "n"
  char	densityType[4];		// "l", "m", "h" or "c"
  char	inFileName[FILENAME_MAX];
  char	outFileName[FILENAME_MAX];
  int	debugLevel;
};

void	opcs_OptRec_setDefault( opcs_OptRec *opts );
int	Ophiuchus( const opcs_OptRec *opts );

#endif	// Ophiuchus_h__
