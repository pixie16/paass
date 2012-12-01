/** \file PositionProcessor.cpp
 * \brief Handle some QDC action to determine positions in a strip detector
 */

#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>
#include <sstream>
#include <vector>

#include "PathHolder.hpp"
#include "PositionProcessor.hpp"
#include "DetectorLibrary.hpp"
#include "RawEvent.hpp"

using namespace std;

namespace dammIds {
    namespace position {
        const int QDC_JUMP = 20;
        const int LOC_SUM  = 18;

        const int D_QDCNORMN_LOCX          = 0;
        const int D_QDCTOTNORM_LOCX        = 160;
        const int D_INFO_LOCX              = 180;
        const int DD_QDCN__QDCN_LOCX       = 200;
        const int DD_QDCTOT__QDCTOT_LOCX   = 360;
        const int DD_POSITION__ENERGY_LOCX = 380;
        const int DD_POSITION              = 399;

        const int DD_QDCR2__QDCR1_LOCX     = 400;
        const int DD_QDCR2__QDCR3_LOCX     = 420;
        const int DD_QDCR2__QDCR4_LOCX     = 440;
        const int DD_QDC1R__POS_LOCX       = 460;
        const int DD_QDC2R__POS_LOCX       = 480;
        const int DD_QDC3R__POS_LOCX       = 500;
        const int DD_QDC4R__POS_LOCX       = 520;

	enum EInfo {INFO_OKAY = 0,
		    INFO_MISSING_BOTTOM_QDC = 1,
		    INFO_MISSING_TOP_QDC = 2,
		    INFO_MULTIPLE_BOTTOM = 3,
		    INFO_MULTIPLE_TOP = 4,
		    INFO_MISSING_BOTTOM = 5,
		    INFO_MISSING_TOP = 6,
		    INFO_NOISE = 7
	};
    }
}


using namespace dammIds::position;

/**
 * Initialize the qdc to handle ssd events
 */
PositionProcessor::PositionProcessor() : EventProcessor(OFFSET, RANGE) {
    name="position";
    associatedTypes.insert("ssd");
}

/**
 * Reads in QDC parameters from an input file
 *   The file format allows comment lines at the beginning
 *   Followed by QDC lengths 
 *   Which QDC to use for position calculation
 *     followed by the amount to scale the [0,1] result by to physical units
 *   And min and max values of the normalized QDC for each location in form:
 *      <location> <min> <max>
 *   Note that QDC 0 is considered to be a baseline section of the trace for
 *     baseline removal for the other QDCs
 */
bool PositionProcessor::Init(RawEvent& rawev)
{
    // Call the parent function to handle the standard stuff
    if (!EventProcessor::Init(rawev)) {
        return false;
    }

    DetectorLibrary* modChan = DetectorLibrary::get();

    int numLocationsTop    = modChan->GetNextLocation("ssd", "top");
    int numLocationsBottom = modChan->GetNextLocation("ssd", "bottom");
    if (numLocationsTop != numLocationsBottom) {
        cerr << "Number of top positions (" << numLocationsTop 
            << ") does not match number of bottom positions ("
            << numLocationsBottom << ") in map!" << endl;
        cerr << "  Disabling QDC processor." << endl;

        return (initDone = false);
    }
    numLocations = numLocationsTop;
    if (numLocations > maxNumLocations) {
        cerr << "Number of positions (" << numLocations 
            << ") is larger then maximum number of supported positions ("
            << maxNumLocations << ") in PositionProcessor" << endl;
        cerr << "  Disabling QDC processor." << endl;
        return (initDone = false);
    }
    minNormQdc.resize(numLocations);
    maxNormQdc.resize(numLocations);

    PathHolder* conf_path = new PathHolder();
    string configFile = conf_path->GetFullPath("qdc.txt");
    delete conf_path;

    ifstream in(configFile.c_str());
    if (!in) {
        cerr << "Failed to open the QDC parameter file, QDC processor disabled." << endl;
        return (initDone = false);	
    }

    // Ignore any lines at the beginning that don't have a digit
    // This allows for some comments at the beginning of the file
    int linesIgnored = 0;
    while ( !isdigit(in.peek()) ) {
        in.ignore(1000, '\n');
        linesIgnored++;
    }
    if (linesIgnored != 0) {
        cout << "Ignored " << linesIgnored << " comment lines in " 
            << configFile << endl;
    }

    for (int i=0; i < numQdcs; i++) 
        in >> qdcLen[i];
    partial_sum(qdcLen, qdcLen + numQdcs, qdcPos);
    totLen = qdcPos[numQdcs - 1]  - qdcLen[0];
    // totLen = accumulate(qdcLen + 1, qdcLen + 8, 0);
    
    in >> whichQdc >> posScale;
    
    int numLocationsRead = 0;
    while (true) {
        int location;
        in >> location;
        if (in.eof()) {
            // place this here so a trailing newline is okay in the config file
            break;
        }
        in >> minNormQdc[location] >> maxNormQdc[location];
        numLocationsRead++;
    }

    if (numLocationsRead != numLocations) {
        cerr << "Only read QDC position calibration information from "
            << numLocationsRead << " locations where a total of "
            << numLocations << " was expected!" << endl;
        cerr << "  Disabling position processor." << endl;
        return (initDone = false);
    }
    
    cout << "QDC processor initialized with " << numLocations 
         << " locations operating on " << numQdcs << " QDCs" << endl;
    cout << "QDC #" << whichQdc << " being used for position determination."
         << endl;

    return true;
}

/**
 *  Declare all the plots we plan on using (part of dammIds::qdc namespace)
 */
void PositionProcessor::DeclarePlots() {
    using namespace dammIds::position;

    const int qdcBins = S7;
    const int normBins = SA;
    const int infoBins = S3;
    const int locationBins = S4;
    const int positionBins = S6;
    const int energyBins   = SA;

    for (int i = 0; i < maxNumLocations; ++i) {	
        stringstream str;

        for (int j = 1; j < numQdcs; ++j) {
            str << "QDC " << j << ", T/B LOC " << i;
            histo.DeclareHistogram2D(DD_QDCN__QDCN_LOCX + QDC_JUMP * j + i , 
                               qdcBins, qdcBins, str.str().c_str() );
            str.str("");
            str << "QDC " << j << " NORM T/B LOC" << i;
            histo.DeclareHistogram1D(D_QDCNORMN_LOCX + QDC_JUMP * j + i,
                               normBins, str.str().c_str() );
            str.str("");
            if (i == 0) {
                // declare only once
                str << "ALL QDC T/B" << j;
                histo.DeclareHistogram2D(DD_QDCN__QDCN_LOCX + QDC_JUMP * j + LOC_SUM, 
                        qdcBins, qdcBins, str.str().c_str() );
                str.str("");
                str << "ALL QDC " << j << " NORM T/B";   
                histo.DeclareHistogram1D(D_QDCNORMN_LOCX + QDC_JUMP * j + LOC_SUM, 
                        normBins, str.str().c_str() );
            }
        }

        str << "QDCTOT T/B LOC " << i;
        histo.DeclareHistogram2D(DD_QDCTOT__QDCTOT_LOCX + i, qdcBins, qdcBins, str.str().c_str() );
        str.str("");

        str << "QDCTOT NORM T/B LOC " << i;
        histo.DeclareHistogram1D(D_QDCTOTNORM_LOCX + i, normBins, str.str().c_str());
        str.str("");

        str << "INFO LOC " << i;
        histo.DeclareHistogram1D(D_INFO_LOCX + i, infoBins, str.str().c_str());
        str.str("");
        
        str << "Energy vs. position, loc " << i;
        histo.DeclareHistogram2D(DD_POSITION__ENERGY_LOCX + i, positionBins, energyBins, str.str().c_str());
        str.str("");

        str << "QDCR2 vs QDCR1, loc " << i;
        histo.DeclareHistogram2D(DD_QDCR2__QDCR1_LOCX + i, SA, SA, str.str().c_str());
        str.str("");

        str << "QDCR2 vs QDCR3, loc " << i;
        histo.DeclareHistogram2D(DD_QDCR2__QDCR3_LOCX + i, SA, SA, str.str().c_str());
        str.str("");

        str << "QDCR2 vs QDCR4, loc " << i;
        histo.DeclareHistogram2D(DD_QDCR2__QDCR4_LOCX + i, SA, SA, str.str().c_str());
        str.str("");

        str << "QDC1R vs Pos, loc " << i;
        histo.DeclareHistogram2D(DD_QDC1R__POS_LOCX + i, SA, SA, str.str().c_str());
        str.str("");

        str << "QDC2R vs Pos, loc " << i;
        histo.DeclareHistogram2D(DD_QDC2R__POS_LOCX + i, SA, SA, str.str().c_str());
        str.str("");

        str << "QDC3R vs Pos, loc " << i;
        histo.DeclareHistogram2D(DD_QDC3R__POS_LOCX + i, SA, SA, str.str().c_str());
        str.str("");

        str << "QDC4R vs Pos, loc " << i;
        histo.DeclareHistogram2D(DD_QDC4R__POS_LOCX + i, SA, SA, str.str().c_str());
        str.str("");
    }
    histo.DeclareHistogram2D(DD_QDCTOT__QDCTOT_LOCX + LOC_SUM, qdcBins, qdcBins, "ALL QDCTOT T/B");
    histo.DeclareHistogram1D(D_QDCTOTNORM_LOCX + LOC_SUM, normBins, "ALL QDCTOT NORM T/B");
    histo.DeclareHistogram2D(DD_POSITION__ENERGY_LOCX + LOC_SUM, positionBins, energyBins, "All energy vs. position");

    histo.DeclareHistogram1D(D_INFO_LOCX + LOC_SUM, infoBins, "ALL INFO");
    histo.DeclareHistogram2D(DD_POSITION, locationBins, positionBins, "Qdc Position", 1, "pos");
}

/**
 *  Process the QDC data involved in top/bottom side for a strip 
 *  Note QDC lengths are HARD-CODED at the moment for the plots and to determine the position
 */
bool PositionProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return false;

    static const vector<ChanEvent*> &sumEvents = 
	event.GetSummary("ssd:sum", true)->GetList();
    static const vector<ChanEvent*> &digisumEvents =
	event.GetSummary("ssd:digisum", true)->GetList();
    static const vector<ChanEvent*> &topEvents =
	event.GetSummary("ssd:top", true)->GetList();
    static const vector<ChanEvent*> &bottomEvents =
	event.GetSummary("ssd:bottom", true)->GetList();

    vector<ChanEvent *> allEvents;
    // just add in the digisum events for now
    allEvents.insert(allEvents.begin(), digisumEvents.begin(), digisumEvents.end());
    allEvents.insert(allEvents.begin(), sumEvents.begin(), sumEvents.end());

    for (vector<ChanEvent*>::const_iterator it = allEvents.begin();
	     it != allEvents.end(); ++it) {
         ChanEvent *sumchan   = *it;

        int location = sumchan->GetChanID().GetLocation();

        // Don't waste our time with noise events
        if ( (*it)->GetEnergy() < 10. || (*it)->GetEnergy() > 16374 ) {
            using namespace dammIds::position;

            plot(D_INFO_LOCX + location, INFO_NOISE);
            plot(D_INFO_LOCX + LOC_SUM , INFO_NOISE);
            continue;
        }

        const ChanEvent *top    = FindMatchingEdge(sumchan, topEvents.begin(), topEvents.end());
        const ChanEvent *bottom = FindMatchingEdge(sumchan, bottomEvents.begin(), bottomEvents.end());


        if (top == NULL || bottom == NULL) {
            using namespace dammIds::position;

            if (top == NULL) {
                // [6] -> Missing top
                plot(D_INFO_LOCX + location, INFO_MISSING_TOP);
                plot(D_INFO_LOCX + LOC_SUM, INFO_MISSING_TOP);
            }

            if (bottom == NULL) {
                // [5] -> Missing bottom
                plot(D_INFO_LOCX + location, INFO_MISSING_BOTTOM);
                plot(D_INFO_LOCX + LOC_SUM, INFO_MISSING_BOTTOM);
            }
            continue;
        }

        /* Make sure we get the same match going backwards to insure there is only one in the vector */
        if ( FindMatchingEdge(sumchan, topEvents.rbegin(), topEvents.rend()) != top) {
            using namespace dammIds::position;
            // [4] -> Multiple top
            plot(D_INFO_LOCX + location, INFO_MULTIPLE_TOP);
            plot(D_INFO_LOCX + LOC_SUM, INFO_MULTIPLE_TOP);
            continue;
        }
        if ( FindMatchingEdge(sumchan, bottomEvents.rbegin(), bottomEvents.rend()) != bottom) {
            using namespace dammIds::position;
            // [3] -> Multiple bottom
            plot(D_INFO_LOCX + location, INFO_MULTIPLE_BOTTOM);
            plot(D_INFO_LOCX + LOC_SUM, INFO_MULTIPLE_BOTTOM);
            continue;
        }

        using namespace dammIds::position;
        
        float topQdc[numQdcs];
        float bottomQdc[numQdcs];
        float topQdcTot = 0;
        float bottomQdcTot = 0;
        float position = NAN;
        
        topQdc[0] = top->GetQdcValue(0);
        bottomQdc[0] = bottom->GetQdcValue(0);
        if (bottomQdc[0] == U_DELIMITER || topQdc[0] == U_DELIMITER) {
            // This happens naturally for traces which have double triggers
            //   Onboard DSP does not write QDCs in this case
#ifdef VERBOSE
            cout << "SSD strip edges are missing QDC information for location " << location << endl;
#endif
            if (topQdc[0] == U_DELIMITER) {
                // [2] -> Missing top QDC
                plot(D_INFO_LOCX + location, INFO_MISSING_TOP_QDC);
                plot(D_INFO_LOCX + LOC_SUM, INFO_MISSING_TOP_QDC);
                // Recreate qdc from trace
                if ( !top->GetTrace().empty() ) {
                    topQdc[0] = accumulate(top->GetTrace().begin(), top->GetTrace().begin() + qdcLen[0], 0);
                } else {
                    topQdc[0] = 0;
                }
            }
            if (bottomQdc[0] == U_DELIMITER) {
                // [1] -> Missing bottom QDC
                plot(D_INFO_LOCX + location, INFO_MISSING_BOTTOM_QDC);
                plot(D_INFO_LOCX + LOC_SUM, INFO_MISSING_BOTTOM_QDC);
                // Recreate qdc from trace
                if ( !bottom->GetTrace().empty() ) {
                    bottomQdc[0] = accumulate(bottom->GetTrace().begin(), bottom->GetTrace().begin() + qdcLen[0], 0);
                } else {
                    bottomQdc[0] = 0;
                }
            }
            if ( topQdc[0] == 0 || bottomQdc[0] == 0 ) {
                continue;
            }
        }
        // [0] -> good stuff
        plot(D_INFO_LOCX + location, INFO_OKAY);
        plot(D_INFO_LOCX + LOC_SUM, INFO_OKAY);


        for (int i = 1; i < numQdcs; ++i) {		
            if (top->GetQdcValue(i) == U_DELIMITER) {
                // Recreate qdc from trace
                topQdc[i] = accumulate(top->GetTrace().begin() + qdcPos[i-1],
                top->GetTrace().begin() + qdcPos[i], 0);
            } else {
                topQdc[i] = top->GetQdcValue(i);
            }

            topQdc[i] -= topQdc[0] * qdcLen[i] / qdcLen[0];
            topQdcTot += topQdc[i];
            topQdc[i] /= qdcLen[i];		
            
            if (bottom->GetQdcValue(i) == U_DELIMITER) {
                // Recreate qdc from trace
                bottomQdc[i] = accumulate(bottom->GetTrace().begin() + qdcPos[i-1],
                                          bottom->GetTrace().begin() + qdcPos[i], 0);
            } else {
                bottomQdc[i] = bottom->GetQdcValue(i);
            }

            bottomQdc[i] -= bottomQdc[0] * qdcLen[i] / qdcLen[0];
            bottomQdcTot += bottomQdc[i];
            bottomQdc[i] /= qdcLen[i];
            
            plot(DD_QDCN__QDCN_LOCX + QDC_JUMP * i + location, topQdc[i] + 10, bottomQdc[i] + 10);
            plot(DD_QDCN__QDCN_LOCX + QDC_JUMP * i + LOC_SUM, topQdc[i], bottomQdc[i]);
            
            float frac = topQdc[i] / (topQdc[i] + bottomQdc[i]) * 1000.; // per mil
            
            plot(D_QDCNORMN_LOCX + QDC_JUMP * i + location, frac);
            plot(D_QDCNORMN_LOCX + QDC_JUMP * i + LOC_SUM, frac);
            if (i == whichQdc) {
                position = posScale * (frac - minNormQdc[location]) / 
                    (maxNormQdc[location] - minNormQdc[location]);		
                sumchan->GetTrace().InsertValue("position", position);
                plot(DD_POSITION__ENERGY_LOCX + location, position, sumchan->GetCalEnergy());
                plot(DD_POSITION__ENERGY_LOCX + LOC_SUM, position, sumchan->GetCalEnergy());
            }
            if (i == 6 && !sumchan->IsSaturated()) {
                // compare the long qdc to the energy
                int qdcSum = topQdc[i] + bottomQdc[i];
                
                // MAGIC NUMBERS HERE, move to qdc.txt
                if (qdcSum < 1000 && sumchan->GetCalEnergy() > 15000) {
                    sumchan->GetTrace().InsertValue("badqdc", 1);
                } else if ( !isnan(position) ) {
                    plot(DD_POSITION, location, position);
                }
            }
        } // end loop over qdcs
        // KM QDC - QDC correlations
        double ratio[4] = {0};
        for (int i = 1; i < 5; ++i) {
                ratio[i - 1] = topQdc[i] / (bottomQdc[i] + topQdc[i]) * 1000.0;
        }

        plot(DD_QDCR2__QDCR1_LOCX + location, ratio[1], ratio[0]);
        plot(DD_QDCR2__QDCR3_LOCX + location, ratio[1], ratio[2]);
        plot(DD_QDCR2__QDCR4_LOCX + location, ratio[1], ratio[3]);

        plot(DD_QDC1R__POS_LOCX + location, ratio[0], position * 10.0 + 200.0);
        plot(DD_QDC2R__POS_LOCX + location, ratio[1], position * 10.0 + 200.0);
        plot(DD_QDC3R__POS_LOCX + location, ratio[2], position * 10.0 + 200.0);
        plot(DD_QDC4R__POS_LOCX + location, ratio[3], position * 10.0 + 200.0);

        topQdcTot    /= totLen;
        bottomQdcTot /= totLen;
        
        plot(DD_QDCTOT__QDCTOT_LOCX + location, topQdcTot, bottomQdcTot);
        plot(DD_QDCTOT__QDCTOT_LOCX + LOC_SUM, topQdcTot, bottomQdcTot);
    } // end iteration over sum events

   // test

    EndProcess();

    return true;
}

ChanEvent* PositionProcessor::FindMatchingEdge(ChanEvent *match,
					  vector<ChanEvent*>::const_iterator begin,
					  vector<ChanEvent*>::const_iterator end) const
{
    for (;begin < end; ++begin) {
        if ( (*begin)->GetChanID().GetLocation() == match->GetChanID().GetLocation() &&
            abs( (*begin)->GetTime() - match->GetTime() ) < matchingTimeCut ) {
            return *begin;
        }
    }
    return NULL;
}

ChanEvent* PositionProcessor::FindMatchingEdge(ChanEvent *match,
					  vector<ChanEvent*>::const_reverse_iterator begin,
					  vector<ChanEvent*>::const_reverse_iterator end) const
{
    for (;begin < end; ++begin) {
        if ( (*begin)->GetChanID().GetLocation() == match->GetChanID().GetLocation() &&
            abs( (*begin)->GetTime() - match->GetTime() ) < matchingTimeCut ) {
            return *begin;
        }
    }
    return NULL;
}
