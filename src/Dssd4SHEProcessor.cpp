/*! \file DssdProcessor.cpp
 *
 * The DSSD processor handles detectors of type dssd_front and dssd_back and
 *   determines whether the events are implants or decays and informs the
 *   correlator accordingly
 */

#include <limits>
#include <stdexcept>

#include "DammPlotIds.hpp"
#include "Globals.hpp"
#include "Dssd4SHEProcessor.hpp"
#include "RawEvent.hpp"

using namespace dammIds::dssd4she;
using namespace std;

Dssd4SHEProcessor::Dssd4SHEProcessor(double frontBackTimeWindow,
                                     double frontBackDE) : 
    EventProcessor(OFFSET, RANGE, "dssd4she")
{
    frontBackTimeWindow_ = frontBackTimeWindow;
    frontBackDE_ = frontBackDE;
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
        if (bestDtime < frontBackTimeWindow_) {
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

    for (vector< pair<ChanEvent*, ChanEvent*> >::iterator it =
                                                 xyEventsTMatch_.begin();
         it != xyEventsTMatch_.end();
         ++it) {
        double xEnergy = (*it).first->GetCalEnergy();
        int xPosition = (*it).first->GetChanID().GetLocation();
        double yEnergy = (*it).second->GetCalEnergy();
        int yPosition = (*it).second->GetChanID().GetLocation();

        plot(D_ENERGY_X, xEnergy); 
        plot(D_ENERGY_Y, yEnergy);
        plot(DD_EVENT_ENERGY__X_POSITION, xEnergy, xPosition);
        plot(DD_EVENT_ENERGY__Y_POSITION, yEnergy, yPosition);

        plot(DD_EVENT_POSITION_FROM_T, xPosition, yPosition);
    }

    for (vector< pair<ChanEvent*, ChanEvent*> >::iterator it =
                                                 xyEventsEMatch_.begin();
         it != xyEventsEMatch_.end();
         ++it) {
        //double xEnergy = (*it).first->GetCalEnergy();
        int xPosition = (*it).first->GetChanID().GetLocation();
        //double yEnergy = (*it).second->GetCalEnergy();
        int yPosition = (*it).second->GetChanID().GetLocation();

        plot(DD_EVENT_POSITION_FROM_E, xPosition, yPosition);
    }


    EndProcess();
    return true;
}


