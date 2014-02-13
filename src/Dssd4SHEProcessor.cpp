/*! \file DssdProcessor.cpp
 *
 * The DSSD processor handles detectors of type dssd_front and dssd_back and
 *   determines whether the events are implants or decays and informs the
 *   correlator accordingly
 */

#include <limits>
#include <stdexcept>
#include <algorithm>

#include "DammPlotIds.hpp"
#include "Globals.hpp"
#include "Dssd4SHEProcessor.hpp"
#include "RawEvent.hpp"

using namespace dammIds::dssd4she;
using namespace std;

Dssd4SHEProcessor::Dssd4SHEProcessor(double timeWindow,
                                     double highEnergyCut,
                                     double lowEnergyCut,
                                     int numFrontStrips,
                                     int numBackStrips) : 
    EventProcessor(OFFSET, RANGE, "dssd4she"),
    correlator(numFrontStrips, numBackStrips)
{
    timeWindow_ = timeWindow;
    highEnergyCut_ = highEnergyCut;
    lowEnergyCut_ = lowEnergyCut;
    name = "dssd";
    associatedTypes.insert("dssd_front");
    associatedTypes.insert("dssd_back");
}


void Dssd4SHEProcessor::DeclarePlots(void)
{
    using namespace dammIds::dssd;

    const int energyBins = SE;
    const int xBins = S7;
    const int yBins = S6;

    DeclareHistogram1D(D_ENERGY_X, energyBins, "Energy dssd X strips");
    DeclareHistogram1D(D_ENERGY_Y, energyBins, "Energy dssd Y strips");

    DeclareHistogram1D(D_DTIME_MISSING, SA, "Missing pairs time diff in ns");
    DeclareHistogram1D(D_DE_MISSING, SA, "Missing pairs energy diff in calib.");

    DeclareHistogram1D(D_ENERGY_CORRELATED_SIDE, energyBins, 
                       "Energy Side corr. with DSSD");

    DeclareHistogram2D(DD_EVENT_POSITION_FROM_T, 
		       xBins, yBins, "DSSD position from time correlation");
    DeclareHistogram2D(DD_EVENT_POSITION_FROM_E, 
		       xBins, yBins, "DSSD position from energy correlation");
    DeclareHistogram2D(DD_EVENT_ENERGY__X_POSITION,
		       energyBins, xBins, "DSSD X strips E vs. position");
    DeclareHistogram2D(DD_EVENT_ENERGY__Y_POSITION,
		       energyBins, yBins, "DSSD Y strips E vs. position");

    DeclareHistogram2D(DD_ENERGY__POSX_T_MISSING,
		       energyBins, xBins, "DSSD T missing X strips E vs. position");
    DeclareHistogram2D(DD_ENERGY__POSY_T_MISSING,
		       energyBins, yBins, "DSSD T missing Y strips E vs. position");

    DeclareHistogram2D(DD_DENERGY__DPOS_X_CORRELATED,
		       energyBins, xBins, "DSSD dE dX correlated events");
    DeclareHistogram2D(DD_DENERGY__DPOS_Y_CORRELATED,
		       energyBins, yBins, "DSSD dE dY correlated events");

}


bool Dssd4SHEProcessor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return false;

    xyEventsTMatch_.clear();
    xyEventsEMatch_.clear();

    vector<ChanEvent*> xEvents = 
        event.GetSummary("dssd_back:dssd_back", true)->GetList();
    vector<ChanEvent*> yEvents = 
        event.GetSummary("dssd_front:dssd_front", true)->GetList();

    /**
     * Matching the front-back by the time correlations
     */
    vector< pair<ChanEvent*, bool> > xEventsTMatch;
    vector< pair<ChanEvent*, bool> > yEventsTMatch;

    for (vector<ChanEvent*>::iterator itx = xEvents.begin();
         itx != xEvents.end();
         ++itx) {
        pair<ChanEvent*, bool> match((*itx), false);
        xEventsTMatch.push_back(match);

        for (vector<ChanEvent*>::iterator itx2 = itx;
            itx2 != xEvents.end();
            ++itx2) {
            int dx = abs( (*itx)->GetChanID().GetLocation() -
                          (*itx2)->GetChanID().GetLocation());
            double dE = abs( (*itx)->GetCalEnergy() -  
                             (*itx2)->GetCalEnergy());
            plot(DD_DENERGY__DPOS_X_CORRELATED, dE, dx);
        }
    }

    for (vector<ChanEvent*>::iterator ity = yEvents.begin();
         ity != yEvents.end();
         ++ity) {
        pair<ChanEvent*, bool> match((*ity), false);
        yEventsTMatch.push_back(match);

        for (vector<ChanEvent*>::iterator ity2 = ity;
            ity2 != yEvents.end();
            ++ity2) {
            int dy = abs( (*ity)->GetChanID().GetLocation() -
                          (*ity2)->GetChanID().GetLocation());
            double dE = abs( (*ity)->GetCalEnergy() -  
                             (*ity2)->GetCalEnergy());
            plot(DD_DENERGY__DPOS_Y_CORRELATED, dE, dy);
        }
    }

    for (vector< pair<ChanEvent*, bool> >::iterator itx = xEventsTMatch.begin();
         itx != xEventsTMatch.end();
         ++itx) {
        double bestDtime = numeric_limits<double>::max();
        vector< pair<ChanEvent*, bool> >::iterator bestMatch =
            yEventsTMatch.end();
        for (vector< pair<ChanEvent*, bool> >::iterator ity = 
                                                     yEventsTMatch.begin();
            ity != yEventsTMatch.end();
            ++ity) {
            if ((*ity).second)
                continue;
            double dTime = abs((*itx).first->GetTime() - 
                               (*ity).first->GetTime()) *
                                Globals::get()->clockInSeconds();
            if (dTime < bestDtime) {
                bestDtime = dTime;
                bestMatch = ity;
            }
        }
        if (bestDtime < timeWindow_) {
            xyEventsTMatch_.push_back(
                pair<ChanEvent*, ChanEvent*>((*itx).first, (*bestMatch).first));
            (*itx).second = true;
            (*bestMatch).second = true;
        } else {
            bestDtime = int(bestDtime / 1e-9);
            if (bestDtime > SA)
                bestDtime = 1023;
            else if (bestDtime < 0)
                bestDtime = 0;
            plot(D_DTIME_MISSING, bestDtime);
        }
    }

    for (vector< pair<ChanEvent*, bool> >::iterator itx = xEventsTMatch.begin();
         itx != xEventsTMatch.end();
         ++itx) {
        if ((*itx).second)
            continue;
        int position = (*itx).first->GetChanID().GetLocation();
        double energy = (*itx).first->GetCalEnergy();
        plot(DD_ENERGY__POSX_T_MISSING, energy, position);
    }

    for (vector< pair<ChanEvent*, bool> >::iterator ity = yEventsTMatch.begin();
         ity != yEventsTMatch.end();
         ++ity) {
        if ((*ity).second)
            continue;
        int position = (*ity).first->GetChanID().GetLocation();
        double energy = (*ity).first->GetCalEnergy();
        plot(DD_ENERGY__POSY_T_MISSING, energy, position);
    }

    /**
     * Matching the front-back by the Energy of the event
     * Using the old style GetMaxEvent for comparison
     */
    if (xEvents.size() > 0 && yEvents.size() > 0) {
        xyEventsEMatch_.push_back(
            pair<ChanEvent*, ChanEvent*>(
                event.GetSummary("dssd_back:dssd_back")->GetMaxEvent(true),
                event.GetSummary("dssd_front:dssd_front")->GetMaxEvent(true)
            )
        );
    }

    return true;
}


bool Dssd4SHEProcessor::Process(RawEvent &event)
{
    using namespace dammIds::dssd4she;

    if (!EventProcessor::Process(event))
        return false;

    vector<ChanEvent*> vetoEvents = 
        event.GetSummary("si:veto", true)->GetList();
    vector<ChanEvent*> sideEvents = 
        event.GetSummary("si:si", true)->GetList();
    int mwpc = event.GetSummary("mcp", true)->GetMult();

    for (vector< pair<ChanEvent*, ChanEvent*> >::iterator it =
                                                 xyEventsTMatch_.begin();
         it != xyEventsTMatch_.end();
         ++it) {
        /**
         * Notice that yEnergy (back) is more precise than
         * xEnergy (front) due to shorter strips and (if that's true)
         * high gain
         */
        double yEnergy = (*it).second->GetCalEnergy();
        if (yEnergy < lowEnergyCut_)
            continue;

        double xEnergy = (*it).first->GetCalEnergy();
        int xPosition = (*it).first->GetChanID().GetLocation();
        int yPosition = (*it).second->GetChanID().GetLocation();

        double time = min((*it).first->GetTime(), (*it).second->GetTime());

        plot(D_ENERGY_X, xEnergy); 
        plot(D_ENERGY_Y, yEnergy);
        plot(DD_EVENT_ENERGY__X_POSITION, xEnergy, xPosition);
        plot(DD_EVENT_ENERGY__Y_POSITION, yEnergy, yPosition);

        plot(DD_EVENT_POSITION_FROM_T, xPosition, yPosition);

        double bestSiTime = numeric_limits<double>::max();

        ChanEvent* correlatedSide = 0;
        for (vector<ChanEvent*>::iterator its = sideEvents.begin();
            its != sideEvents.end();
            ++its) {
            double dt = abs(time - (*its)->GetTime());
            if (dt < bestSiTime) {
                bestSiTime = dt;
                correlatedSide = *its;
            }
        }
        if (correlatedSide != 0) {
            if (bestSiTime < timeWindow_)
                plot(D_ENERGY_CORRELATED_SIDE, correlatedSide->GetCalEnergy());
        }

    }

    for (vector< pair<ChanEvent*, ChanEvent*> >::iterator it =
                                                 xyEventsEMatch_.begin();
         it != xyEventsEMatch_.end();
         ++it) {
        double yEnergy = (*it).second->GetCalEnergy();
        if (yEnergy < lowEnergyCut_)
            continue;

        int xPosition = (*it).first->GetChanID().GetLocation();
        int yPosition = (*it).second->GetChanID().GetLocation();

        plot(DD_EVENT_POSITION_FROM_E, xPosition, yPosition);
    }


    EndProcess();
    return true;
}


bool Dssd4SHEProcessor::pickEventType(SheEvent& event) {
    bool high_energy = false;
    if (event.get_energy() > highEnergyCut_)
        high_energy = true;

    if (high_energy) {
        if (event.get_veto()) {
            if (event.get_mwpc() > 0) {
                if (event.get_beam())
                    event.set_type(lightIon);
                else
                    event.set_type(unknown);
            } else {
                if (event.get_beam())
                    event.set_type(unknown);
                else
                    event.set_type(fission);
            }
        }
        else {
            if (event.get_mwpc() > 0) {
                if (event.get_beam())
                    event.set_type(lightIon);
                else
                    event.set_type(fission);
            } else {
                event.set_type(fission);
            }
        }
    }
    else {
        if (event.get_veto()) {
            if (event.get_mwpc() > 0) {
                if (event.get_beam())
                    event.set_type(lightIon);
                else
                    event.set_type(unknown);
            } else {
                if (event.get_beam())
                    event.set_type(lightIon);
                else
                    event.set_type(unknown);
            }
        }
        else {
            if (event.get_mwpc() > 0) {
                if (event.get_beam())
                    event.set_type(heavyIon);
                else
                    event.set_type(decay);
            } else {
                event.set_type(decay);
            }
        }
    }
    return true;
}
