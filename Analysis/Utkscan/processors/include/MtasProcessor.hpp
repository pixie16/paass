/** @file MtasProcessor.hpp
 * @brief  Basic MtasProcessor for MTAS at FRIB
 * @authors T.T. King, T. Ruland, B.C. Rasco
 * @date 03/25/2022
 */
#ifndef PAASS_MtasProcessor_H
#define PAASS_MtasProcessor_H

#include "EventProcessor.hpp"
#include "PaassRootStruct.hpp"
#include "SegmentDetector.hpp"
#include "RawEvent.hpp"
#include "Globals.hpp"


class MtasProcessor : public EventProcessor {
   public:
    /**Constructor */
    MtasProcessor();

    /** Deconstructor */
    ~MtasProcessor() = default;

    /** Preprocess the event
		 * \param [in] event : the event to preprocess
		 * \return true if successful
		 */
    bool PreProcess(RawEvent& event);

    /** Process the event
		 * \param [in] event : the event to process
		 * \return true if successful
		 */
    bool Process(RawEvent& event);

    /** Declares the plots for the class */
    void DeclarePlots(void);

   private:
    processor_struct::MTAS Mtasstruct;  //!<Root Struct
    std::string PixieRevision;               //! pixie revision
};

class MtasSegment : public SegmentDetector {
   public:
    MtasSegment() : SegmentDetector() {
        gMtasSegID_ = -1;
        RingSegNum_ = -1;  // ! per ring SegmentDetector number (1-6)
    };

    ~MtasSegment() = default;
    
	int gMtasSegID_;
    int RingSegNum_;
    string segRing_;
};

#endif  //PAASS_MtasProcessor_H
