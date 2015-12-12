/** \file PspmtProcessor.hpp
 *  \brief A processor to handle pixelated PMTs
 *  \author Shintaro Go
 *  \date November 16, 2015
 */

#ifndef __PSPMTPROCESSOR_HPP__
#define __PSPMTPROCESSOR_HPP__

#include "RawEvent.hpp"
#include "EventProcessor.hpp"

class PspmtProcessor : public EventProcessor {
public:
    /** Default Constructor */
    PspmtProcessor(void);
    /** Default Destructor */
    ~PspmtProcessor() {};
    
    /** Declare the plots used in the analysis */
    virtual void DeclarePlots(void);
    /** Preprocess the VANDLE data
     * \param [in] event : the event to preprocess
     * \return true if successful */
    virtual bool PreProcess(RawEvent &event);
    /** Process the event for VANDLE stuff
     * \param [in] event : the event to process
     * \return Returns true if the processing was successful */
    virtual bool Process(RawEvent &event);
private:
    struct PspmtData {
        void Clear(void);
    } data_;
};
#endif // __PSPMTPROCESSOR_HPP__
