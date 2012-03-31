/** \file QdcProcessor.hh
 *
 * Handle some QDC action
 */

#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <vector>

#include "QdcProcessor.hh"
#include "RawEvent.h"

using namespace std;

QdcProcessor::QdcProcessor() : EventProcessor()
{
    name="qdc";
    associatedTypes.insert("ssd");
}

void QdcProcessor::DeclarePlots(void) const
{
    const int qdcBins = S8;

    for (int i=0; i < 16; i++) {	
	stringstream str;
	for (int j=1; j < 8; j++) {
	    str << "QDC " << j << ", T/B LOC " << i;
	    DeclareHistogram2D(2500 + 20 * j + i , qdcBins, qdcBins, str.str().c_str() );
	    str.str("");
	    if (i == 0) {
		// declare once
		str << "ALL QDC T/B" << j;
		DeclareHistogram2D(2516 + 20 * j, qdcBins, qdcBins, str.str().c_str() );
		str.str("");
		str << "ALL QDC NORM T/B" << j;
		DeclareHistogram1D(2517 + 20 * j, SA, str.str().c_str() );
		str.str("");
	    }
	}
	str << "QDCTOT T/B LOC " << i;
	DeclareHistogram2D(2660 + i, qdcBins, qdcBins, str.str().c_str() );
	str.str("");
    }
    DeclareHistogram2D(2676, qdcBins, qdcBins, "ALL QDCTOT T/B");
    DeclareHistogram1D(2677, SA, "ALL QDCTOT NORM T/B");
}

bool QdcProcessor::Process(RawEvent &event)
{
    if (!EventProcessor::Process(event))
	return false;

    static const vector<ChanEvent*> &implantEvents = 
	event.GetSummary("ssd:implant", true)->GetList();
    static const vector<ChanEvent*> &topEvents =
	event.GetSummary("ssd:top", true)->GetList();
    static const vector<ChanEvent*> &bottomEvents =
	event.GetSummary("ssd:bottm", true)->GetList();

    for (vector<ChanEvent*>::const_iterator it=implantEvents.begin();
	 it != implantEvents.end(); it++) {
	ChanEvent *chan = *it;
	
	const Identifier& id = chan->GetChanID();
	int location = id.GetLocation();
	double time = chan->GetTime();

	if (chan->GetQdcValue(0) == U_DELIMITER)
	    continue;
	/*
	// find the matching events for the edges
	for (vector<ChanEvent*>::const_iterator itEdge = edgeEvents.begin();
	     itEdge != edgeEvents.end(); itEdge++) {
	    ChanEvent *edge = *itEdge;

	    const Identifier& edgeid = edge->GetChanID();
	    double edgeTime = edge->GetTime();
	    int edgeLocation = edgeid.GetLocation();

	    if ( (edgeLocation % 8) != location)
		continue;
	    if (abs(edgeTime - time) > 100)
		continue;

	    if (edgeLocation == location)
		cout << "      RGHT QDC: ";
	    if (edgeLocation == location + 8)
		cout << "      LEFT QDC: ";
	    for (int i=0; i < 8; i++) {
		cout << setw(6) << edge->GetQdcValue(i);
	    }
	    cout << endl;
	}
	*/
    }

    float qdcLen[8] = {40., 30., 30., 50., 50., 100., 600., 50.};
    float totLen = accumulate(qdcLen + 1, qdcLen + 8, 0);

    // for each location
    for (int loc=1; loc <= 16; loc++) {
	float topQdc[8] = {-1};
	float bottomQdc[8] = {-1};
	float topQdcTot = -1;
	float bottomQdcTot = -1;

	for (vector<ChanEvent*>::const_iterator itTop = topEvents.begin();
	     itTop != topEvents.end(); itTop++) {
	    ChanEvent *edge = *itTop;
	
	    const Identifier& edgeid = edge->GetChanID();
	    double edgeTime = edge->GetTime();
	    if (edgeid.GetLocation() != loc) 
		continue;	    
	    if (edge->GetQdcValue(0) == U_DELIMITER)
		continue;

	    topQdcTot = 0;
	    for (int i=1; i < 8; i++) {		
		topQdc[i] = edge->GetQdcValue(i);
		topQdc[i] -= edge->GetQdcValue(0) * qdcLen[i] / qdcLen[0];
		topQdcTot += topQdc[i];
		topQdc[i] /= qdcLen[i];		
	    }
	    topQdcTot /= totLen;
	}
	for (vector<ChanEvent*>::const_iterator itBottom = bottomEvents.begin();
	     itBottom != bottomEvents.end(); itBottom++) {
	    ChanEvent *edge = *itBottom;
	
	    const Identifier& edgeid = edge->GetChanID();
	    double edgeTime = edge->GetTime();
	    if (edgeid.GetLocation() != loc) 
		continue;	    
	    if (edge->GetQdcValue(0) == U_DELIMITER)
		continue;
	    bottomQdcTot = 0;
	    for (int i=1; i < 8; i++) {		
		bottomQdc[i] = edge->GetQdcValue(i);
		bottomQdc[i] -= edge->GetQdcValue(0) * qdcLen[i] / qdcLen[0];
		bottomQdcTot += bottomQdc[i];
		bottomQdc[i] /= qdcLen[i];
	    }
	    bottomQdcTot /= totLen;
	}
	if (topQdcTot != -1 && bottomQdcTot != -1) {
	    for (int j=1; j < 8; j++) {
		plot(2500 + 20 * j + loc - 1, topQdc[j], bottomQdc[j]);
		plot(2500 + 20 * j + 16, topQdc[j], bottomQdc[j]);
		plot(2500 + 20 * j + 17, topQdc[j] * 1000. / (topQdc[j] + bottomQdc[j])); // per mil
	    }
	    plot(2660 + loc - 1, topQdcTot, bottomQdcTot);
	    plot(2676, topQdcTot, bottomQdcTot);
	    plot(2677, topQdcTot * 1000. / (topQdcTot + bottomQdcTot));
	}
    }

    EndProcess();
    return true;
}

