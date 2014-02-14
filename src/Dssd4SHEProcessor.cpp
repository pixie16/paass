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
                                     double deltaEnergy,
                                     double highEnergyCut,
                                     double lowEnergyCut,
                                     double fissionEnergyCut,
                                     int numBackStrips,
                                     int numFrontStrips) :
    EventProcessor(OFFSET, RANGE, "dssd4she"),
    correlator_(numBackStrips, numFrontStrips)
{
    timeWindow_ = timeWindow;
    deltaEnergy_ = deltaEnergy;
    highEnergyCut_ = highEnergyCut;
    lowEnergyCut_ = lowEnergyCut;
    fissionEnergyCut_ = fissionEnergyCut;
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

    DeclareHistogram1D(D_DTIME, S8, "Pairs time diff in 10 ns (+ 1 bin)");

    DeclareHistogram1D(D_MWPC_MULTI, S5, "MWPC multiplicity");
    DeclareHistogram1D(D_ENERGY_CORRELATED_SIDE, energyBins, 
                       "Energy Side corr. with DSSD");
    DeclareHistogram1D(D_DTIME_SIDE, S8, 
                        "Side det. time diff in 10 ns (+ 1 bin)");

    DeclareHistogram2D(DD_EVENT_POSITION, 
		       xBins, yBins, "DSSD all events positions");
    DeclareHistogram2D(DD_EVENT_POSITION_FROM_E, 
		       xBins, yBins, "DSSD position all max event");
    DeclareHistogram2D(DD_IMPLANT_POSITION, 
		       xBins, yBins, "DSSD position implant");
    DeclareHistogram2D(DD_DECAY_POSITION, 
		       xBins, yBins, "DSSD position decay");
    DeclareHistogram2D(DD_LIGHT_POSITION, 
		       xBins, yBins, "DSSD position light ion");
    DeclareHistogram2D(DD_UNKNOWN_POSITION, 
		       xBins, yBins, "DSSD position unknown");
    DeclareHistogram2D(DD_FISSION_POSITION, 
		       xBins, yBins, "DSSD position fission");

    DeclareHistogram2D(DD_EVENT_ENERGY__X_POSITION,
		       energyBins, xBins, "DSSD X strips E vs. position");
    DeclareHistogram2D(DD_EVENT_ENERGY__Y_POSITION,
		       energyBins, yBins, "DSSD Y strips E vs. position");
    DeclareHistogram2D(DD_MAXEVENT_ENERGY__X_POSITION,
		       energyBins, xBins, "MAXDSSD X strips E vs. position");
    DeclareHistogram2D(DD_MAXEVENT_ENERGY__Y_POSITION,
		       energyBins, yBins, "MAXDSSD Y strips E vs. position");

    DeclareHistogram2D(DD_FRONTE__BACKE, SA, SA,
            "Front vs Back energy (calib / 100)");
    DeclareHistogram2D(DD_ENERGY__POSX_T_MISSING,
		       energyBins, xBins, "DSSD T missing X strips E vs. position");
    DeclareHistogram2D(DD_ENERGY__POSY_T_MISSING,
		       energyBins, yBins, "DSSD T missing Y strips E vs. position");

    /** Check how many strips and how far fired **/
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
            ++ity) 
        {
            // If already matched, skip
            if ((*ity).second)
                continue;

            double energyX = (*itx).first->GetCalEnergy();
            double energyY = (*ity).first->GetCalEnergy();


            /** If energies are in lower range and/or not satured
             *  check if delta energy condition is not met, 
             *  if not, skip this event
             *
             *  For high energy events and satured set 20 MeV
             *  energy for difference check. The calibration in this
             *  range is most likely imprecise, so one cannot correlate
             *  by energy difference.
             **/
            if ( (*itx).first->IsSaturated() || energyX > highEnergyCut_ )
                energyX = 20000.0;
            if ( (*ity).first->IsSaturated() || energyY > highEnergyCut_ )
                energyY = 20000.0;
            if ( abs(energyX - energyY) > deltaEnergy_)
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
            plot(D_DTIME, int(bestDtime / 1.0e-8) + 1);
        } else {
            bestDtime = int(bestDtime / 1.0e-8);
            if (bestDtime > S8)
                bestDtime = S8 - 1;
            else if (bestDtime < 0)
                bestDtime = 0;
            plot(D_DTIME, bestDtime);
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
    plot(D_MWPC_MULTI, mwpc); 

    for (vector< pair<ChanEvent*, ChanEvent*> >::iterator it =
                                                 xyEventsTMatch_.begin();
         it != xyEventsTMatch_.end();
         ++it) {

        double xEnergy = (*it).first->GetCalEnergy();
        double yEnergy = (*it).second->GetCalEnergy();

        /** If saturated set to 100 MeV **/
        if ((*it).first->IsSaturated() && !(*it).second->IsSaturated())
            xEnergy = yEnergy;
        else if (!(*it).first->IsSaturated() && (*it).second->IsSaturated())
            yEnergy = xEnergy;
        else if ((*it).first->IsSaturated() && (*it).second->IsSaturated()) {
            xEnergy = 100000.0;
            yEnergy = 100000.0;
        }

        int xPosition = (*it).first->GetChanID().GetLocation();
        int yPosition = (*it).second->GetChanID().GetLocation();

        double time = min((*it).first->GetTime(), (*it).second->GetTime());

        plot(D_ENERGY_X, xEnergy); 
        plot(D_ENERGY_Y, yEnergy);

        plot(DD_FRONTE__BACKE, xEnergy / 100.0, yEnergy / 100.0);
        plot(DD_EVENT_ENERGY__X_POSITION, xEnergy, xPosition);
        plot(DD_EVENT_ENERGY__Y_POSITION, yEnergy, yPosition);

        plot(DD_EVENT_POSITION, xPosition, yPosition);

        double bestSiTime = numeric_limits<double>::max();

        ChanEvent* correlatedSide = 0;
        bool hasEscape = false;
        double escapeEnergy = 0.0;
        for (vector<ChanEvent*>::iterator its = sideEvents.begin();
            its != sideEvents.end();
            ++its) {
            double dt = abs(time - (*its)->GetTime()) *
                        Globals::get()->clockInSeconds();
            if (dt < bestSiTime) {
                bestSiTime = dt;
                correlatedSide = *its;
            }
        }

        if (correlatedSide != 0) {
            int siTime = int(bestSiTime / 1.0e-8) + 1;
            if (siTime > S8)
                siTime = S8 - 1;
            else if (siTime < 0)
                siTime = 0;
            plot(D_DTIME_SIDE, siTime);

            if (bestSiTime < timeWindow_) {
                plot(D_ENERGY_CORRELATED_SIDE, correlatedSide->GetCalEnergy());
                hasEscape = true;
                escapeEnergy = correlatedSide->GetCalEnergy();
            }
        }

        bool hasBeam = true;
        bool hasVeto = false;
        if (vetoEvents.size() > 0)
            hasVeto = true;
        SheEvent event = SheEvent(xEnergy + escapeEnergy, time, mwpc,
                                  hasBeam, hasVeto, hasEscape, unknown);
        pickEventType(event);
        if (event.get_type() == heavyIon) {
            plot(DD_IMPLANT_POSITION, xPosition, yPosition);
        }
        else if (event.get_type() == alpha) {
            plot(DD_DECAY_POSITION, xPosition, yPosition);
        }
        else if (event.get_type() == lightIon) {
            plot(DD_LIGHT_POSITION, xPosition, yPosition);
        }
        else if (event.get_type() == unknown) {
            plot(DD_UNKNOWN_POSITION, xPosition, yPosition);
        }
        else if (event.get_type() == fission) {
            plot(DD_FISSION_POSITION, xPosition, yPosition);
        }

        correlator_.add_event(event, xPosition, yPosition);

    }

    /** Old style max event */
    for (vector< pair<ChanEvent*, ChanEvent*> >::iterator it =
                                                 xyEventsEMatch_.begin();
         it != xyEventsEMatch_.end();
         ++it) {
        double xEnergy = (*it).first->GetCalEnergy();
        double yEnergy = (*it).second->GetCalEnergy();

        int xPosition = (*it).first->GetChanID().GetLocation();
        int yPosition = (*it).second->GetChanID().GetLocation();

        plot(DD_EVENT_POSITION_FROM_E, xPosition, yPosition);
        plot(DD_MAXEVENT_ENERGY__X_POSITION, xEnergy, xPosition);
        plot(DD_MAXEVENT_ENERGY__Y_POSITION, yEnergy, yPosition);

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
                {
                    if (event.get_energy() > fissionEnergyCut_)
                        event.set_type(fission);
                    else
                        event.set_type(unknown);
                }
            }
        }
        else {
            if (event.get_mwpc() > 0) {
                if (event.get_beam())
                    event.set_type(lightIon);
                else 
                {
                    if (event.get_energy() > fissionEnergyCut_)
                        event.set_type(fission);
                    else
                        event.set_type(unknown);
                }
            }
            else {
                    if (event.get_energy() > fissionEnergyCut_)
                        event.set_type(fission);
                    else
                        event.set_type(unknown);
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
                else {
                    if (event.get_energy() > lowEnergyCut_)
                        event.set_type(alpha);
                    else
                        event.set_type(unknown);
                }
            } else {
                if (event.get_energy() > lowEnergyCut_)
                    event.set_type(alpha);
                else
                    event.set_type(unknown);
            }
        }
    }
    return true;
}


