/** @file MtasProcessor.hpp
 * @brief  Basic MtasProcessor for MTAS at FRIB
 * @authors T.T. King, T. Ruland, B.C. Rasco
 * @date 03/25/2022
 */
#ifndef PAASS_MtasProcessor_H
#define PAASS_MtasProcessor_H

#include "EventProcessor.hpp"
#include "PaassRootStruct.hpp"
#include "RawEvent.hpp"
#include "Globals.hpp"

class MtasSegment {
	public: 
		/** Constructor */
		MtasSegment(){
			segFront_ = nullptr;
			segBack_ = nullptr;
			gMtasSegID_ = -1;

		};
		/** Destructor */
		~MtasSegment() = default;
		bool IsValidSegment() const { return segBack_ != nullptr and segFront_ != nullptr; }
		double GetSegmentPosition() const{
			return (segFront_->GetCalibratedEnergy() - segBack_->GetCalibratedEnergy())/(segFront_->GetCalibratedEnergy() + segBack_->GetCalibratedEnergy());
		}
				
		double GetSegmentAverageEnergy() const { 
			if( IsValidSegment() ){
				return (segFront_->GetCalibratedEnergy() + segBack_->GetCalibratedEnergy())/2.0; 
			}else{
				return 0.0;
			}
		}
		double GetSegmentTdiffInNS() const { 
			if( not IsValidSegment() ){
				throw "Unable to get Tdiff. Not valid Segment";
			}
			double clockInSeconds;
			if (PixieRev == "F"){
				clockInSeconds = Globals::get()->GetClockInSeconds(segFront_->GetChanID().GetModFreq());
			} else {
				clockInSeconds = Globals::get()->GetClockInSeconds();
			}

			return (segFront_->GetTimeSansCfd() - segBack_->GetTimeSansCfd()) * clockInSeconds * 1.0e9;
		}
		double GetFrontEnergy() const{
			if( segFront_ == nullptr ){
				return 0.0;
			}else{
				return segFront_->GetCalibratedEnergy();
			}
		}
		double GetBackEnergy() const{
			if( segBack_ == nullptr ){
				return 0.0;
			}else{
				return segBack_->GetCalibratedEnergy();
			}
		}

	public:
		int gMtasSegID_;
		ChanEvent* segFront_;
		ChanEvent* segBack_ ;
		std::string PixieRev;
};

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
		bool PreProcess(RawEvent &event);

		/** Process the event
		 * \param [in] event : the event to process
		 * \return true if successful
		 */
		bool Process(RawEvent &event);


		/** Declares the plots for the class */
		void DeclarePlots(void);

	private:
		processor_struct::MTAS Mtasstruct;  //!<Root Struct
		std::string PixieRev; //! pixie revision

};

#endif  //PAASS_MtasProcessor_H
