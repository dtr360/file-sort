//
//  sortroutines.cpp
//  Sorter
//
//  Created by Daniel Rencricca on 12/22/15.
//  Copyright © 2015 Daniel Rencricca. All rights reserved.
//



// DESCRIPTION:	This program uses a polyphase mergesort algorithm to sort a text
// 				file. Each line of text in the file must be a distinct record that
// 				ends with an endline ('\n') character. Additionally, each record 
// 				must contains fields separated by commas or tabs, where one of these
//				fields will be used as a sort key to sort the records in the file. 
//				It does the sort by performing the following steps: (1) create a
//				total of MAX_SRT_FILES sort files to be used to temporarily hold 
//				record data; (2) read up to MAX_ARR_SZ records (i.e. lines of text)
//				into a buffer array, where each element of the array consists of 
//				the record (i.e. a line of text)  and the sort key that is copied 
//				from that record; (3) sort the array by the key; (4) find the 
//				lowest key in the buffer that is greater than the last key read, 
//				and copy the record associated with that key from the buffer to a 
//				temporary sort file; (5) read in one new record (i.e. line of text) 
//				and add it to its correct sorted postion; (7) repeat from step 3 
//				until there are no more keys left that are greater than the last 
//				key read; (8) repeat from step 2 until the last sort file has been 
//				filled; (9) read the first record from each of the sort files into 
//				a tempfile array and get the sort key from each record; (10) Find 
//				the lowest key in the tempfile array and write the associated record 
//				into a temporary holder file;	(11) read a new text string from the 
//				sort file which previously had the lowest key and get the key from 
//				that string; (12) repeat from	step 10 until all sort files have 
//				been fully read; (13) erase the sort files and repeat from step 1 
//				until the input file has been fully read.This program uses a 
//				polyphase mergesort to sort a text file.


#include <iostream>
#include <string>
#include <assert.h>

#include "sortroutines.h"

using namespace std;

#define _THIS_FILE	L"SortRoutines"

#ifdef _DEBUG
uint OrgLineCnt;
#endif


////////////////////////////////////////////////////////////////////////////////
// FUNCTION:	SortRoutines
//
// DESCRIPTION	Constructor for SortRoutines.
//
// PARAMETERS:	pParent - parent.
//
// RETURNS:		Nothing.
////////////////////////////////////////////////////////////////////////////////
SortRoutines::SortRoutines(string inFile, string outFile, uint col1, uint col2,
                           uint col3)
{
    
#ifdef _DEBUG
    OrgLineCnt	= 0;
#endif
    
    // initialize variables
    m_iBufArrSz		= 0;
    m_iSrtFlArrSz	= 0;
    m_fProgress     = 0;
    m_aBufArr		= NULL;
    m_aSrtFlArr		= NULL;
    m_fpInfile		= NULL;
    m_sHoldFile		= HLDFILE;
    m_sUserFile     = inFile;
    m_bUsingQuotes  = false;
    m_sOutfile      = outFile;
    m_bSkipFirstLn  = true; // FIX THIS
    m_iSortCol1     = col1;
    m_iSortCol2     = col2;
    m_iSortCol3     = col3;

    
    // make space on heap for m_aBufArr and m_aSrtFlArr arrays
    AllocateBufArr(BUF_ARR_SZ);
    AllocateSrtFlArr(SRT_FL_ARR_SZ);
    
    // initialize m_aSrtFlArr array file pointers
    for (int x = 0; x < m_iSrtFlArrSz; x++)
        m_aSrtFlArr[x]->fp = NULL;
    
    // remove the file in case prior failed processing
    remove(m_sHoldFile.c_str());
}


////////////////////////////////////////////////////////////////////////////////
// FUNCTION:	~SortRoutines
//
// DESCRIPTION:	Destructor for SortRoutines. Close all sort files and
//				deallocate temporary arrays.
//
// PARAMETERS:	None.
//
// RETURNS:		Nothing.
////////////////////////////////////////////////////////////////////////////////
SortRoutines::~SortRoutines()
{
    DeleteSortFiles();
    
    if (m_fpInfile)
        fclose(m_fpInfile);
    
    remove(m_sHoldFile.c_str());
    
    DeallocateBufArr(m_iBufArrSz);
    
    DeallocateSrtFlArr(m_iSrtFlArrSz);
}

////////////////////////////////////////////////////////////////////////////////
// MEMORY ALLOCATION SUBROUTINES											  //
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// FUNCTION:	AllocateBufArr
//
// DESCRIPTION:	Allocates room on the heap for the m_aBufArr array. If there
//				is insufficient memory, it attempts to obtain 80% of amount
//				reached on first attempt.
//
// PARAMETERS:	-> maxSz - size of array we want to allocate.
//
// RETURNS:		Nothing.
////////////////////////////////////////////////////////////////////////////////
void SortRoutines::AllocateBufArr(int maxSz)
{
    try
    {
        m_aBufArr = new BufRecType*[maxSz];
        for (m_iBufArrSz = 0; m_iBufArrSz < maxSz; m_iBufArrSz++)
        {
            m_aBufArr[m_iBufArrSz] = new BufRecType;
        }
    }
    
    catch(...)
    {
        DeallocateBufArr(m_iBufArrSz);
        maxSz = (int) (m_iBufArrSz * .80); // try smaller size
        m_aBufArr = new BufRecType*[maxSz];
        for (m_iBufArrSz = 0; m_iBufArrSz < maxSz; m_iBufArrSz++)
        {
            m_aBufArr[m_iBufArrSz] = new BufRecType;
        }
    }
    
    assert(maxSz == m_iBufArrSz);
}

////////////////////////////////////////////////////////////////////////////////
// FUNCTION:	AllocateSrtFlArr
//
// DESCRIPTION:	Allocates room on the heap for the m_aSrtFlArr array. If
//				there is insufficient memory, it attempts to obtain 80% of
//				amount reached on first attempt.
//
// PARAMETERS:	-> maxSz - size of array we want to allocate.
//
// RETURNS:		Nothing.
////////////////////////////////////////////////////////////////////////////////
void SortRoutines::AllocateSrtFlArr(int maxSz)
{
    try
    {
        m_aSrtFlArr = new SrtFlRecType*[maxSz];
        for (m_iSrtFlArrSz = 0; m_iSrtFlArrSz < maxSz; m_iSrtFlArrSz++)
        {
            m_aSrtFlArr[m_iSrtFlArrSz] = new SrtFlRecType;
        }
        
        assert(maxSz == m_iSrtFlArrSz);
    }
    
    catch (...)
    {
        DeallocateSrtFlArr(m_iSrtFlArrSz);
        maxSz = (int) (m_iSrtFlArrSz * .80); // try smaller array
        m_aSrtFlArr = new SrtFlRecType*[maxSz];
        for (m_iSrtFlArrSz = 0; m_iSrtFlArrSz < maxSz; m_iSrtFlArrSz++)
        {
            m_aSrtFlArr[m_iSrtFlArrSz] = new SrtFlRecType;
        }
    }
}


////////////////////////////////////////////////////////////////////////////////
// FUNCTION:	DeallocateBufArr
//
// DESCRIPTION:	Deletes the m_aBufArr array.
//
// PARAMETERS:	-> bufArrSz	- number of elements in Buffer array.
//
// RETURNS:		Nothing.
////////////////////////////////////////////////////////////////////////////////
void SortRoutines::DeallocateBufArr(uint bufArrSz)
{
    assert(m_aBufArr != NULL); // should always exist
    
    if (m_aBufArr)
    {
        for (uint i = 1; i < bufArrSz; i++)
        {
            delete m_aBufArr[i];
        }
        
        delete[] m_aBufArr;
        
        m_aBufArr = NULL;
    }
}


////////////////////////////////////////////////////////////////////////////////
// FUNCTION:	DeallocateSrtFlArr
//
// DESCRIPTION:	Deletes the m_aSrtFlArr array.
//
// PARAMETERS:	-> srtFlArrSz -	number of items in m_aSrtFlArr array.
//
// RETURNS:		Nothing.
////////////////////////////////////////////////////////////////////////////////
void SortRoutines::DeallocateSrtFlArr(int srtFlArrSz)
{
    assert(m_aSrtFlArr != NULL); // should always exist
    
    if (m_aSrtFlArr)
    {
        for (int i = 0; i < srtFlArrSz; i++)
        {
            delete m_aSrtFlArr[i];
        }
        
        delete[] m_aSrtFlArr;
        
        m_aSrtFlArr = NULL;
    }
}


////////////////////////////////////////////////////////////////////////////////
// FILE MANAGEMENT SUBROUTINES												  //
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// FUNCTION:	FileIOError
//
// DESCRIPTION:	Called if an I/O error was found.  It displays an error
//				message, writes the error message to the log file, sets
//				the BadExit variable to TRUE and erases all files created.
//
// PARAMETERS:	-> msg	-	error message to display and write to log file.
//
// RETURNS:		Nothing.
////////////////////////////////////////////////////////////////////////////////
void SortRoutines::FileIOError(string msg)
{
    cout << msg.c_str();
    //wxMessageBox(msg, _THIS_FILE, wxOK | wxICON_ERROR);
    //AddLogEntry(msg);
    //pGedWiseFrame->SetBadExit();
}


////////////////////////////////////////////////////////////////////////////////
// FUNCTION:	InitTempFiles
//
// DESCRIPTION:	Initialize merge files by creating up to m_iSrtFlArrSz
//		temporary sort	files to be used to hold the runs of data
//		read from the input file.
//
// PARAMETERS:	-> startFileN - starting sort file number to create.
//
// RETURNS:		Nothing.
////////////////////////////////////////////////////////////////////////////////
bool SortRoutines::InitTempFiles(int startFileN)
{
    string filePathName;
    
    // create names for temporary merge sort files.
    for (int x = startFileN; x < m_iSrtFlArrSz; x++)
    {
        sprintf(m_aSrtFlArr[x]->name, SRTFILE, x);
        
        //filePathName = pGedWiseFrame->GetWorkDir() + m_aSrtFlArr[x]->name;
        filePathName = m_aSrtFlArr[x]->name;
        
        if (! (m_aSrtFlArr[x]->fp = fopen(filePathName.c_str(), "w+b")))
        {
            string msg;
            msg = printf(cErrFileOpen, "SR01", filePathName.c_str());
            FileIOError(msg);
            return false;
        }
    }
    
    return true;
}


////////////////////////////////////////////////////////////////////////////////
// FUNCTION:	DeleteSortFiles
//
// DESCRIPTION:	Delete temporary merge files that were created.
//
// PARAMETERS:	None.
//
// RETURNS:	Nothing.
////////////////////////////////////////////////////////////////////////////////
void SortRoutines::DeleteSortFiles(void)
{
    int		x;
    string	filePathName;
    
    for (x = 0; x < m_iSrtFlArrSz; x++)
    {
        if (m_aSrtFlArr[x]->fp)
        {
            fclose(m_aSrtFlArr[x]->fp); // close stream
            m_aSrtFlArr[x]->fp = NULL;
            
            filePathName = m_aSrtFlArr[x]->name;
            remove(filePathName.c_str()); // erase file
        }
    }
    
    return;
}


////////////////////////////////////////////////////////////////////////////////
// FUNCTION:	TermTmpFiles
//
// DESCRIPTION:	Clean up temporary merge sort files. Create a temporary
//				Holder file to hold sorted data.
//
// PARAMETERS:	None.
//
// RETURNS:		Nothing.
////////////////////////////////////////////////////////////////////////////////
bool SortRoutines::TermTmpFiles(void)
{
    string	filePathName;
    string	msg;
    int		lastPos = m_iSrtFlArrSz - 1; // do not erase last array item
    uint	i = 0;
    
    //AddLogEntry(_T("S-"), FALSE); // track number of times sort files created
    
    // Close the last sort file (as it will be renamed)
    if (m_aSrtFlArr[lastPos]->fp)
    {
        if (fclose(m_aSrtFlArr[lastPos]->fp))
        {
            msg = printf(cErrFileClose, "SR02a", m_aSrtFlArr[lastPos]->name);
            //AddLogEntry(msg);
            cout << msg;
            assert(false);
            // this should never happen, but don't make it fatal error for now
        }
        
        m_aSrtFlArr[lastPos]->fp = NULL;
    }
    
    // Rename the m_aSrtFlArr[lastPos] file to _holder.dat file.
    // Allow 6 attempts to rename file, as some RAID systems cannot keep up
    // with file I/O.
    filePathName = m_aSrtFlArr[lastPos]->name;
    
    while (rename(filePathName.c_str(), m_sHoldFile.c_str()))
    {
        msg = printf(cTryRename, "SR02b");
        cout << msg;
        //AddLogEntry(msg);
        
        if (i > 5)
        {
            msg= printf(cErrFileRen, "SR02c", filePathName.c_str());
            FileIOError(msg);
            return false;
        }
        i++;
    }
    
    DeleteSortFiles(); // delete all merge sort files
    
    return true;
}


////////////////////////////////////////////////////////////////////////////////
// FUNCTION:	RewindF
//
// DESCRIPTION:	Set pointer for m_aSrtFlArr[pos] to first record and read
//				firstr ecord key into m_aSrtFlArr[pos]. Also initializes the
//				eof field for m_aSrtFlArr[pos].
//
// Paramters:	-> pos		-	position within m_aSrtFlArr to read.
//				-> fileNo	-	determines which key to used, depending on the
//								file being sorted.
//
// RETURNS:		Nothing.
////////////////////////////////////////////////////////////////////////////////
bool SortRoutines::RewindF(const int pos)
{
    m_aSrtFlArr[pos]->eof = false;
    
    rewind(m_aSrtFlArr[pos]->fp);
    
    // Read next data element from the merge file m_aSrtFlArr[x].rec.dataLn
    if (! fgetws(m_aSrtFlArr[pos]->rec.dataLn, BUFFER_SZ, m_aSrtFlArr[pos]->fp))
    {
        if (feof(m_aSrtFlArr[pos]->fp)) // if at end of this m_aSrtFlArr
        {
            m_aSrtFlArr[pos]->eof = true; // if yes, then mark file as done
        }
        else
        {
            string msg;
            msg = printf(cErrFileRead, "SR05a", m_aSrtFlArr[pos]->name);
            FileIOError(msg);
            return false;
        }
    }
    else
    {
        GetKey(&m_aSrtFlArr[pos]->rec);
    }
    
    return true;
}


////////////////////////////////////////////////////////////////////////////////
// FILE SORTING SUBROUTINES  												  //
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// FUNCTION:	RecCompare
//
// DESCRIPTION:	Performs a case-insensitve comparison of two records.
//				line. The sort	key is assumed to be located at position 0
//				in each dataLn. The key ends at the first tab encountered
//				in the dataLn.
//
// INPUTS:		-> rec1	-	first record to compare.
//				-> rec2	-	second record to compare.
//
// RETURNS:		< 0	if rec1 less than rec2
//				= 0	rec1 identical to rec2
//				> 0	rec1 greater than rec2
////////////////////////////////////////////////////////////////////////////////
int SortRoutines::RecCmp(BufRecType* rec1, BufRecType* rec2)
{
    
    int result = 0;
    
    result = wcscmp(rec1->key1.c_str(), rec2->key1.c_str());
    
    if (result == 0) {
        result = wcscmp(rec1->key2.c_str(), rec2->key2.c_str());
    }
    
    if (result == 0) {
        result = wcscmp(rec1->key3.c_str(), rec2->key3.c_str());
    }
    
    return result;
}


////////////////////////////////////////////////////////////////////////////////
// FUNCTION:	GetKey
//
// DESCRIPTION:	Parses a line of text to retreive the sort key for that line.
//				The sort key is assumed to be located at position 0 in each
//				string of text.
//
// PARAMETERS:	-> rec      -	the record for which we want to get keys.
//
// RETURNS:		Nothing.
////////////////////////////////////////////////////////////////////////////////
void SortRoutines::GetKey(BufRecType* rec)
{
    
    assert (m_iSortCol1 > 0);
    
    wstring data = rec->dataLn; //convert line of data to wstring so we can use find

    assert(data.length() > 0);
    
    //wchar_t findChar[] = L","; // FIX THIS
    wchar_t findChar[] = L"\",\""; // FIX THIS
    
    
    if (data.find(L"\",\"", 0) > 0) {  // does file enclose data fields with quotes?...
        m_bUsingQuotes = true;
    }
    
    wstring::size_type      eLoc = 0;
    wstring::size_type		sLoc = eLoc;
    
    for (int i = 0; i < m_iSortCol1-1; i++)
    {
        sLoc = data.find(findChar, sLoc);
        sLoc += wcslen(findChar); // skip find character
    }
    
    if (m_bUsingQuotes and sLoc == 0) // skip quote at position 0
    {
        sLoc += 1;
    }
    
    eLoc = data.find(findChar, sLoc);
    rec->key1 = data.substr(sLoc, eLoc - sLoc);

    if (m_iSortCol2 > 0)
    {
        sLoc = eLoc = 0;
    
        for (int i = 0; i < m_iSortCol2-1; i++)
        {
            sLoc = data.find(findChar, sLoc);
            sLoc += wcslen(findChar); // skip find character
        }
    
        if (m_bUsingQuotes and sLoc == 0)  // skip quote at position 0
        {
            sLoc += 1;
        }
    
        eLoc = data.find(findChar, sLoc);
        rec->key2 = data.substr(sLoc, eLoc - sLoc);
    }
    
    if (m_iSortCol3 > 0)
    {
        sLoc = eLoc = 0;
    
        for (int i = 0; i < m_iSortCol3-1; i++)
        {
            sLoc = data.find(findChar, sLoc);
            sLoc += wcslen(findChar); // skip find character
        }
   
        if (m_bUsingQuotes and sLoc == 0)
        {
            sLoc += 1;
        }
    
        eLoc = data.find(findChar, sLoc);
        rec->key3 = data.substr(sLoc, eLoc - sLoc);
    }
}


////////////////////////////////////////////////////////////////////////////////
// FUNCTION:	SortList
//
// DESCRIPTION:	Sorts the buffer array in descending order (e.g. ZXYWVU...).
//				It bases the sort on the "key" variable in each array element.
//
// PARAMETERS: 	-> totBufSz -	holds a count of the number of items in the
//         						buffer array so that we don't needlessly sort
//								the entire buffer if it is not full.
//
// RETURNS:		Nothing.
////////////////////////////////////////////////////////////////////////////////
void SortRoutines::SortList(int totBufItems)
{
    int				x, y;
    BufRecType*		holder;
    
    for (x = 0; x < totBufItems-1; x++)
    {
        assert(totBufItems > 0);
        
        for (y = x+1; y < totBufItems; y++)
        {
            if (RecCmp(m_aBufArr[y], m_aBufArr[x]) > 0)
            {
                holder = m_aBufArr[x];
                m_aBufArr[x] = m_aBufArr[y];
                m_aBufArr[y] = holder;
            }
        }
    }
    
    return;
}

////////////////////////////////////////////////////////////////////////////////
// FUNCTION:	SortListIncr
//
// DESCRIPTION:	Sorts the buffer array in descending order (e.g. ZXYWVU...). It
//				assumes only one array element (at position 'pos') is out of
//				order.
//
// PARAMETERS: 	-> totBufSz	-	holds the count of the number of items in the
//         						m_aBufArr array so we don't needlessly sort
//								entire m_aBufArr array if it is not full.
//				-> pos		-	position of new data element added to the
//								m_aBufArr array.
//
// RETURNS:		Nothing.
////////////////////////////////////////////////////////////////////////////////
void SortRoutines::SortListIncr(const int totBufSz, int pos)
{
    BufRecType*	holder;
    
    if (totBufSz <= 0)
        return;
    
    // check if new array item needs to move up in the array.
    while (pos > 0 && RecCmp(m_aBufArr[pos], m_aBufArr[pos-1]) > 0)
    {
        holder = m_aBufArr[pos];
        m_aBufArr[pos] = m_aBufArr[pos-1];
        m_aBufArr[pos-1] = holder;
        pos--;
    }
    
    // check if new array item needs to move up in the array.
    while (pos < totBufSz-1 && RecCmp(m_aBufArr[pos], m_aBufArr[pos+1]) < 0)
    {
        holder = m_aBufArr[pos];
        m_aBufArr[pos] = m_aBufArr[pos+1];
        m_aBufArr[pos+1] = holder;
        pos++;
    }
    
    return;
}

////////////////////////////////////////////////////////////////////////////////
// FUNCTION:	AddToBuffer
//
// DESCRIPTION:	Add a line(s) of text to buffer array as well as the
//				corresponding integer key for each line.  It also sorts the
//				buffer and keeps track of the number of elements in the
//				buffer.
//
// PARAMETERS:	-> pos		-	position in buffer array to add a new number.
//								If position = -1 then start new array.
//				<> totBufSz	- 	keeps a count of the number of items in the
//               				buffer array so that we don't needlessly sort
//								the entire buffer if not full.
//
// RETURNS:		TRUE if successful, else FALSE only if error occurs.
////////////////////////////////////////////////////////////////////////////////
bool SortRoutines::AddToBuffer(int pos, int *totBufSz)
{
    int	x = 0;
    
    if (pos == -1) // start fresh by filling the entire buffer array.
    {
        while (x < m_iBufArrSz)
        {
            // read next line of data (including the CRLF)
            if (!fgetws(m_aBufArr[x]->dataLn, BUFFER_SZ, m_fpInfile))
            {
                if (feof(m_fpInfile))
                {
                    SortList(*totBufSz);
                    return true;
                }
                else
                {
                    string msg;
                    msg = printf(cErrFileRead, "SR03a", "Input");
                    FileIOError(msg);
                    return false;
                }
            }
            
#ifdef _DEBUG
            //if ((m_iLineTot & PROG_DIV) == 0)
            //    UpdateCnt++;
#endif
            
            //if ((m_iLineTot & PROG_DIV) == 0)
            //{
            //    pGedWiseFrame->CheckStatus ();
            //    if (pGedWiseFrame->Canceled ())
            //        return FALSE;
            //}
            
            // Get the key for current record.
            GetKey(m_aBufArr[x]);
            //assert(!m_aBufArr[x]->key->empty());
            
            m_iLineTot++;  // update line counter for log entry.
            
            ShowProgress(false, m_iLineTot);
            
            (*totBufSz)++; // keep count of buffer elements
            
            x++;
        } // while (x < m_iBufArrSz)
        
        SortList(*totBufSz);
    } // (pos == -1)
    
    else  // just replace one element in array.
        
    {
        assert(*totBufSz > 0);
        
        //wcout << L"\nBuffer Array:\n";
        //wcout << L"\nRemoving:";
        //wcout << m_aBufArr[pos]->key1->c_str();
        //wcout << L"\n";
        //for (int i=0; i < m_iBufArrSz; i++){
        //    wstring temp = m_aBufArr[i]->key1->c_str();
        //    wcout << temp;
        //    wcout << L"\n";
        //}
            
        
        if (!fgetws(m_aBufArr[pos]->dataLn, BUFFER_SZ, m_fpInfile))
        {
            if (feof(m_fpInfile))
            {
                m_aBufArr[pos]->key1.clear(); // empty so it sorts to bottom
                m_aBufArr[pos]->key2.clear(); // empty so it sorts to bottom
                m_aBufArr[pos]->key3.clear(); // empty so it sorts to bottom
                SortListIncr(*totBufSz, pos);
                (*totBufSz)--; // reduce count of buffer elements
                return true;
            }
            else
            {
                string msg;
                msg = printf(cErrFileRead, "SR03b", "Input");
                FileIOError(msg);
                return false;
            }
        }
        
        #ifdef _DEBUG
        //if ((m_iLineTot & PROG_DIV) == 0)
        //    UpdateCnt++;
        #endif
        
     
        // Get the key for current record.
        GetKey(m_aBufArr[pos]);
        //assert(!m_aBufArr[pos]->key->empty());
        
        m_iLineTot++; // update line counter
        ShowProgress(false, m_iLineTot);
        
        SortListIncr(*totBufSz, pos);
    } // else
    
//#ifdef _DEBUG
    /*wstring strHld;
     
     for (int z = 0; z < *totBufSz; z++)
     {
     strHld += *m_aBufArr[z]->key;
     strHld += CRLF;
     }
     cout << strHld
     //wxMessageBox (strHld); */
//#endif
    
    return true; // if we reached here then we are not at end of input file.
}

////////////////////////////////////////////////////////////////////////////////
// FUNCTION:	FindLowest
//
// DESCRIPTION:	Find the lowest key in the BufferStr that is greater than
//				lowStr. It uses the stricmp function to properly handle
//				strings with international characters.
//
// PARAMETERS:	<> pos		-	the position in the m_aBufArr array where
//								lowStr was found.
//		  		<> lowStr	-	the lowest number in m_aBufArr that is >= the
//      						highest number in the current run.
//				-> totBufSz	-	total current items in m_aBufArr.
//
// RETURNS:		TRUE if found, else FALSE if not found.
////////////////////////////////////////////////////////////////////////////////
bool SortRoutines::FindLowest(int* pos, BufRecType* lowRec, uint totBufSz)
{
    int	x = totBufSz - 1; // adj. for first array item at position 0

    assert(totBufSz > 0);
    
    while (x >= 0)
    {
        if (RecCmp(m_aBufArr[x], lowRec) >= 0)
        {
            lowRec->key1 = m_aBufArr[x]->key1;
            lowRec->key2 = m_aBufArr[x]->key2;
            lowRec->key3 = m_aBufArr[x]->key3;
            *pos = x;
            return true;
        }
        x--;
    } // while
    
    // Lowest was not found.
    lowRec->key1.clear(); // reset the low number for the next sort file
    lowRec->key2.clear(); // reset the low number for the next sort file
    lowRec->key3.clear(); // reset the low number for the next sort file
    
    //cout << "Starting new sort file...";
    
    return false;   // we have to start filling a new sort file
}

////////////////////////////////////////////////////////////////////////////////
// FUNCTION:	MergeSort
//
// DESCRIPTION:	MergeSort performs the following tasks:
//				(1) read the first text string from each of the sort files
//				into a m_aSrtFlArr array and get the integer key from each
//				line of text; (2) Find the lowest key in the m_aSrtFlArr array
//				and write the associated text string into a temporary holder
//				file; (3) read a new text string from the sort file which
//				previously had the lowest key and get the key from that
//				string; (4) repeat from step 1 until all sort files have
//				been fully read.
//
//	PARAMETERS:	None.
//
//	RETURNS:	TRUE if successful, else FALSE if error.
////////////////////////////////////////////////////////////////////////////////
bool SortRoutines::MergeSort(void)
{
    int			k;
    int			x;
    string      msg;
    
    // Prime the files and get first data line & key into m_aSrtFlArr array.
    for (x = 0; x < m_iSrtFileN; x++)
        RewindF(x);
    
    while (true)
    {
        // First, find the smallest key.
        k = -1;
        for (x = 0; x < m_iSrtFileN; x++)
        {
            if (m_aSrtFlArr[x]->eof) continue;
            if (k < 0 || (k != x && (RecCmp(&m_aSrtFlArr[k]->rec,
                                            &m_aSrtFlArr[x]->rec) > 0)))
                k = x;
        }
        
        if (k < 0) break; // break while loop if finished with all m_aSrtFlArr
        
        // Write m_aSrtFlArr[k].rec.dataLn to m_aSrtFlArr[m_iSrtFlArrSz-1].
        if (fwprintf(m_aSrtFlArr[m_iSrtFlArrSz-1]->fp, L"%S",
                       m_aSrtFlArr[k]->rec.dataLn) < 0)
        {
            msg = printf(cErrFileWrite, "SR06a", m_aSrtFlArr[m_iSrtFlArrSz-1]->name);
            FileIOError(msg);
            return false;
        }
        
        // Replace m_aSrtFlArr[k].rec->key with next item from sort file.
        if (!fgetws(m_aSrtFlArr[k]->rec.dataLn, BUFFER_SZ,
                      m_aSrtFlArr[k]->fp))
        {
            if (feof(m_aSrtFlArr[k]->fp)) // test for end of file
            {
                m_aSrtFlArr[k]->eof = true;
            }
            else
            {
                msg = printf(cErrFileRead, "#SR06b", m_aSrtFlArr[k]->name);
                FileIOError(msg);
                return false;
            }
        }
        else
        {
            GetKey(&m_aSrtFlArr[k]->rec);
        }
        
    } // while (true)
    
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// FUNCTION:	MakeRuns
//
// DESCRIPTION:	Make runs using replacement selection:
//				Read up to MAX_ARR_SZ lines of a text file into a buffer
//				array, where each element of the array consists of the text
//				string and an integer key that is copied from the text
//				string; (3) sort the array by the key;	(4) find the lowest
//				key in the buffer that is greater than the last key read,
//				and copy the text string associated with that key from the
//				buffer to a temporary sort file; (5) read in	one new line
//				of text into the buffer and resort the buffer; (7) repeat
//				from step 3 until	there are no more keys left that are
//				greater than the last key read; (8) repeat from step 2 until
//				he last sort file has been filled.
//
// PARAMETERS:	-> fileNo -	determines which key to used, depending on the file
//							being sorted.
//
// RETURNS:		TRUE if successful, else FALSE if error.
////////////////////////////////////////////////////////////////////////////////
bool SortRoutines::MakeRuns(void)
{
    int         totBufItems = 0; // number of elements in the buffer array
    int         pos			= 0; // position of lowest item in buffer array
    uint        i;
    BufRecType	lowRec;			// lowest item >= highest item in current run
    bool        notEndRun;		// signals the end of a run
    string      msg;			// error message
    string      filePathName;	// file path and name
    
    if (!InitTempFiles(0)) // initial all sort files
        return false; // error occurred
    
    if (!AddToBuffer(-1, &totBufItems)) // fill entire buffer
        return false; // error occurred
    
    m_iSrtFileN = 0; // init
    
    #ifdef _DEBUG
    cout << "Starting main loop in MakeRuns...\n";
    #endif
    
    while (totBufItems > 0) // get data from unsorted input file
    {
        lowRec.key1.clear();
        lowRec.key2.clear();
        lowRec.key3.clear();
        
        notEndRun = true;  // init
        
        while (m_iSrtFileN < m_iSrtFlArrSz-1) // add run of items to m_aSrtFlArr
        {
            // Add run of recs from m_aBufArr to m_aSrtFlArr[m_iSrtFileN] file
            while (notEndRun)
            {
                notEndRun = FindLowest(&pos, &lowRec, totBufItems);
                
                if (notEndRun)
                {
                    if (fwprintf(m_aSrtFlArr[m_iSrtFileN]->fp, L"%S",
                                   m_aBufArr[pos]->dataLn) < 0)
                    {
                        msg = printf(cErrFileWrite, "SR07a", m_aSrtFlArr[m_iSrtFileN]->name);
                        FileIOError(msg);
                        return false;
                    }
                    
                    assert(pos != -1);
                    
                    if (!AddToBuffer(pos, &totBufItems))
                        return false; // error occurred
                    
                } // if (notEndRun)
                
                if (totBufItems <= 0) break; // there is no more data to sort
                
            } // while (notEndRun)
            
            notEndRun = true;
            m_iSrtFileN++; // use next m_aSrtFlArr[srtFileN].fp file
            
            if (totBufItems <= 0) break; // there is no more data to sort
            
        } // while (m_iSrtFileN < m_aSrtFlArr-1)

        // Merge the sort files into one file.
        if (!MergeSort())
            return false;
        
        // Remove temporary sort files & rename last sort file to Holder file.
        if (!TermTmpFiles())
            return false;
     
        // Check if there is still data in the buffer.
        if (totBufItems > 0)
        {
            // Create the temporary sort files (except for _sort000.dat).
            if (!InitTempFiles(1))
                return false;
            
            // Rename _holder.dat file to _sort000.dat
            // Allow 6 trys to rename file: some RAID systems cannot keep
            // up with file I/O
            i = 0;
            filePathName = m_aSrtFlArr[0]->name;
            
            while (rename(m_sHoldFile.c_str(), filePathName.c_str()))
            {
                msg= printf(cTryRename, "SR07b");
                cout << msg;
                //AddLogEntry(msg);
                
                if (i > 5)
                {
                    msg = printf(cErrFileRen, "SR07c", m_sHoldFile.c_str());
                    FileIOError(msg);
                    return false;
                }
                i++;
            }
            
            // Open _sort000.dat file.
            if (!(m_aSrtFlArr[0]->fp = fopen(filePathName.c_str(), "r+b")))
            {
                msg = printf(cErrFileOpen, "SR07d", filePathName.c_str());
                FileIOError(msg);
                return false;
            }
            
        } // if (totBufNums > 0)
        
        m_iSrtFileN = 1; // skip m_aSrtFlArr[0] as that contains the data from first pass
        
    }  // while (totBufNums > 0)
    
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// FUNCTION:	SortFile
//
// DESCRIPTION:	Opens the files to be sorted and calls the merge sort subroutines.
//
// PARAMETERS:	-> fileNo	-	file to sort.
//
// RETURNS:		Nothing.
////////////////////////////////////////////////////////////////////////////////
bool SortRoutines::SortFile()
{
    wchar_t		ch;
    uint		lineCnt; // count lines to process in current file
    string      msg;
    string      filePathName;
    
    // Make sure there was room to allocate the arrays we require.
    if (m_iBufArrSz < MIN_ARR_SZ || m_iSrtFlArrSz < MIN_ARR_SZ)
    {
        msg = printf(cNoMemory, "SR08a");
        FileIOError(msg);
        return false;
    }
    
    m_iLineTot	= 0; // start line counter at zero.
    lineCnt		= 0; // count total lines in m_fpInfile
    
    // Open file we wish to sort.
    filePathName = "" + m_sUserFile;
    //cout << "Attempting to open " + filePathName + "\n";
    
    if (!(m_fpInfile = fopen(filePathName.c_str(), "r+b")))
    {
        msg = printf(cErrFileOpen, "SR08b", m_sUserFile.c_str());//DBFiles[fileNo]->name);
        FileIOError(msg);
        return false;
    }
    
    do // count the total lines in the file to sort
    {
        ch = fgetwc(m_fpInfile);
        if (ch == CHR_LF) lineCnt++;
    }
    while (ch != WEOF);
    
    
    // If nothing to sort in infile then stop.
    if (lineCnt == 0)
    {
        cout << "Error...no lines read\n";
        fclose(m_fpInfile);
        m_fpInfile = NULL;
        return true;
    }
    
    // Add log entry
    msg = printf("Sorting file: %s\n", filePathName.c_str());
    cout << msg;
    //AddLogEntry(msg);
    
    // Go back to first line of m_fpInfile
    rewind(m_fpInfile);
    
    // If first line of file is a header then hold onto it
    if (m_bSkipFirstLn) {
        fgetws(m_bFirstLn, BUFFER_SZ, m_fpInfile);
        lineCnt -=1; // subtract 1 line for header
    }
    
    // Init the progress bar
    ShowProgress(true, lineCnt);
    
    
    // Sorting the file.
    if (!MakeRuns())
        return false; // error occurred

    // Update the window progress bar.
    //pGedWiseFrame->ProgressUpdateData();
    //wxGetApp().Yield(TRUE);
    
    #ifdef _DEBUG
    OrgLineCnt = lineCnt;
    CheckSort();
    #endif
    
    // Add log entries.
    //AddLogEntry(STR_NUL, 2);
    
    msg = printf("Total Lines Read: = %d\n", m_iLineTot);
    cout << msg;
    //AddLogEntry(msg, 2);
    
    // Close m_fpInfile file.
    fclose(m_fpInfile);
    m_fpInfile = NULL;
    
    if (m_bSkipFirstLn) {
        FILE*   fP;
        FILE*   fPOutfile;
        wchar_t dataLn[BUFFER_SZ+1];
        
        cout << "Adding header to file...\n";
        
        fP = fopen(m_sHoldFile.c_str(), "r+b");
        
        fPOutfile = fopen(m_sOutfile.c_str(), "w+b");
        
        // FIRST, write header line.
        if (fwprintf(fPOutfile, L"%S", m_bFirstLn) < 0)
        {
            msg = printf(cErrFileWrite, "SR09a", m_sOutfile.c_str());
            fclose(fP);
            fclose(fPOutfile);
            FileIOError(msg);
            return false;
        }
        
        // SECOND, write all lines from m_sHoldFile.
        while (fgetws(dataLn, BUFFER_SZ, fP))
        {
            if (fwprintf(fPOutfile, L"%S", dataLn) < 0)
            {
                msg = printf(cErrFileWrite, "SR09b", m_sOutfile.c_str());
                fclose(fP);
                fclose(fPOutfile);
                FileIOError(msg);
                return false;
            }
        }
        fclose(fP);
        fclose(fPOutfile);
        remove (m_sHoldFile.c_str());
        
    } // if (m_bSkipFirstLn)
    else
    {
        // Rename _holder.dat file to m_fpInfile's file.
        if (rename(m_sHoldFile.c_str(), m_sOutfile.c_str()))
        {
            msg = printf(cErrFileRen, "SR08c", m_sHoldFile.c_str());
            FileIOError(msg);
            return false;
        }
    }
    
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// FUNCTION:	ShowProgress
//
// DESCRIPTION:	Updates the progress bar in the console out
//
// PARAMETERS:	-> setCnt -
//              -> count -
//
// RETURNS:		Nothing.
////////////////////////////////////////////////////////////////////////////////
void SortRoutines::ShowProgress(bool setCnt, uint count)
{
    if (setCnt)
    {
        m_fProgress = count;
    }
    else
    {
        if (count % 100 != 0 and count != m_fProgress)
            return;
        
        int barWidth = 60;
        float progPct = float(count/m_fProgress);
        
        std::cout << "[";
        int pos = barWidth * progPct;
        for (int i = 0; i < barWidth; ++i)
        {
            if (i < pos) std::cout << "=";
            else if (i == pos) std::cout << ">";
            else std::cout << " ";
        }
        std::cout << "] " << int(progPct * 100.0) << " %\r";
        std::cout.flush();
    }
}


#ifdef _DEBUG
////////////////////////////////////////////////////////////////////////////////
// FUNCTION:	CheckSort - FOR DEBUGING PURPOSES ONLY
//
// DESCRIPTION:	Checks the last sorted file to make sure it was sorted
//				correctly. Makes sure total lines in final file is same as
//				in original file.
//				FOR DEBUGING PURPOSES ONLY.  Do no compile this in final code.
//
// PARAMETERS:	-> fileNo - file number just sorted.
//
// RETURNS:		Nothing.
////////////////////////////////////////////////////////////////////////////////
void SortRoutines::CheckSort(void)
{
    BufRecType      rec1;
    BufRecType      rec2;
    FILE*			fP;
    uint			chkLineCnt = 0;
    
    cout << "\nDEBUG: Checking that data was sorted correctly.\n";
    
    fP = fopen(m_sHoldFile.c_str(), "r+b");
    
    while (fgetws(rec2.dataLn, BUFFER_SZ, fP))
    {
        chkLineCnt++;
        
        GetKey(&rec2);
        
        if (RecCmp(&rec2, &rec1) < 0)
        {
            FileIOError("CheckSort Sorting Error");
        }
        rec1.key1.clear();
        rec1.key1.append(rec2.key1.c_str());
        rec1.key2.clear();
        rec1.key2.append(rec2.key2.c_str());
        rec1.key3.clear();
        rec1.key3.append(rec2.key3.c_str());
    }
    
    assert(chkLineCnt == OrgLineCnt);
    
    cout << "DEBUG: Data was sorted correctly.\n";
    
    fclose(fP);
}
#endif
