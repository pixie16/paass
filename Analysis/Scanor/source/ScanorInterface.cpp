///@file ScanorInterface.cpp
///@authors C. R. Thornsberry and S. V. Paulauskas
///@date September 16, 2016

#include <iostream>
#include <sstream>
#include <vector>

#include <cstring>

#include "ScanorInterface.hpp"

#define TOTALREAD 1000000
#define EXTERNAL_FIFO_LENGTH 131072
#define U_DELIMITER 0xFFFFFFFF

//Actually define the instance of the ScanorInterface
ScanorInterface* ScanorInterface::instance_ = NULL;

///The get method returns the only instance of ScanorInterface that will ever
/// be created. This is the core feature that will make this a singleton.
ScanorInterface* ScanorInterface::get() {
    if (!instance_)
        instance_ = new ScanorInterface();
    return instance_;
}

/** \brief inserts a delimiter in between individual module data and at end of
 * buffer. Data is then passed to hissub_sec() for processing.
 * \param [in] data : the data to parse
 * \param [in] nWords : the length of the data
 * \param [in] maxWords : the maximum words to get
 * \return true if successful */
bool ScanorInterface::MakeModuleData(const uint32_t *data, unsigned long nWords,
                                        unsigned int maxWords) {
    const unsigned int maxVsn = 14; // no more than 14 pixie modules per crate

    unsigned int inWords = 0, outWords = 0;

    static uint32_t modData[TOTALREAD];

    do {
	uint32_t lenRec = data[inWords];
        uint32_t vsn    = data[inWords+1];
	/* Check sanity of record length and vsn*/
	if(lenRec > maxWords || (vsn > maxVsn && vsn != 9999 && vsn != 1000)) {
#ifdef VERBOSE
	    std::cout << "SANITY CHECK FAILED: lenRec = " << lenRec
		 << ", vsn = " << vsn << ", inWords = " << inWords
		 << " of " << nWords << ", outWords = " << outWords << std::endl;
#endif
	    return false;
	}

	/*Extract the data from TotData and place into ModData*/
	memcpy(&modData[outWords], &data[inWords], lenRec * sizeof(uint32_t));
	inWords  += lenRec;
	outWords += lenRec;
        
    } while (inWords < nWords);

    if(nWords > TOTALREAD || inWords > TOTALREAD || outWords > TOTALREAD ) {
        std::stringstream ess;
        ess << "Values of nn - " << nWords << " nk - "<< inWords
            << " mm - " << outWords << " TOTALREAD - " << TOTALREAD << std::endl;
        ess << "One of the variables named nn, nk, or mm"
            << "have exceeded the value of TOTALREAD. The value of "
            << "TOTALREAD MUST NEVER exceed 1000000 for correct "
            << "opertation of code between 32-bit and 64-bit architecture "
            << "Either these variables have not been zeroed correctly or "
            << "the poll program controlling pixie16 is trying to send too "
            << "much data at once.";
        //throw GeneralException(ess.str());
        return false;
    }

	// Process the data.
	unpacker_->ReadSpill(modData, outWords);

    return true;
}

/** \fn extern "C" void hissub_(unsigned short *ibuf[],unsigned short *nhw)
 * \brief interface between scan and C++
 *
 * In a typical experiment, Pixie16 reads data from all modules when one module
 * has hit the maximum number of events which is programmed during experimental
 * setup.  This spill of data is then broken into smaller chunks for
 * transmitting across the network.  The hissub_ function takes the chunks
 * and reconstructs the spill.
 *
 * Summarizing the terminology:
 *  - Spill  - a readout of all Pixie16 modules
 *  - Buffer - the data from a specific Pixie16 module
 *  - Chunk  - the packet transferred from Pixie16 to the acquisition
 *
 * The hissub_ function is passed a pointer to an array with data (ibuf) and
 * the number of half words (nhw) contained in it.  This function is used with
 * the new Pixie16 readout (which is the default).  If the old Pixie16 readout
 * is used, the code should be recompiled without the newreadout flag in which
 * case this particular function is not used.
 * \param [in] ibuf : the array with the data
 * \param [in] nhw : the number of half words contained in the data buffer
*/
void ScanorInterface::Hissub(unsigned short **sbuf, unsigned short *nhw) {
    const unsigned int maxChunks = 200;

    static uint32_t totData[TOTALREAD];
    // keep track of the number of bad spills
    static unsigned int spillInvalidCount = 0, spillValidCount = 0;
    static bool firstTime = true;
    // might take a few entries into this function to get all the buffers in a spill
    static unsigned int bufInSpill = 0;
    static unsigned int dataWords = 0;

    /*Assign ibuf variable to local variable for use in function */
    uint32_t *buf=(uint32_t*)sbuf;

    /* Initialize variables */
    unsigned long totWords=0;
    uint32_t nWords=buf[0] / 4;
    uint32_t totBuf=buf[1];
    uint32_t bufNum=buf[2];
    static unsigned int lastBuf = U_DELIMITER;
    unsigned int maxWords = EXTERNAL_FIFO_LENGTH;

    // Check to make sure the number of buffers is not excessively large
    if (totBuf > maxChunks) {
        std::cout << "LARGE TOTNUM = " << bufNum << std::endl;
        return;
    }

    /* Find a starting point in a file immediately following the 5-word buffer
         which indicates the end of a spill
     */
    if(bufNum != 0 && firstTime) {
        do {
            if (buf[totWords] == U_DELIMITER) {
                std::cout << "  -1 DELIMITER, "
                    << buf[totWords] << buf[totWords + 1] << std::endl;
                return;
            }
            nWords = buf[totWords] / 4;
            totBuf = buf[totWords+1];
            bufNum = buf[totWords+2];
            totWords += nWords+1;
            std::cout << "SKIP " << bufNum << " of " << totBuf << std::endl;
        } while(nWords != 5);
    }
    firstTime = false;

    do {
        do {
            /*Determine the number of words, total number of buffers, and
            current buffer number at this point in the chunk.
            Note: the total number of buffers is repeated for each
            buffer in the chunk */
            if (buf[totWords] == U_DELIMITER) return;

            nWords = buf[totWords] / 4;
            bufNum = buf[totWords+2];
            // read total number of buffers later after we check if the last spill was good
            if (lastBuf != U_DELIMITER && bufNum != lastBuf + 1) {
#ifdef VERBOSE
            std::cout << "Buffer skipped, Last: " << lastBuf << " of " << totBuf
                << " buffers read -- Now: " << bufNum << std::endl;
#endif
            // if we are only missing the vsn 9999 terminator, reconstruct it
            if (lastBuf + 2 == totBuf && bufInSpill == totBuf - 1) {
#ifdef VERBOSE
                std::cout << "  Reconstructing final buffer "
                     << lastBuf + 1 << "." << std::endl;
#endif
                totData[dataWords++] = 2;
                totData[dataWords++] = 9999;

                MakeModuleData(totData, dataWords, maxWords);
                spillValidCount++;
                bufInSpill = 0; dataWords = 0; lastBuf = -1;
            } else if (bufNum == 0) {
#ifdef VERBOSE
                std::cout << "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"
                << "  INCOMPLETE BUFFER " << spillInvalidCount
                << "\n  " << spillValidCount << " valid spills so far."
                << " Starting fresh spill." << std::endl;
#endif
                spillInvalidCount++;
                // throw away previous collected data and start fresh
                bufInSpill = 0; dataWords = 0; lastBuf = -1;
            }
            } // check that the chunks are in order
            // update the total chunks only after the sanity checks above
            totBuf = buf[totWords+1];
            if (totBuf > maxChunks) {
#ifdef VERBOSE
            std::cout << "EEEEE LOST DATA: Total buffers = " << totBuf
                <<  ", word count = " << nWords << std::endl;
#endif
            return;
            }
            if (bufNum > totBuf - 1) {
#ifdef VERBOSE
            std::cout << "EEEEEEE LOST DATA: Buffer number " << bufNum
                << " of total buffers " << totBuf << std::endl;
#endif
            return;
            }
            lastBuf = bufNum;

            /* Increment the number of buffers in a spill*/
            bufInSpill++;
            if(nWords == 0) {
#ifdef VERBOSE
            std::cout << "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE NWORDS 0" << std::endl;
#endif
            return;
            }

            /* Extract this buffer information into the TotData array*/
            memcpy(&totData[dataWords], &buf[totWords+3],
                (nWords - 3) * sizeof(int));
            dataWords += nWords - 3;

            // Increment location in file
            // one extra word to pass over "-1" delimiter signalling end of buffer
            totWords += nWords+1;
            if (bufNum == totBuf - 1 && nWords != 5) {
            std::cout << "Strange final buffer " << bufNum << " of " << totBuf
                << " with " << nWords << " words" << std::endl;
            }
            if (nWords == 5 && bufNum != totBuf - 1) {
#ifdef VERBOSE
            std::cout << "Five word buffer " << bufNum << " of " << totBuf
                << " WORDS: "
                << std::hex << buf[3] << " " << buf[4] << std::dec << std::endl;
#endif
            }
        } while(nWords != 5 || bufNum != totBuf - 1);
        /* reached the end of a spill when nwords = 5 and last chunk in spill */

        /* make sure we retrieved all the chunks of the spill */
        if (bufInSpill != totBuf) {
#ifdef VERBOSE
            std::cout << "EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE  INCOMPLETE BUFFER "
            << spillInvalidCount
            << "\n I/B [  " << bufInSpill << " of " << totBuf << " : pos " << totWords
            << "    " << spillValidCount << " total spills"
            << "\n| " << std::hex << buf[0] << " " << buf[1] << "  "
            << buf[2] << " " << buf[3]
            << "\n| " << std::dec << buf[totWords] << " " << buf[totWords+1] << "  "
            << buf[totWords+2] << " " << buf[totWords+3] << std::endl;
#endif
            spillInvalidCount++;
        } else {
            spillValidCount++;
            MakeModuleData(totData, dataWords, maxWords);
        } // else the number of buffers is complete
        dataWords = 0; bufInSpill = 0; lastBuf = -1; // reset the number of buffers recorded
    } while (totWords < nhw[0] / 2);
}