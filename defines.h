////////////////////////////////////////////////////////////////////////////////
// PROGRAM:			GedWise 8
//
// FILE:			defines.h
//
// Written by:		Daniel Rencricca
//
// Last Revision:	July 18, 2009
//
// DESCRIPTION:		Header file containing defines used by various modules.
//
// Put this in Linker > Input > "Ignore Specific Library": LIBCMTD MSVCRTD
// Make sure wxWidget libraries all have the following settings for Batch Build
// Configuration Properties > C/C++ > Code Generation > Runtime Library >
// should all be set to /MT or /MTd (NOT the DLL libraries).
// Under Configuration Properties > General > Character Set select
// 'Use Unicode Character set' for the Unicode build.
//
// Required libraries:
// odbc32.lib comctl32.lib rpcrt4.lib wsock32.lib wxmsw28_core.lib wxbase28.lib
// wxpng.lib wxzlib.lib wxregex.lib wxexpat.lib wxmsw28_xrc.lib wxmsw28_adv.lib
// wxbase28_xml.lib rapi.lib
//
//
// VC++ Directories:
// Include:		$(WXWIN)\include\msvc
//				C:\Program Files\Microsoft Visual Studio 8\WTL80\include
//				
// Library:		$(WXWIN)\lib\vc_lib
//				C:\Program Files\Windows CE Tools\wce500\
//					Windows Mobile 5.0 Pocket PC SDK\Activesync\Lib
//
// Building WxWidgets libraries:
// Batch Build only the libraries you are going to use.
// Under Properties > C/C++ > Code Generation > Runtime Library > 
// select Multi-threaded and Mult-threaded Debug (NOT THE DLL libraries).
//
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _DEFINES_H_
#define _DEFINES_H_


#define _DEBUG

#define LOGFILE			_T("logfile.txt")		// log file
#define ERR_LOG_FILE	_T("errlog.txt")		// error log file

#define	MAX_LOGFILE_SZ	131072

// Windows uses a pair of CR and LF characters to terminate lines. UNIX (Including
// Linux and FreeBSD) uses an LF character only. The Mac uses a CR character only.


#define	CHR_TAB			 L'\t'	    // tab character (0x0009)
#define	CHR_COM			 L','	    // tab character (0x0009)
#define	CHR_LF			 L'\n'	    // newline character (0x000A)
#define	CHR_NUL			 L'\0'	    // end of line character (0x0000)
#define	CHR_CR			 L'\r'    	// carriage return character (0x000D)
#define CHR_SP			 L' '
#define STR_SP			 L" "
#define STR_NUL			 L"\0"	    // string terminator

#define	BUFFER_SZ		0xffff	// should be plenty big enough for one line of GEDCOM file

// File Open and Write Errors
#define	cWriteErr					0x01
#define	cOpenErr					0x02
#define	cUnkErr						0x03
#define	cReadErr					0x04

#endif // _DEFINES_H_
