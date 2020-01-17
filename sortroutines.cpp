/**
 * @file sortroutines.cpp
 * @author Daniel Rencrica
 * @brief Sort a csv file using a polyphase mergesort
 * 
 * This program uses a polyphase mergesort algorithm to sort a text
 * file. Each line of text in the file must be a distinct record that
 * ends with an endline ('\n') character. Additionally, each record
 * must contains fields separated by commas or tabs, where one of these
 * fields will be used as a sort key to sort the records in the file.
 * It does the sort by performing the following steps: (1) create a
 * total of MAX_SRT_FILES sort files to be used to temporarily hold
 * record data; (2) read up to MAX_ARR_SZ records (i.e. lines of text)
 * into a buffer array, where each element of the array consists of
 * the record (i.e. a line of text)  and the sort key that is copied
 * from that record; (3) sort the array by the key; (4) find the
 * lowest key in the buffer that is greater than the last key read,
 * and copy the record associated with that key from the buffer to a
 * temporary sort file; (5) read in one new record (i.e. line of text)
 * and add it to its correct sorted postion; (7) repeat from step 3
 * until there are no more keys left that are greater than the last
 * key read; (8) repeat from step 2 until the last sort file has been
 * filled; (9) read the first record from each of the sort files into
 * a tempfile array and get the sort key from each record; (10) Find
 * the lowest key in the tempfile array and write the associated record
 * into a temporary holder file; (11) read a new text string from the
 * sort file which previously had the lowest key and get the key from
 * that string; (12) repeat from step 10 until all sort files have
 * been fully read; (13) erase the sort files and repeat from step 1
 * until the input file has been fully read.This program uses a
 * polyphase mergesort to sort a text file.
 * 
 * @version 1.1
 * @date 2015-12-22
 * @copyright Copyright (c) 2015
 * 
 */

#include <iostream>
#include <stdio.h>
#include <string>
#include <assert.h>

#include "sortroutines.h"

using namespace std;

#define _THIS_FILE L"SortRoutines"

#ifdef _DEBUG
uint OrgLineCnt;
#endif

/**
 * @brief Construct a new Sort Routines:: Sort Routines object
 * 
 * @param inFile    name of file to SortRoutines.
 * @param outFile   name of file in which to save sorted data.
 * @param col1      name of file in which to save sorted data.
 * @param col2      second column (if any) to use as sort key.
 * @param col3      third column (if any) to use as sort key.
 */
SortRoutines::SortRoutines(string inFile, string outFile, uint col1, uint col2,
                           uint col3)
{

#ifdef _DEBUG
    OrgLineCnt = 0;
#endif

    // initialize variables
    m_iBufArrSz = 0;
    m_iSrtFlArrSz = 0;
    m_fProgress = 0;
    m_aBufArr = NULL;
    m_aSrtFlArr = NULL;
    m_fpInfile = NULL;
    m_sHoldFile = HLDFILE;
    m_sUserFile = inFile;
    m_bUsingQuotes = false;
    m_sOutfile = outFile;
    m_bSkipFirstLn = true;
    m_iSortCol1 = col1;
    m_iSortCol2 = col2;
    m_iSortCol3 = col3;
    m_LogFileP = NULL;

    // make space on heap for m_aBufArr and m_aSrtFlArr arrays
    AllocateBufArr(BUF_ARR_SZ);
    AllocateSrtFlArr(SRT_FL_ARR_SZ);

    // initialize m_aSrtFlArr array file pointers
    for (int x = 0; x < m_iSrtFlArrSz; x++)
        m_aSrtFlArr[x]->fp = NULL;

    // remove the file in case prior failed processing
    remove(m_sHoldFile.c_str());
}

/**
 * @brief Destroy the Sort Routines:: Sort Routines object
 * Close all sort files and deallocate temporary arrays.
 */
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
// MEMORY ALLOCATION SUBROUTINES                                              //
////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Allocates room on the heap for the m_aBufArr array. If there is
 *  insufficient memory, it attempts to obtain 80% of amount reached on first
 *  attempt.
 * 
 * @param maxSz The size of array we want to allocate.
 * 
 * @return Void.
 */
void SortRoutines::AllocateBufArr(int maxSz)
{
    try
    {
        m_aBufArr = new BufRecType *[maxSz];
        for (m_iBufArrSz = 0; m_iBufArrSz < maxSz; m_iBufArrSz++)
        {
            m_aBufArr[m_iBufArrSz] = new BufRecType;
        }
    }

    catch (...)
    {
        DeallocateBufArr(m_iBufArrSz);
        maxSz = (int)(m_iBufArrSz * .80); // try smaller size
        m_aBufArr = new BufRecType *[maxSz];
        for (m_iBufArrSz = 0; m_iBufArrSz < maxSz; m_iBufArrSz++)
        {
            m_aBufArr[m_iBufArrSz] = new BufRecType;
        }
    }

    assert(maxSz == m_iBufArrSz);
}

/**
 * @brief Allocates room on the heap for the m_aSrtFlArr array. If there is
 *  insufficient memory, it attempts to obtain 80% of amount reached on first
 *  attempt.
 * 
 * @param maxSz The size of array we want to allocate.
 * 
 * @return Void.
 */
void SortRoutines::AllocateSrtFlArr(int maxSz)
{
    try
    {
        m_aSrtFlArr = new SrtFlRecType *[maxSz];
        for (m_iSrtFlArrSz = 0; m_iSrtFlArrSz < maxSz; m_iSrtFlArrSz++)
        {
            m_aSrtFlArr[m_iSrtFlArrSz] = new SrtFlRecType;
        }

        assert(maxSz == m_iSrtFlArrSz);
    }

    catch (...)
    {
        DeallocateSrtFlArr(m_iSrtFlArrSz);
        maxSz = (int)(m_iSrtFlArrSz * .80); // try smaller array
        m_aSrtFlArr = new SrtFlRecType *[maxSz];
        for (m_iSrtFlArrSz = 0; m_iSrtFlArrSz < maxSz; m_iSrtFlArrSz++)
        {
            m_aSrtFlArr[m_iSrtFlArrSz] = new SrtFlRecType;
        }
    }
}

/**
 * @brief Deletes the m_aBufArr array.
 * 
 * @param bufArrSz The number of elements in Buffer array.
 * 
 * @return Void.
 */
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

/**
 * @brief Deletes the m_aSrtFlArr array.
 * 
 * @param srtFlArrSz number of items in m_aSrtFlArr array.
 * 
 * @return Void.
 */
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
// FILE MANAGEMENT SUBROUTINES                                                  //
////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Called if an I/O error was found.  It displays an error message, 
 * writes the error message to the log file, sets the BadExit variable to 
 * TRUE and erases all files created.
 * 
 * @param msg Error message to display and write to log file.
 * 
 * @return Void.
 */
void SortRoutines::FileIOError(string msg)
{
    printf("%s", msg.c_str());
    if ((m_LogFileP = fopen (LOGFILE, "a")) != NULL)
    {
        fprintf(m_LogFileP, "%s\n", msg.c_str());
        fclose(m_LogFileP);
    }
    AddLogEntry(msg);
}

/**
 * @brief Adds a message to the log file and displays a message if there is an
 * error writing to the file.
 * 
 * @param logMessage Message to add to the log file.
 * 
*/
void SortRoutines::AddLogEntry(const string msg)
{
    #ifdef _DEBUG
    if ((m_LogFileP = fopen (LOGFILE, "a")) != NULL)
    {
        fprintf(m_LogFileP, "%s\n", msg.c_str());
        fclose(m_LogFileP);
    }
    #endif
}

/**
 * @brief Initialize merge files by creating up to m_iSrtFlArrSz temporary sort
 * files to be used to hold the runs of data read from the input file.
 * 
 * @param startFileN Starting sort file number to create.
 * 
 * @return true if it successfully created the sort files, else false if error.
  */
bool SortRoutines::InitTempFiles(int startFileN)
{
    string filePathName;

    // create names for temporary merge sort files.
    for (int x = startFileN; x < m_iSrtFlArrSz; x++)
    {
        sprintf(m_aSrtFlArr[x]->name, SRTFILE, x);

        filePathName = m_aSrtFlArr[x]->name;

        if (!(m_aSrtFlArr[x]->fp = fopen(filePathName.c_str(), "w+b")))
        {
            sprintf(msg_buf, cErrFileOpen, "SR01", filePathName.c_str());   
            FileIOError(msg_buf);
            return false;
        }
    }

    return true;
}

/**
 * @brief Delete temporary merge files that were created.
 * @return Void.
 */
void SortRoutines::DeleteSortFiles(void)
{
    int x;
    string filePathName;


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

/**
 * @brief Clean up temporary merge sort files. Create a temporary. Holder file
 *  to hold sorted data.
 * 
 * @return true if the sort files were successfully removed, else false if error.
 */
bool SortRoutines::TermTmpFiles(void)
{
    string filePathName;
    int lastPos = m_iSrtFlArrSz - 1; // do not erase last array item
    uint i = 0;

    // Close the last sort file (as it will be renamed)
    if (m_aSrtFlArr[lastPos]->fp)
    {
        if (fclose(m_aSrtFlArr[lastPos]->fp))
        {
            sprintf(msg_buf, cErrFileClose, "SR02a", m_aSrtFlArr[lastPos]->name);
            AddLogEntry(msg_buf);
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
        sprintf(msg_buf, cTryRename, "SR02b");
        AddLogEntry(msg_buf);

        if (i > 5)
        {
            sprintf(msg_buf, cErrFileRen, "SR02c", filePathName.c_str());
            FileIOError(msg_buf);
            return false;
        }
        i++;
    }

    DeleteSortFiles(); // delete all merge sort files

    return true;
}

/**
 * @brief Set pointer for m_aSrtFlArr[pos] to first record and read first 
 * record key into m_aSrtFlArr[pos]. Also initializes the eof field for 
 * m_aSrtFlArr[pos].
 * 
 * @param pos The position within m_aSrtFlArr to read.
 * @return true if the record was read successfully, else false if error.
 */
bool SortRoutines::RewindF(const int pos)
{
    m_aSrtFlArr[pos]->eof = false;

    rewind(m_aSrtFlArr[pos]->fp);

    // Read next data element from the merge file m_aSrtFlArr[x].rec.dataLn
    if (!fgetws(m_aSrtFlArr[pos]->rec.dataLn, BUFFER_SZ, m_aSrtFlArr[pos]->fp))
    {
        if (feof(m_aSrtFlArr[pos]->fp)) // if at end of this m_aSrtFlArr
        {
            m_aSrtFlArr[pos]->eof = true; // if yes, then mark file as done
        }
        else
        {
            sprintf(msg_buf, cErrFileRead, "SR05a", m_aSrtFlArr[pos]->name);
            FileIOError(msg_buf);
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
// FILE SORTING SUBROUTINES                                                    //
////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Performs a case-insensitve comparison of two records. 
 * The sort key is assumed to be located at position 0 in each dataLn.
 * The key ends at the first tab encountered in the dataLn.
 * 
 * @param rec1 The first record to compare.
 * @param rec2 The second record to compare.
 * 
 * @return int This will be < 0 if rec1 less than rec2, = 0 rec1 if identical
 * to rec2, or > 0 if rec1 greater than rec2
 */
int SortRoutines::RecCmp(BufRecType *rec1, BufRecType *rec2)
{

    int result = 0;

    result = wcscmp(rec1->key1.c_str(), rec2->key1.c_str());

    if (result == 0)
    {
        result = wcscmp(rec1->key2.c_str(), rec2->key2.c_str());
    }

    if (result == 0)
    {
        result = wcscmp(rec1->key3.c_str(), rec2->key3.c_str());
    }

    return result;
}
/**
 * @brief Parses a line of text to retreive the sort key for that line. The
 *  sort key is assumed to be located at position 0 in each string of text.
 * 
 * @param rec The record for which we want to get keys.
 * 
 * @return Void.
 */
void SortRoutines::GetKey(BufRecType *rec)
{

    assert(m_iSortCol1 > 0);

    wstring data = rec->dataLn; //convert line of data to wstring so we can use find

    assert(data.length() > 0);

    //wchar_t findChar[] = L","; // FIX THIS
    wchar_t findChar[] = L"\",\""; // FIX THIS

    if (data.find(L"\",\"", 0) > 0)
    { // does file enclose data fields with quotes?...
        m_bUsingQuotes = true;
    }

    wstring::size_type eLoc = 0;
    wstring::size_type sLoc = eLoc;

    for (int i = 0; i < m_iSortCol1 - 1; i++)
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

        for (int i = 0; i < m_iSortCol2 - 1; i++)
        {
            sLoc = data.find(findChar, sLoc);
            sLoc += wcslen(findChar); // skip find character
        }

        if (m_bUsingQuotes and sLoc == 0) // skip quote at position 0
        {
            sLoc += 1;
        }

        eLoc = data.find(findChar, sLoc);
        rec->key2 = data.substr(sLoc, eLoc - sLoc);
    }

    if (m_iSortCol3 > 0)
    {
        sLoc = eLoc = 0;

        for (int i = 0; i < m_iSortCol3 - 1; i++)
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

/**
 * @brief Sorts the buffer array in descending order (e.g. ZXYWVU...). It bases
 *  the sort on the "key" variable in each array element.
 * 
 * @param totBufItems This holds a count of the number of items in the buffer
 *  array so that we don't needlessly sort the entire buffer if it is not full.
 * 
 * @return Void.
 */
void SortRoutines::SortList(int totBufItems)
{
    int x, y;
    BufRecType *holder;

    DBGVAR(totBufItems);

    for (x = 0; x < totBufItems - 1; x++)
    {
        assert(totBufItems > 0);

        for (y = x + 1; y < totBufItems; y++)
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


/**
 * @brief Sorts the buffer array in descending order (e.g. ZXYWVU...). It
 *  assumes only one array element (at position 'pos') is out of order.
 * 
 * @param totBufSz holds the count of the number of items in the 
 * m_aBufArr array so we don't needlessly sort entire m_aBufArr array
 *  if it is not full.
 * @param pos position of new data element added to the m_aBufArr array.
 * 
 * @return Void.
 */
void SortRoutines::SortListIncr(const int totBufSz, int pos)
{
    BufRecType *holder;

    if (totBufSz <= 0)
        return;

    // check if new array item needs to move up in the array.
    while (pos > 0 && RecCmp(m_aBufArr[pos], m_aBufArr[pos - 1]) > 0)
    {
        holder = m_aBufArr[pos];
        m_aBufArr[pos] = m_aBufArr[pos - 1];
        m_aBufArr[pos - 1] = holder;
        pos--;
    }

    // check if new array item needs to move up in the array.
    while (pos < totBufSz - 1 && RecCmp(m_aBufArr[pos], m_aBufArr[pos + 1]) < 0)
    {
        holder = m_aBufArr[pos];
        m_aBufArr[pos] = m_aBufArr[pos + 1];
        m_aBufArr[pos + 1] = holder;
        pos++;
    }

    return;
}

/**
 * @brief Add a line(s) of text to buffer array as well as the corresponding
 * integer key for each line. It also sorts the buffer and keeps track of the
 * number of elements in the buffer.
 * 
 * @param pos position in buffer array to add a new number. If position = -1
 * then start new array.
 * @param totBufSz keeps a count of the number of items in the buffer array
 *  so that we don't needlessly sort the entire buffer if its not full.
 * 
 * @return true if the operation was successful, else false if an error
 * occurred.
 */
bool SortRoutines::AddToBuffer(int pos, int *totBufSz)
{
    int x = 0;

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
                    sprintf(msg_buf, cErrFileRead, "SR03a", "Input");
                    FileIOError(msg_buf);
                    return false;
                }
            }

            // Get the key for current record.
            GetKey(m_aBufArr[x]);
            //assert(!m_aBufArr[x]->key->empty());

            m_iLineTot++; // update line counter for log entry.

            ShowProgress(false, m_iLineTot);

            (*totBufSz)++; // keep count of buffer elements

            x++;
        } // while (x < m_iBufArrSz)

        SortList(*totBufSz);
    } // (pos == -1)

    else // just replace one element in array.

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
                sprintf(msg_buf, cErrFileRead, "SR03b", "Input");
                FileIOError(msg_buf);
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

    return true; // if we reached here then we are not at end of input file.
}

/**
 * @brief Find the lowest key in the BufferStr that is greater than lowStr.
 * It uses the stricmp function to properly handle strings with international
 * characters.
 * 
 * @param pos the position in the m_aBufArr array where lowStr was found.
 * @param lowRec the lowest number in m_aBufArr that is >= the highest number
 * in the current run.
 * @param totBufSz total current items in m_aBufArr.
 * 
 * @return true if the lowest key was found, else false.
 */
bool SortRoutines::FindLowest(int *pos, BufRecType *lowRec, uint totBufSz)
{
    int x = totBufSz - 1; // adj. for first array item at position 0

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

    return false; // we have to start filling a new sort file
}

/**
 * @brief MergeSort performs the following tasks: (1) read the first text
 * string from each of the sort files into a m_aSrtFlArr array and get the 
 * integer key from each line of text; (2) Find the lowest key in the
 * m_aSrtFlArr array and write the associated text string into a temporary 
 * holder file; (3) read a new text string from the sort file which previously
 * had the lowest key and get the key from that string; (4) repeat from step 
 * 1 until all sort files have been fully read.
 * 
 * @return true if function was successful else false if error occurred.
 */
bool SortRoutines::MergeSort(void)
{
    int k;
    int x;

    // Prime the files and get first data line & key into m_aSrtFlArr array.
    for (x = 0; x < m_iSrtFileN; x++)
        RewindF(x);

    while (true)
    {
        // First, find the smallest key.
        k = -1;
        for (x = 0; x < m_iSrtFileN; x++)
        {
            if (m_aSrtFlArr[x]->eof)
                continue;
            if (k < 0 || (k != x && (RecCmp(&m_aSrtFlArr[k]->rec,
                                            &m_aSrtFlArr[x]->rec) > 0)))
                k = x;
        }

        if (k < 0)
            break; // break while loop if finished with all m_aSrtFlArr

        // Write m_aSrtFlArr[k].rec.dataLn to m_aSrtFlArr[m_iSrtFlArrSz-1].
        if (fwprintf(m_aSrtFlArr[m_iSrtFlArrSz - 1]->fp, L"%S",
                     m_aSrtFlArr[k]->rec.dataLn) < 0)
        {
            sprintf(msg_buf, cErrFileWrite, "SR06a", m_aSrtFlArr[m_iSrtFlArrSz - 1]->name);
            FileIOError(msg_buf);
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
                sprintf(msg_buf, cErrFileRead, "#SR06b", m_aSrtFlArr[k]->name);
                FileIOError(msg_buf);
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

/**
 * @brief Make runs using replacement selection.
 * Methodology: Read up to MAX_ARR_SZ lines of a text file into a
 * buffer array, where each element of the array consists of the
 * text string and an integer key that is copied from the text string;
 * (3) sort the array by the key; (4) find the lowest key in
 *  the buffer that is greater than the last key read, and copy the text
 * string associated with that key from the buffer to a temporary sort file; 
 * (5) read in one new line of text into the buffer and resort the buffer;
 * (7) repeat from step 3 until there are no more keys left that are greater
 * than the last key read; (8) repeat from step 2 until he last sort file
 * has been filled.
 * 
 * @return true if successful, else false if an error occurred.
 */
bool SortRoutines::MakeRuns(void)
{
    int totBufItems = 0; // number of elements in the buffer array
    int pos = 0;         // position of lowest item in buffer array
    uint i;
    BufRecType lowRec;   // lowest item >= highest item in current run
    bool notEndRun;      // signals the end of a run
    string filePathName; // file path and name

    if (!InitTempFiles(0)) // initial all sort files
        return false;      // error occurred

    if (!AddToBuffer(-1, &totBufItems)) // fill entire buffer
        return false;                   // error occurred

    m_iSrtFileN = 0; // init

    DBGPRINT("%s", "Starting main loop in MakeRuns...");

    while (totBufItems > 0) // get data from unsorted input file
    {
        lowRec.key1.clear();
        lowRec.key2.clear();
        lowRec.key3.clear();

        notEndRun = true; // init

        while (m_iSrtFileN < m_iSrtFlArrSz - 1) // add run of items to m_aSrtFlArr
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
                        sprintf(msg_buf, cErrFileWrite, "SR07a", m_aSrtFlArr[m_iSrtFileN]->name);
                        FileIOError(msg_buf);
                        return false;
                    }

                    assert(pos != -1);

                    if (!AddToBuffer(pos, &totBufItems))
                        return false; // error occurred

                } // if (notEndRun)

                if (totBufItems <= 0)
                    break; // there is no more data to sort

            } // while (notEndRun)

            notEndRun = true;
            m_iSrtFileN++; // use next m_aSrtFlArr[srtFileN].fp file

            if (totBufItems <= 0)
                break; // there is no more data to sort

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
                sprintf(msg_buf, cTryRename, "SR07b");
                AddLogEntry(msg_buf);

                if (i > 5)
                {
                    sprintf(msg_buf, cErrFileRen, "SR07c", m_sHoldFile.c_str());
                    FileIOError(msg_buf);
                    return false;
                }
                i++;
            }

            // Open _sort000.dat file.
            if (!(m_aSrtFlArr[0]->fp = fopen(filePathName.c_str(), "r+b")))
            {
                sprintf(msg_buf, cErrFileOpen, "SR07d", filePathName.c_str());
                FileIOError(msg_buf);
                return false;
            }

        } // if (totBufNums > 0)

        m_iSrtFileN = 1; // skip m_aSrtFlArr[0] as that contains the data from first pass

    } // while (totBufNums > 0)

    return true;
}

/**
 * @brief pens the files to be sorted and calls the merge sort subroutines.
 * 
 * @return true if the function was successful, else false if an error occurred.
 */
bool SortRoutines::SortFile()
{
    wchar_t ch;
    uint lineCnt; // count lines to process in current file
    string filePathName;

    // Make sure there was room to allocate the arrays we require.
    if (m_iBufArrSz < MIN_ARR_SZ || m_iSrtFlArrSz < MIN_ARR_SZ)
    {
        sprintf(msg_buf, cNoMemory, "SR08a");
        FileIOError(msg_buf);
        return false;
    }

    m_iLineTot = 0; // start line counter at zero.
    lineCnt = 0;    // count total lines in m_fpInfile

    // Open file we wish to sort.
    filePathName = "" + m_sUserFile;
    //cout << "Attempting to open " + filePathName + "\n";

    if (!(m_fpInfile = fopen(filePathName.c_str(), "r+b")))
    {
        sprintf(msg_buf, cErrFileOpen, "SR08b", m_sUserFile.c_str());
        FileIOError(msg_buf);
        return false;
    }

    do // count the total lines in the file to sort
    {
        ch = fgetwc(m_fpInfile);
        if (ch == CHR_LF)
            lineCnt++;
    } while (ch != WEOF);

    // If nothing to sort in infile then stop.
    if (lineCnt == 0)
    {
        cout << "Error...no lines read\n";
        fclose(m_fpInfile);
        m_fpInfile = NULL;
        return true;
    }

    DBGPRINT("Sorting file: %s", filePathName.c_str());

    // Go back to first line of m_fpInfile
    rewind(m_fpInfile);

    // If first line of file is a header then hold onto it
    if (m_bSkipFirstLn)
    {
        fgetws(m_bFirstLn, BUFFER_SZ, m_fpInfile);
        lineCnt -= 1; // subtract 1 line for header
    }

    // Init the progress bar
    ShowProgress(true, lineCnt);

    // Sorting the file.
    if (!MakeRuns())
        return false; // error occurred

#ifdef _DEBUG
    OrgLineCnt = lineCnt;
    CheckSort();
#endif

    sprintf(msg_buf, "Total Lines Read: = %d", m_iLineTot);
    AddLogEntry(msg_buf);
    DBGPRINT("%s", msg_buf);
    
    // Close m_fpInfile file.
    fclose(m_fpInfile);
    m_fpInfile = NULL;

    if (m_bSkipFirstLn)
    {
        FILE *fP;
        FILE *fPOutfile;
        wchar_t dataLn[BUFFER_SZ + 1];

        DBGPRINT("%s", "Adding header to file...");

        fP = fopen(m_sHoldFile.c_str(), "r+b");

        fPOutfile = fopen(m_sOutfile.c_str(), "w+b");

        // FIRST, write header line.
        if (fwprintf(fPOutfile, L"%S", m_bFirstLn) < 0)
        {
            sprintf(msg_buf, cErrFileWrite, "SR09a", m_sOutfile.c_str());
            fclose(fP);
            fclose(fPOutfile);
            FileIOError(msg_buf);
            return false;
        }
    
           //    SECOND,    write all lines from m_sHoldFile.
        while (fgetws(dataLn, BUFFER_SZ, fP))
        {
            if (fwprintf(fPOutfile, L"%S", dataLn) < 0)
            {
                sprintf(msg_buf, cErrFileWrite, "SR09b", m_sOutfile.c_str());
                fclose(fP);
                fclose(fPOutfile);
                FileIOError(msg_buf);
                return false;
            }
        }
        fclose(fP);
        fclose(fPOutfile);
        remove(m_sHoldFile.c_str());

    } // if (m_bSkipFirstLn)
    else
    {
        // Rename _holder.dat file to m_fpInfile's file.
        if (rename(m_sHoldFile.c_str(), m_sOutfile.c_str()))
        {
            sprintf(msg_buf, cErrFileRen, "SR08c", m_sHoldFile.c_str());
            FileIOError(msg_buf);
            return false;
        }
    }

    return true;
}

/**
 * @brief Updates the progress bar in the console out
 * 
 * @param setCnt This should be true if initializing progress bar, else false.
 * @param count The number of times the progress bar will be updated.
 * 
 * @return Void.
 */
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
        float progPct = float(count / m_fProgress);

        std::cout << "[";
        int pos = barWidth * progPct;
        for (int i = 0; i < barWidth; ++i)
        {
            if (i < pos)
                std::cout << "=";
            else if (i == pos)
                std::cout << ">";
            else
                std::cout << " ";
        }
        std::cout << "] " << int(progPct * 100.0) << " %\r";
        std::cout.flush();
    }
}

#ifdef _DEBUG
/**
 * @brief Checks the last sorted file to make sure it was sorted correctly. 
 * Makes sure total lines in final file is same as in original file. 
 * FOR DEBUGING PURPOSES ONLY.  Do no compile this in final code.
 * 
 * @return Void.
 */
void SortRoutines::CheckSort(void)
{
    BufRecType rec1;
    BufRecType rec2;
    FILE *fP;
    uint chkLineCnt = 0;

    std::cout << "\n";
    DBGPRINT("%s", "Checking that data was sorted correctly.");

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

    DBGPRINT("%s", "Data was sorted correctly.");

    fclose(fP);
}
#endif
