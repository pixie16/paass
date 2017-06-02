///@file PspmtProcessor.cpp
///@brief Processes information from a Position Sensitive PMT.
///@authors S. Go and S. V. Paulauskas
///@date August 24, 2016
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <signal.h>
#include <limits.h>

#include "DammPlotIds.hpp"
#include "PspmtProcessor.hpp"
#include "Globals.hpp"
#include "Messenger.hpp"

using namespace std;
using namespace dammIds::pspmt;

namespace dammIds {
    namespace pspmt {
        const int DD_QDC = 0;
        const int DD_POSITION_ENERGY = 1;
        const int DD_POSITION_QDC = 2;
        const int DD_POSITION_TRACE = 3;
    }
} // namespace dammIds



void PspmtProcessor::DeclarePlots(void) {
    DeclareHistogram2D(DD_QDC, SD, S3, "QDC - Dynode 0 - Anodes 1-4");
    DeclareHistogram2D(DD_POSITION_ENERGY, SB, SB, "Pos from Raw Energy");
    DeclareHistogram2D(DD_POSITION_QDC, SB, SB, "Pos from QDC");
    DeclareHistogram2D(DD_POSITION_TRACE, SB, SB, "Pos from TraceFilter");
}

PspmtProcessor::PspmtProcessor(const std::string &vd, const double &scale,
                               const unsigned int &offset,
                               const double &threshold) :
        EventProcessor(OFFSET, RANGE, "PspmtProcessor") {
    if(vd == "SIB064_1018")
        vdtype_ = SIB064_1018;
    else if(vd == "SIB064_0926")
        vdtype_ = SIB064_0926;
    else
        vdtype_ = UNKNOWN;
    histogramScale_ = scale;
    histogramOffset_ = offset;
    threshold_ = threshold;

    ///Associates this processor with the pspmt detector type
    associatedTypes.insert("pspmt");
}

bool PspmtProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return false;

    static const vector<ChanEvent *> &dynodeEvents =
            event.GetSummary("pspmt:dynode")->GetList();
    static const vector<ChanEvent *> &anodeEvents =
            event.GetSummary("pspmt:anode")->GetList();

    //If we do not have more than 4 anode events then something went awry. 
    if (anodeEvents.size() > 4) {
        EndProcess();
        return false;
    }

    for(vector<ChanEvent *>::const_iterator it = dynodeEvents.begin();
            it != dynodeEvents.end(); it++) {
        plot(DD_QDC, (*it)->GetTrace().GetQdc(), 0);
    }

    //Define some maps that we will use to hold the information necessary to
    // calculate the positions.
    map<string, double> m_qdc, m_energy, m_trace;

    //Define some values that we will use inside the loop repeatedly.
    double qdc = 0, energy = 0, traceFilter = 0;

    //Loop over all of the anode events to gather up all the values we need
    // to calculate the position
    for (vector<ChanEvent *>::const_iterator it = anodeEvents.begin();
         it != anodeEvents.end(); it++) {

        //Obtain the energy calculated by the Pixie-16 on-board trapezoidal
        // filter.
        energy = (*it)->GetCalibratedEnergy();

        //We will skip this event if the energy is below threshold
        if(energy < threshold_)
            continue;

        qdc = (*it)->GetTrace().GetQdc();
        traceFilter = (*it)->GetTrace().GetQdc();

        if ((*it)->GetChanID().HasTag("xa")) {
            InsertMapValue(m_energy, "xa", energy);
            InsertMapValue(m_qdc, "xa", qdc);
            InsertMapValue(m_trace, "xa", traceFilter);
            plot(DD_QDC, qdc, 1);
        }

        if ((*it)->GetChanID().HasTag("xb")) {
            InsertMapValue(m_energy, "xb", energy);
            InsertMapValue(m_qdc, "xb", qdc);
            InsertMapValue(m_trace, "xb", traceFilter);
            plot(DD_QDC, qdc, 2);
        }

        if ((*it)->GetChanID().HasTag("ya")) {
            InsertMapValue(m_energy, "ya", energy);
            InsertMapValue(m_qdc, "ya", qdc);
            InsertMapValue(m_trace, "ya", traceFilter);
            plot(DD_QDC, qdc, 3);
        }

        if ((*it)->GetChanID().HasTag("yb")) {
            InsertMapValue(m_energy, "yb", energy);
            InsertMapValue(m_qdc, "yb", qdc);
            InsertMapValue(m_trace, "yb", traceFilter);
            plot(DD_QDC, qdc, 4);
        }
    }//for(vector<ChanEvent*>::const_iterator it = anodeEvents.begin();

    if(m_energy.size() == 4)
        posEnergy_ = CalculatePosition(m_energy, vdtype_);
    if(m_qdc.size() == 4)
        posQdc_ = CalculatePosition(m_qdc, vdtype_);
    if(m_trace.size() == 4)
        posTrace_ = CalculatePosition(m_trace, vdtype_);

    plot(DD_POSITION_ENERGY, posEnergy_.first*histogramScale_+histogramOffset_,
         posEnergy_.second*histogramScale_+histogramOffset_);
    plot(DD_POSITION_QDC, posQdc_.first*histogramScale_+histogramOffset_,
         posQdc_.second*histogramScale_+histogramOffset_);
    plot(DD_POSITION_TRACE, posTrace_.first*histogramScale_+histogramOffset_,
         posTrace_.second*histogramScale_+histogramOffset_);

    EndProcess();
    return true;
}

pair<double, double> PspmtProcessor::CalculatePosition(
        const std::map<std::string, double> &map, const VDTYPES &vdtype) {
    double x_val = 0, y_val = 0;
    double xa = map.find("xa")->second;
    double xb = map.find("xb")->second;
    double ya = map.find("ya")->second;
    double yb = map.find("yb")->second;

    switch (vdtype) {
        case SIB064_1018:
            x_val = (0.5 * (yb + xa)) / (xa + xb + ya + yb);
            y_val = (0.5 * (xa + xb)) / (xa + xb + ya + yb);
            break;
        case SIB064_0926:
            x_val = (xa - xb) / (xa + xb);
            y_val = (ya - yb) / (ya + yb);
            break;
        case UNKNOWN:
        default:
            cerr << "We received a VD_TYPE we didn't recognize"
                 << vdtype << endl;
            x_val = y_val = 0.0;
    }
    return make_pair(x_val, y_val);
}

pair<unsigned int,unsigned int> PspmtProcessor::CalculatePixel(
        const std::pair<double, double> &pos) {
    return make_pair(0,0);
}

pair<map<string, double>::iterator, bool> PspmtProcessor::InsertMapValue(
        std::map<string, double> &map, const std::string &key, const double
&value) {
    return map.insert(make_pair(key, value));
}
