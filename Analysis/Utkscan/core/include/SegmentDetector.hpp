/** @file SegmentDetector.hpp
 * @brief  Basic SegmentDetector Class (Similar to BarDetector, but without the HighRes Timing stuff). Originally written as MtasSegment but now generalized.
 * @authors T.T. King, T. Ruland, B.C. Rasco
 * @date 03/25/2022
 */
#ifndef PAASS_SegmentDetector_H
#define PAASS_SegmentDetector_H

#include "RawEvent.hpp"
#include "Globals.hpp"

class SegmentDetector {
	public:
		/** Constructor */
		SegmentDetector() {
			segFront_ = nullptr;
			segBack_ = nullptr;
		};
		/** Destructor */
		~SegmentDetector() = default;

		virtual ChanEvent* GetSegFront() {
			return segFront_;
		};
		virtual ChanEvent* GetSegBack() {
			return segBack_;
		};

		virtual void SetSegFront(ChanEvent* a) {
			segFront_ = a;
		};
		virtual void SetSegBack(ChanEvent* a) {
			segBack_ = a;
		};

		virtual void SetPixieRev(string &a) {
			PixieRev = a;
		};

		virtual string GetPixieRev() noexcept{
			return PixieRev;
		};

		virtual bool IsValidSegment() const  noexcept{ 
			return segBack_ != nullptr and segFront_ != nullptr;
		};

		virtual double GetSegAvgEnergy() const  noexcept{
			if (IsValidSegment()) {
				return (segFront_->GetCalibratedEnergy() + segBack_->GetCalibratedEnergy()) / 2.0;
			} else {
				return 0.0;
			}
		}

		virtual double GetSegQDC(unsigned int erg,unsigned int bkg) const{
			if (IsValidSegment()) {
				double fQDC = segFront_->GetQdc().at(erg) - segFront_->GetQdc().at(bkg);
				double bQDC = segBack_->GetQdc().at(erg) - segBack_->GetQdc().at(bkg);
				return ((fQDC + bQDC) /2.0);
			} else {
				return 0.0;
			}

		}

		virtual double GetSegQDC() const {
			if (IsValidSegment()) {
				double fQDC = segFront_->GetQdc().at(2) - segFront_->GetQdc().at(0);
				double bQDC = segBack_->GetQdc().at(2) - segBack_->GetQdc().at(0);
				return ((fQDC + bQDC) /2.0);
			} else {
				return 0.0;
			}
		}

		virtual double GetSegTimeAvgInNs() const {
			if (not IsValidSegment()) {
				throw "Unable to get SegmentDetector time average. Not valid SegmentDetector";
			}
			double clockInSeconds;
			if (PixieRev == "F") {
				clockInSeconds = Globals::get()->GetClockInSeconds(segFront_->GetChanID().GetModFreq());
			} else {
				clockInSeconds = Globals::get()->GetClockInSeconds();
			}

			return ( ((segFront_->GetTimeSansCfd() + segBack_->GetTimeSansCfd()) / 2.0)  * clockInSeconds * 1.0e9);
		}

		virtual double GetSegPosition() const {
			if( IsValidSegment() ){
				return (segFront_->GetCalibratedEnergy() - segBack_->GetCalibratedEnergy()) / (segFront_->GetCalibratedEnergy() + segBack_->GetCalibratedEnergy());
			}else{
				return -1.0e12;
			}
		}

		virtual double GetSegTdiffInNs() const {
			if( IsValidSegment() ){
				double clockInSeconds;
				if (PixieRev == "F") {
					clockInSeconds = Globals::get()->GetClockInSeconds(segFront_->GetChanID().GetModFreq());
				} else {
					clockInSeconds = Globals::get()->GetClockInSeconds();
				}

				return (segFront_->GetTimeSansCfd() - segBack_->GetTimeSansCfd()) * clockInSeconds * 1.0e9;
			}else{
				return -1.0e12;
			}
		}

		virtual double GetFrontEnergy() const {
			if (segFront_ == nullptr) {
				return 0.0;
			} else {
				return segFront_->GetCalibratedEnergy();
			}
		}

		virtual double GetBackEnergy() const {
			if (segBack_ == nullptr) {
				return 0.0;
			} else {
				return segBack_->GetCalibratedEnergy();
			}
		}

	protected:
		ChanEvent* segFront_;
		ChanEvent* segBack_;
		string PixieRev;
};
#endif
