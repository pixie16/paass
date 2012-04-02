/** \file QdcProcessor.hh
 *
 * Handle some QDC action
 */

#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>
#include <sstream>
#include <vector>

#include "QdcProcessor.hh"
#include "RawEvent.h"

using namespace std;

namespace dammIds {
    namespace qdc {
	const int QDC_JUMP = 20;
	const int LOC_SUM  = 18;

	const int D_QDCNORMN_LOCX          = 2300;
	const int D_QDCTOTNORM_LOCX        = 2460;
	const int D_INFO_LOCX              = 2480;
	const int DD_QDCN__QDCN_LOCX       = 2500;
	const int DD_QDCTOT__QDCTOT_LOCX   = 2660;
	const int DD_POSITION__ENERGY_LOCX = 2680;
	const int DD_POSITION              = 2699;
    }
}

const string QdcProcessor::configFile("qdc.txt");

/**
 * Initialize the qdc to handle ssd events
 */
QdcProcessor::QdcProcessor() : EventProcessor()
{
    name="qdc";
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
bool QdcProcessor::Init(DetectorDriver &driver)
{
    // Call the parent function to handle the standard stuff
    if (!EventProcessor::Init(driver)) {
	return false;
    }

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
    totLen = accumulate(qdcLen + 1, qdcLen + 8, 0);

    in >> whichQdc >> posScale;

    int numLocationsRead = 0;
    while (true) {
	int location;
	in >> location;
	if (in.eof()) {
	    // place this here so a trailing newline is okay in the config file
	    break;
	}
	in >> minNormQdc[location-1] >> maxNormQdc[location-1];
	numLocationsRead++;
    }
    if (numLocationsRead != numLocations) {
	cerr << "Only read QDC position calibration information from "
	     << numLocationsRead << " locations where a total of "
	     << numLocations << " was expected!" << endl;
	cerr << "  Disabling QDC processor." << endl;
	return (initDone = false);
    }
    
    cout << "QDC processor initialized with " << numLocations 
	 << " locations operating on " << numQdcs << " QDCs" << endl;
    cout << "  QDC #" << whichQdc << " being used for position determination."
	 << endl;

    return true;
}

/**
 *  Declare all the plots we plan on using (part of dammIds::qdc namespace)
 */
void QdcProcessor::DeclarePlots(void) const
{
    using namespace dammIds::qdc;

    const int qdcBins = S8;
    const int normBins = SA;
    const int infoBins = S3;
    const int locationBins = S4;
    const int positionBins = S6;
    const int energyBins   = SE;

    for (int i=0; i < numLocations; i++) {	
	stringstream str;
	for (int j=1; j < 8; j++) {
	    str << "QDC " << j << ", T/B LOC " << i;
	    DeclareHistogram2D(DD_QDCN__QDCN_LOCX + QDC_JUMP * j + i , qdcBins, qdcBins, str.str().c_str() );
	    str.str("");
	    str << "QDC " << j << " NORM T/B LOC" << i;
	    DeclareHistogram1D(D_QDCNORMN_LOCX + QDC_JUMP * j + i, normBins, str.str().c_str() );
	    str.str("");

	    if (i == 0) {
		// declare only once
		str << "ALL QDC T/B" << j;
		DeclareHistogram2D(DD_QDCN__QDCN_LOCX + QDC_JUMP * j + LOC_SUM, 
				   qdcBins, qdcBins, str.str().c_str() );
		str.str("");
		str << "ALL QDC " << j << " NORM T/B";   
		DeclareHistogram1D(D_QDCNORMN_LOCX + QDC_JUMP * j + LOC_SUM, 
				   normBins, str.str().c_str() );
	    }
	}
	str << "QDCTOT T/B LOC " << i;
	DeclareHistogram2D(DD_QDCTOT__QDCTOT_LOCX + i, qdcBins, qdcBins, str.str().c_str() );
	str.str("");

	str << "QDCTOT NORM T/B LOC " << i;
	DeclareHistogram1D(D_QDCTOTNORM_LOCX + i, normBins, str.str().c_str());
	str.str("");

	str << "INFO LOC " << i;
	DeclareHistogram1D(D_INFO_LOCX + i, infoBins, str.str().c_str());
	str.str("");
	
	str << "Energy vs. position, loc " << i;
	DeclareHistogram2D(DD_POSITION__ENERGY_LOCX + i, positionBins, energyBins, str.str().c_str());
	str.str("");
    }
    DeclareHistogram2D(DD_QDCTOT__QDCTOT_LOCX + LOC_SUM, qdcBins, qdcBins, "ALL QDCTOT T/B");
    DeclareHistogram1D(D_QDCTOTNORM_LOCX + LOC_SUM, normBins, "ALL QDCTOT NORM T/B");
    DeclareHistogram2D(DD_POSITION__ENERGY_LOCX + LOC_SUM, positionBins, energyBins, "All energy vs. position");

    DeclareHistogram1D(D_INFO_LOCX + LOC_SUM, infoBins, "ALL INFO");
    DeclareHistogram2D(DD_POSITION, locationBins, positionBins, "Qdc Position");
}

/**
 *  Process the QDC data involved in top/bottom side for a strip 
 *  Note QDC lengths are HARD-CODED at the moment for the plots and to determine the position
 */
bool QdcProcessor::Process(RawEvent &event)
{
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

    for (vector<ChanEvent*>::const_iterator it=allEvents.begin();
	 it != allEvents.end(); it++) {
	ChanEvent *sumchan   = *it;

	const ChanEvent *top    = FindMatchingEdge(sumchan, topEvents.begin(), topEvents.end());
	const ChanEvent *bottom = FindMatchingEdge(sumchan, bottomEvents.begin(), bottomEvents.end());

	int location = sumchan->GetChanID().GetLocation();
	if (top == NULL || bottom == NULL) {
	    using namespace dammIds::qdc;

	    if (top == NULL) {
		plot(D_INFO_LOCX + location - 1, 3);
		plot(D_INFO_LOCX + LOC_SUM, 3);
	    }

	    if (bottom == NULL) {
		plot(D_INFO_LOCX + location - 1, 4);
		plot(D_INFO_LOCX + LOC_SUM, 4);
	    }
	    continue;
	}

	/* Make sure we get the same match going backwards to insure there is only one in the vector */
	/* Isn't working due to some C++ STL mumbo jumbo -- normal iterators and reverse iterators not interchangeable 
	if ( FindMatchingEdge(sumchan, topEvents.rbegin(), topEvents.rend()) != top) {
	    cout << "Multiple top edges found for sum location " << location << endl; 
	}
	if ( FindMatchingEdge(sumchan, bottomEvents.rbegin(), bottomEvents.rend()) != bottom) {
	    cout << "Multiple top edges found for sum location " << location << endl; 
	}
	*/
	using namespace dammIds::qdc;
	
	float topQdc[numQdcs];
	float bottomQdc[numQdcs];
	float topQdcTot = 0;
	float bottomQdcTot = 0;
	float position = NAN;
	
	if (bottom->GetQdcValue(0) == U_DELIMITER || top->GetQdcValue(0) == U_DELIMITER) {
	    // This happens naturally for traces which have double triggers
	    //   Onboard DSP does not write QDCs in this case
#ifdef VERBOSE
	    cout << "SSD strip edges are missing QDC information for location " << location << endl;
#endif
	    if (top->GetQdcValue(0) == U_DELIMITER) {
		plot(D_INFO_LOCX + location - 1, 1);
		plot(D_INFO_LOCX + LOC_SUM, 1);
	    }
	    if (bottom->GetQdcValue(0) == U_DELIMITER) {
		plot(D_INFO_LOCX + location - 1, 2);
		plot(D_INFO_LOCX + LOC_SUM, 2);		
	    }
	    continue;
	}
	plot(D_INFO_LOCX + location - 1, 0); // good stuff
	plot(D_INFO_LOCX + LOC_SUM     , 0); // good stuff

	for (int i=1; i < numQdcs; i++) {		
	    topQdc[i] = top->GetQdcValue(i);
	    topQdc[i] -= top->GetQdcValue(0) * qdcLen[i] / qdcLen[0];
	    topQdcTot += topQdc[i];
	    topQdc[i] /= qdcLen[i];		
	    
	    bottomQdc[i] = bottom->GetQdcValue(i);
	    bottomQdc[i] -= bottom->GetQdcValue(0) * qdcLen[i] / qdcLen[0];
	    bottomQdcTot += bottomQdc[i];
	    bottomQdc[i] /= qdcLen[i];
	    
	    plot(DD_QDCN__QDCN_LOCX + QDC_JUMP * i + location - 1, topQdc[i], bottomQdc[i]);
	    plot(DD_QDCN__QDCN_LOCX + QDC_JUMP * i + LOC_SUM, topQdc[i], bottomQdc[i]);
	    float frac = topQdc[i] / (topQdc[i] + bottomQdc[i]) * 1000.; // per mil
	    plot(D_QDCNORMN_LOCX + QDC_JUMP * i + location - 1, frac);
	    plot(D_QDCNORMN_LOCX + QDC_JUMP * i + LOC_SUM, frac);
	    if (i == whichQdc) {
		position = posScale * (frac - minNormQdc[location]) / 
		    (maxNormQdc[location] - minNormQdc[location]);
		sumchan->GetTrace().InsertValue("position", position);
		plot(DD_POSITION, location - 1, position);
		plot(DD_POSITION__ENERGY_LOCX + location - 1, position, sumchan->GetCalEnergy());
		plot(DD_POSITION__ENERGY_LOCX + LOC_SUM, position, sumchan->GetCalEnergy());
	    }
	} // end loop over qdcs
	topQdcTot    /= totLen;
	bottomQdcTot /= totLen;
	
	plot(DD_QDCTOT__QDCTOT_LOCX + location - 1, topQdcTot, bottomQdcTot);
	plot(DD_QDCTOT__QDCTOT_LOCX + LOC_SUM, topQdcTot, bottomQdcTot);
    } // end iteration over sum events

    EndProcess();
    return true;
}

ChanEvent* QdcProcessor::FindMatchingEdge(ChanEvent *match,
					  vector<ChanEvent*>::const_iterator begin,
					  vector<ChanEvent*>::const_iterator end) const
{
    const float timeCut = 100.; // maximum difference between edge and sum timestamps
	
    for (;begin < end; begin++) {
	if ( (*begin)->GetChanID().GetLocation() == match->GetChanID().GetLocation() &&
	     abs( (*begin)->GetTime() - match->GetTime() ) < timeCut ) {
	    return *begin;
	}
    }
    return NULL;
}

