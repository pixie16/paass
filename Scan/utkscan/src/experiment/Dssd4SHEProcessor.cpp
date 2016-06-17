/*! \file DssdProcessor.cpp
 *
 * The DSSD processor handles detectors of type dssd_front and dssd_back and
 *   determines whether the events are implants or decays and informs the
 *   correlator accordingly
 */

#include <algorithm>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>

#include "Dssd4SHEProcessor.hpp"
#include "DammPlotIds.hpp"
#include "Globals.hpp"
#include "Messenger.hpp"
#include "Notebook.hpp"
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

    stringstream ss;
    ss << fixed 
       << "#T" 
       << " " << setw(12) << "E (keV)"
       << " " << setw(12) << "t (ms)"
       << " M" << " "
       << "B" << " "
       << "V" << " "
       << "E" << " "
       << endl;
    Notebook::get()->report(ss.str());
}


void Dssd4SHEProcessor::DeclarePlots(void)
{
    using namespace dammIds::dssd;

    const int energyBins = SE;
    const int energyBins2 = SB;
    const int xBins = S7;
    const int yBins = S6;

    DeclareHistogram1D(D_ENERGY_X, energyBins, "Energy/10 dssd X strips");
    DeclareHistogram1D(D_ENERGY_Y, energyBins, "Energy/10 dssd Y strips");

    DeclareHistogram1D(D_DTIME, S8, "Pairs time diff in 10 ns (+ 1 bin)");

    DeclareHistogram1D(D_MWPC_MULTI, S5, "MWPC multiplicity");
    DeclareHistogram1D(D_ENERGY_CORRELATED_SIDE, energyBins, 
                       "Energy Side corr. with DSSD");
    DeclareHistogram1D(D_DTIME_SIDE, S8, 
                        "Side det. time diff in 10 ns (+ 1 bin)");

    DeclareHistogram2D(DD_ENERGY_DT__DSSD_MWPC, 
		       SB, S8, "DSSD energy/100 vs DT (10 ns) to MWPC");

    DeclareHistogram2D(DD_DE_E__DSSD_VETO, 
		       SB, SB, "DSSD energy/100 vs veto/100");

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

    DeclareHistogram1D(D_ENERGY_IMPLANT,
		       energyBins2, "DSSD energy/100 implant");
    DeclareHistogram1D(D_ENERGY_DECAY,
		       energyBins2, "DSSD energy/100 decay");
    DeclareHistogram1D(D_ENERGY_LIGHT,
		       energyBins2, "DSSD energy/100 light ion");
    DeclareHistogram1D(D_ENERGY_UNKNOWN,
		       energyBins2, "DSSD energy/100 unknown");
    DeclareHistogram1D(D_ENERGY_FISSION,
		       energyBins2, "DSSD energy/100 fission");
    DeclareHistogram1D(D_ENERGY_DECAY_BEAMSTOP,
		       energyBins, "DSSD energy*1 alpha beam stopped");

    DeclareHistogram2D(DD_EVENT_ENERGY__X_POSITION,
		       energyBins, xBins, "DSSD X strips E vs. position");
    DeclareHistogram2D(DD_EVENT_ENERGY__Y_POSITION,
		       energyBins, yBins, "DSSD Y strips E vs. position");
    DeclareHistogram2D(DD_MAXEVENT_ENERGY__X_POSITION,
		       energyBins, xBins, "MAXDSSD X strips E vs. position");
    DeclareHistogram2D(DD_MAXEVENT_ENERGY__Y_POSITION,
		       energyBins, yBins, "MAXDSSD Y strips E vs. position");

    DeclareHistogram1D(D_ENERGY_WITH_VETO, energyBins, 
                      "Energy dssd/10 coin. veto");
    DeclareHistogram1D(D_ENERGY_WITH_MWPC, energyBins, 
                      "Energy dssd/10 coin. mwpc");
    DeclareHistogram1D(D_ENERGY_WITH_VETO_MWPC, energyBins, 
                      "Energy dssd/10 coin. veto and mwpc");
    DeclareHistogram1D(D_ENERGY_NO_VETO_MWPC, energyBins, 
                      "Energy dssd/10 coin. no veto and mwpc");

    DeclareHistogram2D(DD_FRONTE__BACKE, energyBins2, energyBins2,
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
    vector< pair<StripEvent, bool> > xEventsTMatch;
    vector< pair<StripEvent, bool> > yEventsTMatch;

    for (vector<ChanEvent*>::iterator itx = xEvents.begin();
         itx != xEvents.end();
         ++itx) {
        StripEvent ev((*itx)->GetCalEnergy(), 
                      (*itx)->GetTime(),
                      (*itx)->GetChanID().GetLocation(),
                      (*itx)->IsSaturated());
        pair<StripEvent, bool> match(ev, false);
        xEventsTMatch.push_back(match);

        const Trace& trace = (*itx)->GetTrace();

        /** Handle additional pulses (no. 2, 3, ...) */
        int pulses = trace.GetValue("numPulses");
        for (int i = 1; i < pulses; ++i) {
            stringstream energyCalName;
            energyCalName << "filterEnergy" << i + 1 << "Cal";
            stringstream timeName;
            timeName << "filterTime" << i + 1;

            ev.pileup = true;

            StripEvent ev2;
            ev2.E = trace.GetValue(energyCalName.str());
            ev2.t = (trace.GetValue(timeName.str()) - 
                     trace.GetValue("filterTime") + ev.t);
            ev2.pos = ev.pos;
            ev2.sat = false;
            ev2.pileup = true;
            pair<StripEvent, bool> match2(ev2, false);
            xEventsTMatch.push_back(match2);

            if (i > 1) {
                stringstream ss;
                ss << "DSSD X, " << i + 1 << " pulse"
                << ", E = " << ev2.E
                << ", dt = " << ev2.t - ev.t;
                Messenger m;
                m.run_message(ss.str());
            }
        }

        for (vector<ChanEvent*>::iterator itx2 = itx;
            itx2 != xEvents.end();
            ++itx2) {
            int dx = abs( ev.pos -
                          (*itx2)->GetChanID().GetLocation());
            double dE = abs( ev.E -
                             (*itx2)->GetCalEnergy());
            plot(DD_DENERGY__DPOS_X_CORRELATED, dE, dx);
        }
    }

    for (vector<ChanEvent*>::iterator ity = yEvents.begin();
         ity != yEvents.end();
         ++ity) {
        StripEvent ev((*ity)->GetCalEnergy(), 
                      (*ity)->GetTime(),
                      (*ity)->GetChanID().GetLocation(),
                      (*ity)->IsSaturated());
        pair<StripEvent, bool> match(ev, false);
        yEventsTMatch.push_back(match);

        const Trace& trace = (*ity)->GetTrace();

        int pulses = trace.GetValue("numPulses");
        for (int i = 1; i < pulses; ++i) {
            stringstream energyCalName;
            energyCalName << "filterEnergy" << i + 1 << "Cal";
            stringstream timeName;
            timeName << "filterTime" << i + 1;

            ev.pileup = true;

            StripEvent ev2;
            ev2.E = trace.GetValue(energyCalName.str());
            ev2.t = (trace.GetValue(timeName.str()) - 
                     trace.GetValue("filterTime") + ev.t);
            ev2.pos = ev.pos;
            ev2.sat = false;
            ev2.pileup = true;
            pair<StripEvent, bool> match2(ev2, false);
            yEventsTMatch.push_back(match2);

            if (i > 1) {
                stringstream ss;
                ss << "DSSD Y, " << i + 1 << " pulse"
                << ", E = " << ev2.E
                << ", dt = " << ev2.t - ev.t;
                Messenger m;
                m.run_message(ss.str());
            }
        }

        for (vector<ChanEvent*>::iterator ity2 = ity;
            ity2 != yEvents.end();
            ++ity2) {
            int dy = abs( ev.pos -
                          (*ity2)->GetChanID().GetLocation());
            double dE = abs( ev.E -
                             (*ity2)->GetCalEnergy());
            plot(DD_DENERGY__DPOS_Y_CORRELATED, dE, dy);
        }
    }

    for (vector< pair<StripEvent, bool> >::iterator itx = xEventsTMatch.begin();
         itx != xEventsTMatch.end();
         ++itx) {
        double bestDtime = numeric_limits<double>::max();
        vector< pair<StripEvent, bool> >::iterator bestMatch =
            yEventsTMatch.end();
        for (vector< pair<StripEvent, bool> >::iterator ity = 
                                                     yEventsTMatch.begin();
            ity != yEventsTMatch.end();
            ++ity) 
        {
            // If already matched, skip
            if ((*ity).second)
                continue;

            double energyX = (*itx).first.E;
            double energyY = (*ity).first.E;

            /** If energies are in lower range and/or not satured
             *  check if delta energy condition is not met, 
             *  if not, skip this event
             *
             *  For high energy events and satured set 20 MeV
             *  energy for difference check. The calibration in this
             *  range is most likely imprecise, so one cannot correlate
             *  by energy difference.
             **/
            if ( (*itx).first.sat || energyX > highEnergyCut_ )
                energyX = 20000.0;
            if ( (*ity).first.sat || energyY > highEnergyCut_ )
                energyY = 20000.0;
            if ( abs(energyX - energyY) > deltaEnergy_)
                continue;

            double dTime = abs((*itx).first.t - (*ity).first.t) *
                                Globals::get()->clockInSeconds();
            if (dTime < bestDtime) {
                bestDtime = dTime;
                bestMatch = ity;
            }
        }
        if (bestDtime < timeWindow_) {
            xyEventsTMatch_.push_back(
                pair<StripEvent, StripEvent>((*itx).first, (*bestMatch).first));
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

    for (vector< pair<StripEvent, bool> >::iterator itx = xEventsTMatch.begin();
         itx != xEventsTMatch.end();
         ++itx) {
        if ((*itx).second)
            continue;
        int position = (*itx).first.pos;
        double energy = (*itx).first.E;
        plot(DD_ENERGY__POSX_T_MISSING, energy, position);
    }

    for (vector< pair<StripEvent, bool> >::iterator ity = yEventsTMatch.begin();
         ity != yEventsTMatch.end();
         ++ity) {
        if ((*ity).second)
            continue;
        int position = (*ity).first.pos;
        double energy = (*ity).first.E;
        plot(DD_ENERGY__POSY_T_MISSING, energy, position);
    }

    /**
     * Matching the front-back by the Energy of the event
     * Using the old style GetMaxEvent for comparison
     */
    if (xEvents.size() > 0 && yEvents.size() > 0) {
            ChanEvent* maxFront =
                event.GetSummary("dssd_back:dssd_back")->GetMaxEvent(true);
            ChanEvent* maxBack = 
                event.GetSummary("dssd_front:dssd_front")->GetMaxEvent(true);
            StripEvent evf(maxFront->GetCalEnergy(), 
                           maxFront->GetTime(),
                           maxFront->GetChanID().GetLocation(),
                           maxFront->IsSaturated());
            StripEvent evb(maxBack->GetCalEnergy(), 
                           maxBack->GetTime(),
                           maxBack->GetChanID().GetLocation(),
                           maxBack->IsSaturated());
        xyEventsEMatch_.push_back(pair<StripEvent, StripEvent>(evf, evb));
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
    vector<ChanEvent*> mwpcEvents = 
        event.GetSummary("si:si", true)->GetList();
    int mwpc = event.GetSummary("mcp", true)->GetMult();

    bool hasBeam = TreeCorrelator::get()->place("Beam")->status();

    plot(D_MWPC_MULTI, mwpc); 

    for (vector< pair<StripEvent, StripEvent> >::iterator it =
                                                 xyEventsTMatch_.begin();
         it != xyEventsTMatch_.end(); ++it)
    {
        double xEnergy = (*it).first.E;
        double yEnergy = (*it).second.E;

        /** If saturated set to 200 MeV **/
        if ((*it).first.sat && !(*it).second.sat)
            xEnergy = yEnergy;
        else if (!(*it).first.sat && (*it).second.sat)
            yEnergy = xEnergy;
        else if ((*it).first.sat && (*it).second.sat) {
            xEnergy = 200000.0;
            yEnergy = 200000.0;
        }

        int xPosition = (*it).first.pos;
        int yPosition = (*it).second.pos;

        double time = min((*it).first.t, (*it).second.t);

        plot(D_ENERGY_X, xEnergy / 10.0); 
        plot(D_ENERGY_Y, yEnergy / 10.0);

        plot(DD_FRONTE__BACKE, xEnergy / 100.0, yEnergy / 100.0);
        plot(DD_EVENT_ENERGY__X_POSITION, xEnergy, xPosition);
        plot(DD_EVENT_ENERGY__Y_POSITION, yEnergy, yPosition);

        plot(DD_EVENT_POSITION, xPosition, yPosition);

        double mwpcTime = numeric_limits<double>::max();
        for (vector<ChanEvent*>::iterator itm = mwpcEvents.begin();
            itm != mwpcEvents.end();
            ++itm) {
            double dt = abs(time - (*itm)->GetTime()) *
                        Globals::get()->clockInSeconds();
            if (dt < mwpcTime) {
                mwpcTime = dt;
            }
        }
        // Plot up to 3 us only
        if (mwpcTime < 3.0e-6) {
            int timeBin = int(mwpcTime / 1.0e-8);
            int energyBin = xEnergy / 100.0;
            plot(DD_ENERGY_DT__DSSD_MWPC, energyBin, timeBin);
        }

        if (vetoEvents.size() > 0) {
            for (vector<ChanEvent*>::iterator itv = vetoEvents.begin();
                itv != vetoEvents.end();
                ++itv) {
                double vetoEnergy = (*itv)->GetCalEnergy();
                plot(DD_DE_E__DSSD_VETO, (vetoEnergy + xEnergy) / 100.0,
                     xEnergy / 100.0);
            }
        }

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

        bool hasVeto = false;
        if (vetoEvents.size() > 0)
            hasVeto = true;

        if (hasVeto)
            plot(D_ENERGY_WITH_VETO, xEnergy / 10.0);
        if (mwpc > 0)
            plot(D_ENERGY_WITH_MWPC, xEnergy / 10.0);
        if (hasVeto && mwpc > 0)
            plot(D_ENERGY_WITH_VETO_MWPC, xEnergy / 10.0);
        if (!hasVeto && mwpc == 0)
            plot(D_ENERGY_NO_VETO_MWPC, xEnergy / 10.0);

        SheEvent event = SheEvent(xEnergy + escapeEnergy, time, mwpc,
                                  hasBeam, hasVeto, hasEscape, unknown);
        pickEventType(event);

        if (!event.get_beam())
            plot(D_ENERGY_DECAY_BEAMSTOP, event.get_energy());

        if (event.get_type() == heavyIon) {
            plot(DD_IMPLANT_POSITION, xPosition, yPosition);
            plot(D_ENERGY_IMPLANT, event.get_energy());
        }
        else if (event.get_type() == alpha) {
            plot(DD_DECAY_POSITION, xPosition, yPosition);
            plot(D_ENERGY_DECAY, event.get_energy() / 100.0);
        }
        else if (event.get_type() == lightIon) {
            plot(DD_LIGHT_POSITION, xPosition, yPosition);
            plot(D_ENERGY_LIGHT, event.get_energy() / 100.0);
        }
        else if (event.get_type() == unknown) {
            plot(DD_UNKNOWN_POSITION, xPosition, yPosition);
            plot(D_ENERGY_UNKNOWN, event.get_energy() / 100.0);
        }
        else if (event.get_type() == fission) {
            plot(DD_FISSION_POSITION, xPosition, yPosition);
            plot(D_ENERGY_FISSION, event.get_energy() / 100.0);
        }

        correlator_.add_event(event, xPosition, yPosition);

    }

    /** Old style max event for comparison */
    for (vector< pair<StripEvent, StripEvent> >::iterator it =
                                                 xyEventsEMatch_.begin();
         it != xyEventsEMatch_.end();
         ++it) {
        double xEnergy = (*it).first.E;
        double yEnergy = (*it).second.E;

        int xPosition = (*it).first.pos;
        int yPosition = (*it).second.pos;

        plot(DD_EVENT_POSITION_FROM_E, xPosition, yPosition);
        plot(DD_MAXEVENT_ENERGY__X_POSITION, xEnergy, xPosition);
        plot(DD_MAXEVENT_ENERGY__Y_POSITION, yEnergy, yPosition);

    }

    EndProcess();
    return true;
}


bool Dssd4SHEProcessor::pickEventType(SheEvent& event) {
    /**
     * Logic table (V - veto, M - mwpc, B - beam )
     * Logic state is converted into a numerical value N
     * like a binary number:
     *
     * V M B | N | decision
     * --------------------
     * 0 0 0 | 0 | unknown / alpha / fission (depending on energy)
     * 0 0 1 | 1 | -"-
     * 0 1 0 | 2 | unknown
     * 0 1 1 | 3 | heavyIon
     * 1 0 0 | 4 | unknown
     * 1 0 1 | 5 | lightIon
     * 1 1 0 | 6 | unknown
     * 1 1 1 | 7 | lightIon
     *
     **/

    int condition = 0;
    if (event.get_beam()) 
        condition += 1;
    if (event.get_mwpc() > 0) 
        condition += 2;
    if (event.get_veto()) 
        condition += 4;

    if (condition == 0) {
        double energy = event.get_energy();
        if (energy < highEnergyCut_)
            event.set_type(alpha);
        else if (energy < fissionEnergyCut_)
            event.set_type(unknown);
        else 
            event.set_type(fission);
    } 
    else if (condition == 1) {
        double energy = event.get_energy();
        if (energy < lowEnergyCut_)
            event.set_type(unknown);
        else if (energy < highEnergyCut_)
            event.set_type(alpha);
        else if (energy < fissionEnergyCut_)
            event.set_type(unknown);
        else 
            event.set_type(fission);
    } 
    else if (condition == 2 || 
             condition == 4 ||
             condition == 6) {
        event.set_type(unknown);
    } 
    else if (condition == 3) {
        event.set_type(heavyIon);
    }
    else if (condition == 5 || condition == 7) {
        event.set_type(lightIon);
    }
    else
        event.set_type(unknown);

    return true;
}


