/** \file McpProcessor.cpp
 * \brief Handles MCP detectors
 *
 * Mcp class takes the signals from the mcp detector
 *   and calculates a 2D position based on the readout
 *
 * \author S. Liddick
 * \date 02 Feb 2008
 * <STRONG>Modified : </strong> D. Miller 09-09
 */
#ifdef useroot
#include <TTree.h>
#endif

#include "DammPlotIds.hpp"
#include "McpProcessor.hpp"
#include "RawEvent.hpp"

using std::string;
using std::vector;
using namespace dammIds::mcp;

namespace dammIds {
    namespace mcp {
        const int D_POSX   = 1; //!< Position in X
        const int D_POSY   = 2;//!< Position in Y
        const int DD_POSXY = 3;//!< Position in X,Y
    }
}


void McpProcessor::McpData::Clear(void) {
  for (size_t i=0; i < nPos; i++)
    raw[i] = 0;
  xpos = ypos = 0.;

  mult = 0;
}

McpProcessor::McpProcessor(void) : EventProcessor(OFFSET, RANGE, "McpProcessor") {
  associatedTypes.insert("mcp");
}

void McpProcessor::DeclarePlots(void) {

  const int posBins   = SE;
  const int posBins2D = S9;

  DeclareHistogram1D(D_POSX, posBins, "Horiz. - MCP Left");
  DeclareHistogram1D(D_POSY, posBins, "Vert. - MCP Left");
  DeclareHistogram2D(DD_POSXY, posBins2D, posBins2D, "MCP Left 2D");
}


bool McpProcessor::Process(RawEvent &event) {
  if (!EventProcessor::Process(event))
    return false;

  double qTotal, qRight, qTop;

  static const vector<ChanEvent*> &mcpEvents = sumMap["mcp"]->GetList();

  data.Clear();
  for (vector<ChanEvent*>::const_iterator it = mcpEvents.begin();
       it != mcpEvents.end(); it++) {
      ChanEvent *chan = *it;

      string subtype   = chan->GetChanID().GetSubtype();
      double calEnergy = chan->GetCalEnergy();

      if(subtype == "1time") {
	  // do nothing
      } else if (subtype == "1position1"){
	  data.raw[0] = calEnergy;
	  data.mult++;
      } else if (subtype == "1position2"){
	  data.raw[1] = calEnergy;
	  data.mult++;
      } else if (subtype == "1position3"){
	  data.raw[2] = calEnergy;
	  data.mult++;
      } else if (subtype == "1position4"){
	  data.raw[3] = calEnergy;
	  data.mult++;
      }
  }

  // calculation of position from charge collected on the four corners
  // magic numbers here
  data.raw[0] *= 1.3;

  qTotal = data.raw[0] + data.raw[1] + data.raw[2] + data.raw[3];

  qRight = data.raw[3] + data.raw[0];
  // qLeft   = data.raw[2] + data.raw[1];
  qTop   = data.raw[0] + data.raw[1];
  // qBottom = data.raw[2] + data.raw[3];

  data.xpos = (qRight / qTotal) * 512. - 75; //horizontal MCP pos
  data.ypos = (qTop   / qTotal) * 512. - 75; //vertical MCP pos
  // qLeft, qBottom not used

  if (data.mult == 4) {
    using namespace dammIds::mcp;

    plot(D_POSX, data.xpos);
    plot(D_POSY, data.ypos);
    plot(DD_POSXY, data.xpos, data.ypos);
  }

  EndProcess();
  return (data.mult == 4);
}

#ifdef useroot
bool McpProcessor::AddBranch(TTree *tree) {
  if (tree) {
    TBranch *mcpBranch =
      tree->Branch(name.c_str(), &data, "raw[4]/D:xpos:ypos:mult/I");

    return (mcpBranch != NULL);
  }

  return false;
}

void McpProcessor::FillBranch(void) {
  if (!HasEvent())
    data.Clear();
}
#endif //useroot
