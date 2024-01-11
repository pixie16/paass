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

class MtasProcessor : public EventProcessor {
	public:
		/**Constructor */
		MtasProcessor(bool,double,double,double,double);

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
		processor_struct::MTASTOTALS MtasTotalsstruct;
		std::string PixieRevision;               //! pixie revision

		std::string MTASMode;
		
		std::vector<MtasSegment> MtasSegVec;
		double MTASTotal;
		double MTASCenter;
		double MTASInner;
		double MTASMiddle;
		double MTASOuter;
		double MTASFirstTime;
		
		bool HasBetaInfo;
		double BetaMin;
		double BetaMax;
		double IonMin;
		double IonMax;

		bool IsPrevBetaTriggered;
		double PrevBetaTimeStamp;
		double PrevBetaEnergy;
		
		bool IsPrevIonTriggered;
		double PrevIonTimeStamp;
		double PrevIonEnergy;
};


#endif  //PAASS_MtasProcessor_H
