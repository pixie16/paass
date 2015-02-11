/** \file IonChamberProcessor.cpp
 * \brief implementation for ion chambers
 */
#include <vector>

#include <cmath>

#include "DammPlotIds.hpp"
#include "Globals.hpp"
#include "RawEvent.hpp"
#include "IonChamberProcessor.hpp"

#ifdef useroot
#include <TBranch.h>
#include <TTree.h>
#endif

using namespace std;

const double IonChamberProcessor::minTime =
                                18.0e-6 / Globals::get()->clockInSeconds();

using namespace dammIds::ionChamber;

namespace dammIds {
  namespace ionChamber {
    const int D_ENERGYSUM     = 0;//!< Energy Sum
    const int D_ENERGYTHREE_GROUPX = 10; //!< Energy Three + starting det
    const int D_ENERGYTWO_GROUPX   = 20; //!< Energy Two  + starting det
    const int D_DTIME_DETX    = 30; //!< Time + detector num
    const int D_RATE_DETX     = 40; //!< Rates + detector num
    const int DD_ESUM__ENERGY_DETX   = 50; //!< Esum vs. Energy + detector num
    const int DD_EBACK__ENERGY_DETX  = 100; //!< Energy Back vs. Energy + detector num
  }
}

IonChamberProcessor::IonChamberProcessor() :
    EventProcessor(OFFSET, RANGE, "ionchamber") {
    associatedTypes.insert("ion_chamber");

    for (size_t i=0; i < noDets; i++) {
      lastTime[i] = -1;
      timeDiffs[i].clear();
    }
}

void IonChamberProcessor::DeclarePlots(void) {
  DeclareHistogram1D(D_ENERGYSUM, SE, "ion chamber energy");

  for (size_t i=0; i < noDets - 2; i++) {
    DeclareHistogram1D(D_ENERGYTHREE_GROUPX + i, SE, "ion chamber 3sum");
  }
  for (size_t i=0; i < noDets - 1; i++) {
    DeclareHistogram1D(D_ENERGYTWO_GROUPX + i, SE, "ion chamber 2sum");
  }
  for (size_t i=0; i < noDets; i++) {
    DeclareHistogram1D(D_DTIME_DETX + i, SE, "dtime for det i, 100 ns");
    DeclareHistogram1D(D_RATE_DETX + i, SE, "calc rate for det i, Hz");
    DeclareHistogram2D(DD_ESUM__ENERGY_DETX + i,
		       SA, SA, "ion seg i v sum");
    DeclareHistogram2D(DD_EBACK__ENERGY_DETX + i,
		       SA, SA, "ion seg i v ion 234");
  }
}

bool IonChamberProcessor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
	return false;

    static const vector<ChanEvent*> &icEvents = sumMap["ion_chamber"]->GetList();

    double esum = 0.; // all
    double etwo[noDets-1] = {0};
    double ethree[noDets-2] = {0};

    data.Clear();

    data.mult = icEvents.size();

    for (vector<ChanEvent*>::const_iterator it = icEvents.begin();
	 it != icEvents.end(); it++) {
      // make the energy sum
      size_t loc = (*it)->GetChanID().GetLocation();
      double ecal = (*it)->GetCalEnergy();

      data.raw[loc] += (*it)->GetEnergy();
      data.cal[loc] += ecal;

      esum += ecal;
      etwo[loc] += ecal;
      ethree[loc] += ecal;
      if (loc >= 1) {
	etwo[loc-1] += ecal;
	ethree[loc-1] += ecal;
      }
      if (loc >= 2) {
	ethree[loc-2] += ecal;
      }
    }
    plot(D_ENERGYSUM, esum / 6);
    for (size_t i=0; i < noDets - 1; i++) {
      plot(D_ENERGYTWO_GROUPX + i, etwo[i] / 2);
    }
    for (size_t i=0; i < noDets - 2; i++) {
      plot(D_ENERGYTHREE_GROUPX + i, ethree[i] / 3);
    }

    // once more to do the plots
    for (vector<ChanEvent*>::const_iterator it = icEvents.begin();
	 it != icEvents.end(); it++) {
      size_t loc = (*it)->GetChanID().GetLocation();
      double ecal = (*it)->GetCalEnergy();
      if (loc > noDets) {
	// unexpected location
	continue;
      }

      // messy for right now
      if (icEvents.size() == noDets) {
	plot(DD_ESUM__ENERGY_DETX + loc, esum / 6, ecal);
	switch(loc) {
	case 0:
	case 1:
	case 5:
	  plot(DD_EBACK__ENERGY_DETX + loc, ethree[2]/ 3, ecal);
	  break;
	case 2:
	case 3:
	case 4:
	  plot(DD_EBACK__ENERGY_DETX + loc, (ethree[2] - ecal)/2, ecal);
	  break;
	}

      }
      if (lastTime[loc] != -1) {
	double dtime = (*it)->GetTime() - lastTime[loc];
	plot(D_DTIME_DETX + loc, dtime / 10);

	if (dtime > minTime)
	  timeDiffs[loc].push_back( dtime );
	if (timeDiffs[loc].size() >= timesToKeep) {
	  timeDiffs[loc].pop_front();

	  // now calculate the rate as the inverse of the mean
	  //  of the assumed Poissonic distribution
	  // since there is some dead time only take times greater than a
	  //  specific safe value (thanks to memorylessness of distribution)

	  double sum = 0.;
	  int count = 0;

	  for (deque<double>::const_iterator dit = timeDiffs[loc].begin();
	       dit < timeDiffs[loc].end(); dit++) {
	    sum += *dit - minTime;
	    count++;
	  }
	  double mean = double(sum / count);
	  plot(D_RATE_DETX + loc,
          (double)(1 / mean / Globals::get()->clockInSeconds()));
	}

      }
      lastTime[loc] = (*it)->GetTime();
    }
    EndProcess(); // update the processing time
    return true;
}

void IonChamberProcessor::Data::Clear(void) {
  for (size_t i=0; i < noDets; i++)
    raw[i] = cal[i] = 0;
  mult = 0;
}

#ifdef useroot
bool IonChamberProcessor::AddBranch(TTree *tree)
{
  if (tree) {
    TBranch *branch = tree->Branch(name.c_str(), &data,
				   "raw[6]/D:cal[6]:mult/I");
    return (branch != NULL);
  }
  return false;
}

void IonChamberProcessor::FillBranch(void)
{
  if (!HasEvent())
    data.Clear();
}

#endif // USEROOT
