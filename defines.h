//
//  defines.h
//  Sorter
//
//  Created by Daniel Rencricca on 12/22/15.
//  Copyright © 2015 Daniel Rencricca. All rights reserved.
//
//  Description
//  Header file containing defined values used by various modules.
//

#ifndef _DEFINES_H_
#define _DEFINES_H_


#define _DEBUG

#define LOGFILE			_T("logfile.txt")		// log file
#define ERR_LOG_FILE	_T("errlog.txt")		// error log file

#define	MAX_LOGFILE_SZ	131072

// Windows uses a pair of CR and LF characters to terminate lines. UNIX (Including
// Linux and FreeBSD) uses an LF character only. The Mac uses a CR character only.

#define	CHR_TAB			 L'\t'	    // tab character (0x0009)
#define	CHR_COM			 L','	    // comma (0x002C)
#define	CHR_LF			 L'\n'	    // newline character (0x000A)

#define	BUFFER_SZ		0xffff	// should be plenty big enough

// File Open and Write Errors
#define	cWriteErr					0x01
#define	cOpenErr					0x02
#define	cUnkErr						0x03
#define	cReadErr					0x04

#endif // _DEFINES_H_
