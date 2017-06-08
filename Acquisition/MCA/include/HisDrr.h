/*
 * Copyright Krzysztof Miernik 2012
 * k.a.miernik@gmail.com 
 *
 * Distributed under GNU General Public Licence v3
 */

#ifndef HISDRR_H
#define HISDRR_H

#include <vector>
#include <string>
#include <fstream>
#include "DrrBlock.h"

/**
 * This structure is used for creating new his and drr files.
 * It consist of fields present in a input file and resembles the
 * structure of DrrHisRecord.
 */
struct SimpleDrrBlock {
    /** Histogram id. */
    int hisID;

    /** Number of half-words (2 bytes) per channel */
    short halfWords;

    /** Histogram lengths (X, Y) */
    short scaled[2];

    /** Histogram title.*/
    string title;
};

/**
 * Class for handling histogram files. Class opens drrFile and loads histogram
 * information. On request can return specific histogram data points or 
 * histogram information. There are some cabalilites of creating a new file,
 * putting data into specific histogram.
 */
class HisDrr {
public:
    /** Constructor taking fstreams. User is responsible of opening files, 
     * and passing fstreams creating them on heap (new).
     * Ones is passed the HisDrr takes the ownership to pointers.
     * Dtor will close files and delete pointers.*/
    HisDrr(fstream *drr, fstream *his);

    /** Constructor taking names and opening fstreams. */
    HisDrr(const string &drr, const string &his);

    /** Constructor creating and opening new his and drr using definition from input file. */
    HisDrr(const string &drr, const string &his, const string &input);

    /** Dtor, closing files and deleting memory. */
    virtual ~HisDrr() {
        drrFile->close();
        hisFile->close();
        delete drrFile;
        delete hisFile;
    }

    /** Returns specified histogram data. */
    virtual void getHistogram(vector<unsigned int> &rtn, int id);

    /** Returns drr data on specified histogram. */
    virtual DrrHisRecordExtended getHistogramInfo(int id) const;

    /** Returns histograms id's list. */
    virtual void getHisList(vector<int> &r);

    /** Zeroes data for a given histogram. */
    virtual void zeroHistogram(int id);

    /** Replaces in histogram 'id' point 'i' by 'value'. 4-bytes long word version. */
    virtual void setValue(const int id, unsigned pos, unsigned value);

    /** Replaces in histogram 'id' point 'i' by 'value'. The 2-bytes long word version. */
    virtual void setValue(const int id, unsigned pos, unsigned short value);

    /** Replaces histogram id values by ones given in a vector. The 4-bytes long word version.  */
    virtual void setValue(const int id, vector<unsigned> &value);

    /** Replaces histogram id values by ones given in a vector. The 2-bytes long word version.  */
    virtual void setValue(const int id, vector<unsigned short> &value);

private:
    /** Vector holding all the histogram info read from drr file. */
    vector<DrrHisRecordExtended> hisList;

    /** Pointer to drr file containing information about his structure. */
    fstream *drrFile;

    /** Pointer to his file containg data. */
    fstream *hisFile;

    /** Reads block of data from drr file. */
    void readBlock(drrBlock *block);

    /** Function loading .drr file and filling in spectrum vector. */
    void loadDrr();

};

#endif
