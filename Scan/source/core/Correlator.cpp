/** \file Correlator.cpp
 *  \brief Correlates events
 *
 *  The correlator class keeps track of where and when implantation and decay
 *  events have occurred, and then correlates each decay with its corresponding
 *  implant. A decay will only be validate if it occurred close enough in time
 *  to the implant, and the implant was well separated in time with regard to
 *  all other implants at the same location
 *
 *  This file is derived from previous "correlator.cpp"
 *
 *  \author David Miller
 *  \date April 2010
 */

#include <iomanip>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <cmath>
#include <ctime>

#include "DammPlotIds.hpp"
#include "Globals.hpp"
#include "DetectorDriver.hpp"
#include "LogicProcessor.hpp"
#include "RawEvent.hpp"
#include "Correlator.hpp"

using namespace std;
using namespace dammIds::correlator;

namespace dammIds {
    namespace correlator {
        const int D_CONDITION            = 0;//!< Conditions
        const int D_TIME_BW_IMPLANTS     = 1;//!< Time between implants
        const int D_TIME_BW_ALL_IMPLANTS = 2;//!< Time between all implants
    }
} // correlator namespace

// all in seconds
const double Correlator::minImpTime = 5e-3;
const double Correlator::corrTime   = 60; // used to be 3300
const double Correlator::fastTime   = 40e-6;

Correlator::Correlator() : histo(OFFSET, RANGE, "correlator"),
    lastImplant(NULL), lastDecay(NULL), condition(UNKNOWN_CONDITION) {
}

EventInfo::EventInfo() {
    flagged = false;
    pileUp  = false;
    hasVeto = false;
    hasTof  = false;
    beamOn  = false;
    foilTime = offTime = energy = time = position = NAN;
    boxMult = mcpMult = impMult = 0;
    type = UNKNOWN_EVENT;
    logicBits[0] = 'X';
    logicBits[1] = '\0';
    logicBits[dammIds::logic::MAX_LOGIC] = '\0';
    generation = 0;
}

CorrelationList::CorrelationList() : std::vector<EventInfo>() {
    flagged = false;
}

double CorrelationList::GetDecayTime() const {
    if(empty() || back().type == EventInfo::IMPLANT_EVENT) {
            return NAN;
        }
    else {
            return back().dtime;
        }
}

double CorrelationList::GetImplantTime() const {
    if(empty() || front().type != EventInfo::IMPLANT_EVENT) {
            return NAN;
        }
    else {
            return front().time;
        }
}

void CorrelationList::Flag() {
    if(!empty())
        back().flagged = true;
    flagged = true;
}

bool CorrelationList::IsFlagged() const {
    return flagged;
}

void CorrelationList::clear() {
    flagged = false;
    vector<EventInfo>::clear();
}

void CorrelationList::PrintDecayList() const {
    ofstream fullLog("HIS/full_decays.txt", ios::app);
    stringstream str;
    DetectorDriver* driver = DetectorDriver::get();
    const double printTimeResolution = 1e-3;
    if(empty()) {
            cout << "    EMPTY" << endl;
            return;
        }
    double firstTime = front().time;
    double lastTime = firstTime;
    time_t theTime = driver->GetWallTime(firstTime);
    str  << " " << ctime(&theTime)
         << "    TAC: " << setw(8) << front().tof
         << ",    ts: " << fixed << setprecision(8)
         << (firstTime * Globals::get()->clockInSeconds())
         << ",    cc: " << scientific << setprecision(3)
         << front().clockCount << endl;
    cout << str.str();
#ifndef ONLINE
    fullLog << str.str();
#endif
    str.str("");
    for(const_iterator it = begin(); it != end(); it++) {
            double dt   = ((*it).time - firstTime) *
                          Globals::get()->clockInSeconds() / printTimeResolution;
            double dt2 = ((*it).time - lastTime) *
                         Globals::get()->clockInSeconds() / printTimeResolution;
            double offt = (*it).offTime *
                          Globals::get()->clockInSeconds() / printTimeResolution;
            if((*it).flagged) {
                    str << " * " << setw(2) << (*it).generation << " E";
                }
            else {
                    if((*it).type == EventInfo::GAMMA_EVENT) {
                            str << " G ";
                        }
                    else {
                            str << "   ";
                        }
                    str << setw(2) << (*it).generation << " E";
                }
            str  << setw(10) << fixed << setprecision(4) << (*it).energy
                 << " [ch] at T " << setw(10) << dt
                 << ", DT= " << setw(10) << dt2;
            if((*it).type != EventInfo::GAMMA_EVENT) {
                    str << ", OT (" << (*it).logicBits << ")= " << setw(10) << offt << " [ms]"
                        << " M" << (*it).mcpMult << "I" << (*it).impMult << "B" << (*it).boxMult;
                    if(!isnan((*it).position)) {
                            str << " POS = " << (*it).position;
                        }
                }
            str << endl;
            /*
            if ((*it).mcpMult > 0 && !isnan((*it).foilTime) )
                str << "      Foil time: " << (*it).foilTime << endl;
            */
            if((*it).boxMult > 0)
                str << "      Box: E " << (*it).energyBox << " for location " << (*it).boxMax << endl;
            lastTime = (*it).time;
        }
    str << endl;
    cout << str.str();
#ifndef ONLINE
    fullLog << str.str();
#endif
    cout.unsetf(ios::floatfield);
}

Correlator::~Correlator() {
    // dump any flagged decay lists which have not been output
    for(unsigned int i=0; i < arraySize; i++) {
            for(unsigned int j=0; j < arraySize; j++) {
                    if(IsFlagged(i,j))
                        PrintDecayList(i,j);
                }
        }
}

void Correlator::DeclarePlots() {
    using namespace dammIds::correlator;
    static bool done = false;
    if(done)  {
            return;
        }
    DeclareHistogram1D(D_CONDITION, S9, "Correlator condition");
    DeclareHistogram1D(D_TIME_BW_IMPLANTS, S9, "time between implants, 100 ms/bin");
    DeclareHistogram1D(D_TIME_BW_ALL_IMPLANTS, SA, "time between all implants, 1 us/bin");
    done = true;
}

void Correlator::Correlate(EventInfo &event, unsigned int fch,
                           unsigned int bch) {
    using namespace dammIds::correlator;
    if(fch < 0 || fch >= arraySize || bch < 0  || bch >= arraySize) {
            plot(D_CONDITION, INVALID_LOCATION);
            return;
        }
    CorrelationList &theList = decaylist[fch][bch];
    double lastTime = NAN;
    double clockInSeconds = Globals::get()->clockInSeconds();
    switch(event.type) {
            case EventInfo::IMPLANT_EVENT:
                if(theList.IsFlagged()) {
                        PrintDecayList(fch, bch);
                    }
                lastTime = GetImplantTime(fch, bch);
                theList.clear();
                condition = VALID_IMPLANT;
                if(lastImplant != NULL) {
                        double dt = event.time - lastImplant->time;
                        plot(D_TIME_BW_ALL_IMPLANTS, dt * clockInSeconds / 1e-6);
                    }
                if(!isnan(lastTime)) {
                        condition = BACK_TO_BACK_IMPLANT;
                        event.dtime = event.time - lastTime;
                        plot(D_TIME_BW_IMPLANTS, event.dtime * clockInSeconds / 100e-3);
                    }
                else {
                        event.dtime = INFINITY;
                    }
                event.generation = 0;
                theList.push_back(event);
                lastImplant = &theList.back();
                break;
            default:
                if(theList.empty()) {
                        break;
                    }
                if(isnan(theList.GetImplantTime())) {
                        cout << "No implant time for decay list" << endl;
                        break;
                    }
                if(event.type == EventInfo::UNKNOWN_EVENT) {
                        condition = UNKNOWN_CONDITION;
                    }
                else {
                        condition = VALID_DECAY;
                    }
                condition = VALID_DECAY; // tmp -- DTM
                lastTime = theList.back().time;
                double dt = event.time - theList.GetImplantTime();
                if(dt < 0) {
                        if(dt < -5e11 && event.time < 1e9) {
                                cout << "Decay following pixie clock reset, clearing decay lists!" << endl;
                                cout << "  Event time: " << event.time
                                     << "\n  Implant time: " << theList.GetImplantTime()
                                     << "\n  DT: " << dt << endl;
                                // PIXIE's clock has most likely been zeroed due to a file marker
                                //   no chance of doing correlations
                                for(unsigned int i=0; i < arraySize; i++) {
                                        for(unsigned int j=0; j < arraySize; j++) {
                                                if(IsFlagged(i,j)) {
                                                        PrintDecayList(i, j);
                                                    }
                                                decaylist[i][j].clear();
                                            }
                                    }
                            }
                        else if(event.type != EventInfo::GAMMA_EVENT) {
                                // since gammas are processed at a different time than everything else
                                cout << "negative correlation time, DECAY: " << event.time
                                     << " IMPLANT: " << theList.GetImplantTime()
                                     << " DT: " << dt << endl;
                            }
                        event.dtime = NAN;
                        break;
                    } // negative correlation itme
                if(theList.front().dtime * clockInSeconds >= minImpTime) {
                        if(dt * clockInSeconds < corrTime) {
                                // event.dtime = event.time - lastTime; // (FOR CHAINS)
                                event.dtime = event.time - theList.front().time; // FOR LERIBSS
                                if(event.dtime * clockInSeconds < fastTime && event.dtime > 0) {
                                        // event.flagged = true;
                                    }
                            }
                        else {
                                // event.dtime = event.time - lastTime; // (FOR CHAINS)
                                event.dtime = event.time - theList.front().time; // FOR LERIBSS
                                condition = DECAY_TOO_LATE;
                            }
                    }
                else {
                        condition = IMPLANT_TOO_SOON;
                    }
                if(condition == VALID_DECAY) {
                        event.generation = theList.back().generation + 1;
                    }
                theList.push_back(event);
                if(event.energy == 0 && isnan(event.time))
                    cout << " Adding zero decay event " << endl;
                if(event.flagged)
                    theList.Flag();
                if(condition == VALID_DECAY) {
                        lastDecay = &theList.back();
                    }
                else if(condition == DECAY_TOO_LATE) {
                        theList.clear();
                    }
                break;
        }
    plot(D_CONDITION, condition);
}

void Correlator::CorrelateAll(EventInfo &event) {
    for(unsigned int fch=0; fch < arraySize; fch++) {
            for(unsigned int bch=0; bch < arraySize; bch++) {
                    if(decaylist[fch][bch].size() == 0)
                        continue;
                    if(event.time - decaylist[fch][bch].back().time <
                            10e-6 / Globals::get()->clockInSeconds()) {
                            // only correlate fast events for now
                            Correlate(event, fch, bch);
                        }
                }
        }
}

void Correlator::CorrelateAllX(EventInfo &event, unsigned int bch) {
    for(unsigned int fch = 0; fch < arraySize; fch++) {
            Correlate(event, fch, bch);
        }
}

void Correlator::CorrelateAllY(EventInfo &event, unsigned int fch) {
    for(unsigned int bch = 0; bch < arraySize; bch++) {
            Correlate(event, fch, bch);
        }
}

double Correlator::GetDecayTime(void) const {
    if(lastDecay == NULL) {
            return NAN;
        }
    else {
            return lastDecay->dtime;
        }
}

double Correlator::GetDecayTime(int fch, int bch) const {
    return decaylist[fch][bch].GetDecayTime();
}

double Correlator::GetImplantTime(void) const {
    if(lastImplant == NULL) {
            return NAN;
        }
    else {
            return lastImplant->time;
        }
}

double Correlator::GetImplantTime(int fch, int bch) const {
    return decaylist[fch][bch].GetImplantTime();
}

void Correlator::Flag(int fch, int bch) {
    if(!decaylist[fch][bch].empty())
        decaylist[fch][bch].Flag();
}

bool Correlator::IsFlagged(int fch, int bch) {
    return decaylist[fch][bch].IsFlagged();
}

void Correlator::PrintDecayList(unsigned int fch, unsigned int bch) const {
    cout << "Current decay list for " << fch << " , " << bch << " : " << endl;
    decaylist[fch][bch].PrintDecayList();
}
