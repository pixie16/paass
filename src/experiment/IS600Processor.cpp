/** \file IS600Processor.cpp
 * \brief A class to process data from ISOLDE 599 and 600 experiments using
 * VANDLE.
 *
 *\author S. V. Paulauskas
 *\date July 14, 2015
 */
#include <fstream>
#include <iostream>

#include <cmath>

#include "BarBuilder.hpp"
#include "DammPlotIds.hpp"
#include "GetArguments.hpp"
#include "Globals.hpp"
#include "RawEvent.hpp"
#include "TimingMapBuilder.hpp"
#include "IS600Processor.hpp"

namespace dammIds {
    namespace vandle {
        const unsigned int DEBUGGING_OFFSET = 70;
        const int DD_DEBUGGING0  = 0+DEBUGGING_OFFSET;
        const int DD_DEBUGGING1  = 1+DEBUGGING_OFFSET;
        const int DD_DEBUGGING2  = 2+DEBUGGING_OFFSET;
        const int DD_DEBUGGING3  = 3+DEBUGGING_OFFSET;
        const int DD_DEBUGGING4  = 4+DEBUGGING_OFFSET;
        const int DD_DEBUGGING5  = 5+DEBUGGING_OFFSET;
        const int DD_DEBUGGING6  = 6+DEBUGGING_OFFSET;
        const int DD_DEBUGGING7  = 7+DEBUGGING_OFFSET;
        const int DD_DEBUGGING8  = 8+DEBUGGING_OFFSET;
        const int DD_DEBUGGING9  = 9+DEBUGGING_OFFSET;
        const int DD_DEBUGGING10  = 10+DEBUGGING_OFFSET;
        const int DD_DEBUGGING11  = 11+DEBUGGING_OFFSET;
        const int DD_DEBUGGING12  = 12+DEBUGGING_OFFSET;
    }
}//namespace dammIds

using namespace std;
using namespace dammIds::vandle;

void IS600Processor::DeclarePlots(void) {
    VandleProcessor::DeclarePlots();
    DeclareHistogram2D(DD_DEBUGGING0, SC, SD, "QDC CTof- No Tape Move");
    DeclareHistogram2D(DD_DEBUGGING1, SC, SD, "QDC ToF Ungated");
    DeclareHistogram2D(DD_DEBUGGING2, SC, SC, "Cor ToF vs. Gamma E");
    DeclareHistogram2D(DD_DEBUGGING4, SC, SC, "QDC vs Cor Tof Mult1");
    DeclareHistogram1D(DD_DEBUGGING3, S7, "Vandle Multiplicity");
    DeclareHistogram2D(DD_DEBUGGING5, SC, SC, "Mult2 Sym Plot Tof ");
}

IS600Processor::IS600Processor(const std::vector<std::string> &typeList,
    const double &res, const double &offset, const double &numStarts) :
    VandleProcessor(typeList,res,offset,numStarts) {
    associatedTypes.insert("vandle");

    char hisFileName[32];
    GetArgument(1, hisFileName, 32);
    string temp = hisFileName;
    temp = temp.substr(0, temp.find_first_of(" "));
    stringstream name;
    name << temp;
    fileName_ = name.str();
    fileNames_.push_back(fileName_ + "-tof.dat");
    fileNames_.push_back(fileName_ + "-tof-02Plus.dat");
    fileNames_.push_back(fileName_ + "-tof-04Plus.dat");
}

bool IS600Processor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return(false);
    if(!VandleProcessor::Process(event))
        return(false);

    int bananaNum = 2;
    bool hasMultOne = bars_.size() == 1;
    bool hasMultTwo = bars_.size() == 2;
    bool isFirst = true;
    bool isTapeMoving = TreeCorrelator::get()->place("TapeMove")->status();

    plot(DD_DEBUGGING3, bars_.size());

    for (BarMap::iterator it = bars_.begin(); it !=  bars_.end(); it++) {
        TimingDefs::TimingIdentifier barId = (*it).first;
        BarDetector bar = (*it).second;

        if(!bar.GetHasEvent() || bar.GetType() == "small")
            continue;

        unsigned int barLoc = barId.first;
        TimingCalibration cal = bar.GetCalibration();

        for(BarMap::iterator itStart = barStarts_.begin();
        itStart != barStarts_.end(); itStart++) {
            unsigned int startLoc = (*itStart).first.first;
            unsigned int barPlusStartLoc = barLoc*numStarts_ + startLoc;

            BarDetector start = (*itStart).second;

            double tofOffset = cal.GetTofOffset(startLoc);
            double tof = bar.GetWalkCorTimeAve() -
                start.GetWalkCorTimeAve() + tofOffset;

            double corTof =
                CorrectTOF(tof, bar.GetFlightPath(), cal.GetZ0());

	    bool notPrompt = corTof > 45.;
	    bool inPeel = histo.BananaTest(bananaNum,
                                           corTof*plotMult_+plotOffset_,
                                           bar.GetQdc());
	    bool isLowStart = start.GetQdc() < 300;	    

	    plot(DD_DEBUGGING1, tof*plotMult_+plotOffset_, bar.GetQdc());
	    if(!isTapeMoving && !isLowStart)
	      plot(DD_DEBUGGING0, corTof*plotMult_+plotOffset_,bar.GetQdc());
	    if(hasMultOne)
	      plot(DD_DEBUGGING4, corTof*plotMult_+plotOffset_, bar.GetQdc());

	    ///Starting to look for 2n coincidences in VANDLE
	    BarMap::iterator itTemp = it;
	    itTemp++;
	    for (BarMap::iterator it2 = itTemp; it2 !=  bars_.end(); it2++) {
	      TimingDefs::TimingIdentifier barId2 = (*it2).first;
	      BarDetector bar2 = (*it2).second;

	      if(!bar.GetHasEvent())
		continue;
	      
	      unsigned int barLoc2 = barId2.first;

	      bool isAdjacent = abs((int)barLoc2 - (int)barLoc) < 1;

	      TimingCalibration cal2 = bar2.GetCalibration();
	      
	      double tofOffset2 = cal2.GetTofOffset(startLoc);
	      double tof2 = bar2.GetWalkCorTimeAve() -
                start.GetWalkCorTimeAve() + tofOffset2;
	      
	      double corTof2 =
                CorrectTOF(tof2, bar2.GetFlightPath(), cal2.GetZ0());
	      
	      bool inPeel2 = histo.BananaTest(bananaNum,
					     corTof2*plotMult_+plotOffset_,
					     bar2.GetQdc());
	      
	      if(hasMultTwo && inPeel && inPeel2 && !isAdjacent) {
		plot(DD_DEBUGGING5, corTof*plotMult_+plotOffset_, corTof2*plotMult_+plotOffset_);
		plot(DD_DEBUGGING5, corTof2*plotMult_+plotOffset_, corTof*plotMult_+plotOffset_);
	      }
	    }
	    ///End 2n coincidence routine

	    if (geSummary_ && notPrompt && hasMultOne) {
                if (geSummary_->GetMult() > 0) {
                    const vector<ChanEvent *> &geList = geSummary_->GetList();
                    for (vector<ChanEvent *>::const_iterator itGe = geList.begin();
                        itGe != geList.end(); itGe++) {
                        double calEnergy = (*itGe)->GetCalEnergy();
			plot(DD_DEBUGGING2, calEnergy, corTof*plotMult_+plotOffset_);
                    }
                } else {
		  //plot(DD_TQDCAVEVSTOF_VETO+histTypeOffset, tof, bar.GetQdc());
		  //plot(DD_TOFBARS_VETO+histTypeOffset, tof, barPlusStartLoc);
                }
            }
        } // for(TimingMap::iterator itStart
    } //(BarMap::iterator itBar
    /*

    for (BarMap::iterator it = bars_.begin(); it !=  bars_.end(); it++) {
        TimingDefs::TimingIdentifier barId = (*it).first;
        BarDetector bar = (*it).second;
        if(!bar.GetHasEvent())
            continue;

        unsigned int barLoc = barId.first;
        TimingCalibration cal = bar.GetCalibration();

        int bananaNum = 1;

        for(TimingMap::iterator itStart = barStarts_.begin();
        itStart != barStarts_.end(); itStart++) {
            BarDetector start = (*itStart).second;
            if(!start.GetIsValidData())
                continue;

            unsigned int startLoc = (*itStart).first.first;
            unsigned int barPlusStartLoc = numStarts_*barLoc+startLoc;

	    double tofOffset = cal.GetTofOffset(startLoc);
            double tof = bar.GetWalkCorTimeAve() -
                start.GetWalkCorrectedTime() + tofOffset;
            double corTof =
                VandleProcessor::CorrectTOF(tof, bar.GetFlightPath(), cal.GetZ0());
            bool inPeel = histo.BananaTest(bananaNum,
                                           corTof*plotMult_+plotOffset_,
                                           bar.GetQdc());

            double cycleTime = TreeCorrelator::get()->place("Cycle")->last().time;
            cycleTime *= (Globals::get()->clockInSeconds()*1.e9);

            double decayTime = (bar.GetTimeAverage() - cycleTime)/0.01;

            plot(DD_DEBUGGING10, corTof*plotMult_+plotOffset_, decayTime*1.e-9);
            if(inPeel)
                plot(DD_DEBUGGING12, corTof*plotMult_+plotOffset_, decayTime*1.e-9);

            ofstream gammaGated;
            if(geSummary_->GetMult() > 0 && inPeel) {
                const vector<ChanEvent *> &geList = geSummary_->GetList();
                for(vector<ChanEvent *>::const_iterator itGe = geList.begin();
                itGe != geList.end(); itGe++) {
                    double calEnergy = (*itGe)->GetCalEnergy();
                    plot(DD_DEBUGGING11, calEnergy, decayTime * 1e-9);
                    if(calEnergy >= 595 && calEnergy <= 603)
		      plot(DD_DEBUGGING4, corTof * plotMult_ + plotOffset_, bar.GetQdc());
		    if(calEnergy >= 692 && calEnergy <= 704)
		      plot(DD_DEBUGGING5, corTof * plotMult_ + plotOffset_, bar.GetQdc());
                }// for(vector<ChanEvent *>::const_iterator
                gammaGated.close();
            }//geSummary_->GetMult() > 0
        }//loop over starts
    }//loop over bars
    */
    EndProcess();
    return(true);
}
