///@file E11027Processor.cpp
///@brief Class that handles outputting MONA-LISA-SWEEPER timestamps from the E11027 experiment conducted at NSCL.
///@author S. V. Paulauskas
///@date May 26, 2017
#include <fstream>
#include <iostream>

#include <cmath>

#include "DammPlotIds.hpp"
#include "E11027Processor.hpp"
#include "RawEvent.hpp"

namespace dammIds {
    namespace experiment {
        ///we have no plots to declare
    }
}//namespace dammIds

using namespace std;
using namespace dammIds::experiment;

void E11027Processor::DeclarePlots(void) {
    //There are no plots to declare
}

E11027Processor::E11027Processor() : EventProcessor(OFFSET, RANGE, "E11027Processor") {
    associatedTypes.insert("valid");

    stringstream name;
    name << Globals::get()->GetOutputPath()
         << Globals::get()->GetOutputFileName() << "-monaTS.dat";
    poutstream_ = new ofstream(name.str().c_str());
}

///Destructor to close output files and clean up pointers
E11027Processor::~E11027Processor() {
    poutstream_->close();
    delete (poutstream_);
}

///Main processing of data of interest
bool E11027Processor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return (false);

    static const vector<ChanEvent *> &validEvents = event.GetSummary("valid")->GetList();

    for (vector<ChanEvent *>::const_iterator itValid = validEvents.begin(); itValid != validEvents.end(); itValid++)
        if ((*itValid)->GetChanID().HasTag("output"))
            *poutstream_ << (*itValid)->GetChanID().GetType() << " " << (*itValid)->GetChanID().GetLocation() << " "
                        << (*itValid)->GetQdc()[0] << " " << (*itValid)->GetQdc()[1] << endl;

    EndProcess();
    return (true);
}
