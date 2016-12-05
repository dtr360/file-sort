////////////////////////////////////////////////////////////////////////////////
// PROGRAM:			Sorter 1.0
//
// FILE:			sortroutines.h
//
// DESCRIPTION:		Header file for sortroutines.cpp.
////////////////////////////////////////////////////////////////////////////////

#ifndef _SORT_ROUTINES_H_
#define _SORT_ROUTINES_H_

#include "defines.h"
#include <string>
#include <iostream>
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Constant Definitions
////////////////////////////////////////////////////////////////////////////////

const char cErrFileOpen[]	= "Error #%s opening file: %s\n";
const char cErrFileRead[]	= "Error #%s reading from file: %s\n";
const char cErrFileWrite[]	= "Error #%s writing to file: %s\n";
const char cErrFileRen[]    = "Error #%s renaming file: %s\n";
const char cErrFileClose[]	= "Error #%s closing file: %s\n";
const char cTryRename[]     = "Re-attempting file rename #%s\n";
const char cNoMemory[]		= "Error #%s insufficient memory for array.\n";

#define SRTFILE				"_sort%03d.dat"	// Temporary sort file name
#define HLDFILE				"_holder.dat" // Temp file for sorted data


#define WORK_DIR            ""


// Note the number of sort files makes the biggest difference in sorting time.
// Increasing the BUF_ARR_SZ seems to slow down the sort time signficantly.
#define SRT_FL_ARR_SZ	24	// max number of temporary sort files created
#define BUF_ARR_SZ		24	// max number of elements for buffer array
#define MIN_ARR_SZ		3	// minimum size of m_aSrtFlArr & m_aBufArr arrrays
#define FNAME_SZ		16	// maximum size of a file name (eg "_sort000.dat")


//typedef wchar_t DataLineType[BUFFER_SZ+1]; // a line of data read from input file.

typedef struct // holds line of data and its sort key
{
    wstring         key1;
    wstring         key2;
    wstring         key3;
	wchar_t         dataLn[BUFFER_SZ+1]; // a line of data read from input file.
}	BufRecType;


typedef struct 
{
	FILE*		fp;		// file pointer to a temporary sort file
	char		name[FNAME_SZ];	// sort file name (eg _sort001.dat)
	BufRecType	rec;		// line records
	bool		eof;		// end of file flag
}	SrtFlRecType;


////////////////////////////////////////////////////////////////////////////////
// SortRoutines Class Definition
////////////////////////////////////////////////////////////////////////////////
class SortRoutines
{

public:

	SortRoutines(string inFile, string outFile="outfile.txt", uint col1=1, uint col2=0,
                 uint col3=0);
	~SortRoutines();
    bool SortFile(void);

protected:

	bool		AddToBuffer(int position, int *totBufNums);
	void		AllocateBufArr(int maxSz);
	void		AllocateSrtFlArr(int maxSz);
	void		DeallocateBufArr(uint bufArrSz);
	void		DeallocateSrtFlArr(int srtFlArrSz);
	void		DeleteSortFiles(void);
	void		FileIOError(string errMsg);
	bool		FindLowest(int* pos, BufRecType* holdRec, uint totBufSz);
	void		GetKey(BufRecType* rec);
	bool		InitTempFiles(int startFileN);
	bool		MakeRuns(void);
	bool		MergeSort(void);
    int         RecCmp(BufRecType* rec1, BufRecType* rec2);
	bool		RewindF(const int pos);
    void        ShowProgress(bool setCnt, uint count);
	void		SortList(int totBufSz);
	void		SortListIncr(const int totBufSz, int pos);
	bool		TermTmpFiles(void);

	#ifdef _DEBUG
	void	CheckSort(void); // checks files are sorted correctly
	#endif

	//GedWiseFrame*	pGedWiseFrame;

	BufRecType**	m_aBufArr;			// buffer of text lines to be sorted
	SrtFlRecType**	m_aSrtFlArr;		// sort file array
	int				m_iBufArrSz;		// holds actual size of m_Buffer array
	int				m_iSrtFlArrSz;		// holds actual size of buffer array
	uint			m_iLineTot;			// counter for total lines in infile
	uint			m_iTotInFiles;		// count of total sort files
	int				m_iSrtFileN;		// current sort file num being processed
    string          m_sOutfile;         // name of output file
	FILE*			m_fpInfile;			// input file containing unsorted text
	string          m_sHoldFile;		// name of the temporary Holder File
    string          m_sUserFile;        // file to be sorted
    bool            m_bSkipFirstLn;     // skip first line of data file (header)
    wchar_t         m_bFirstLn[BUFFER_SZ+1]; // first line of data file
    float           m_fProgress;
    bool            m_bUsingQuotes;     // flag file has quotes between fields
    uint            m_iSortCol1;
    uint            m_iSortCol2;
    uint            m_iSortCol3;
};

#endif //SORT_ROUTINES_H_
