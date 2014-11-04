/** \file RootProcessor.cpp
 * \brief Implemenation of class to dump event info to a root tree
 * \author David Miller
 * \date Jan 2010
 */

#ifdef useroot

#include <algorithm>
#include <iostream>

#include <TTree.h>

#include "DetectorDriver.hpp"
#include "RootProcessor.hpp"

using std::cout;
using std::endl;

/** Open a file for tree output */
RootProcessor::RootProcessor(const char *fileName, const char *treeName)
    : EventProcessor()
{
    name = "RootProcessor";
    file = new TFile(fileName, "recreate"); //! overwrite tree for now
    tree = new TTree(treeName, treeName);
}

/** Add branches to the tree from the event processors in the driver */
bool RootProcessor::Init(RawEvent& rawev)
{
    DetectorDriver* driver = DetectorDriver::get();
    if (file == NULL || tree == NULL) {
        cout << "Failed to create ROOT objects for "
        << name << " processor" << endl;
        return false;
    }

    const vector<EventProcessor *>& drvProcess = driver->GetProcessors();

    for (vector<EventProcessor *>::const_iterator it = drvProcess.begin();
        it != drvProcess.end(); it++) {
        if ((*it)->AddBranch(tree)) {
        vecProcess.push_back(*it);
        set_union( (*it)->GetTypes().begin(), (*it)->GetTypes().end(),
            associatedTypes.begin(), associatedTypes.end(),
            inserter(associatedTypes, associatedTypes.begin()) );
        }
    }
    return EventProcessor::Init(rawev);
}

/** Fill the tree for each event, saving to file occasionally */
bool RootProcessor::Process(RawEvent &event)
{
    if (!EventProcessor::Process(event))
	return false;

    for (vector<EventProcessor *>::iterator it = vecProcess.begin();
	 it != vecProcess.end(); it++) {
	(*it)->FillBranch();
    }

    tree->Fill();
    if (tree->GetEntries() % 1000 == 0) {
	tree->AutoSave();
    }

    EndProcess();
    return true;
}

/** Finish flushing the file to disk, and clean up memory */
RootProcessor::~RootProcessor()
{
  if (tree != NULL) {
    cout << "  saving " << tree->GetEntries() << " tree entries" << endl;
    tree->AutoSave();
  }
  if (file != NULL) {
    file->Close();

    delete file; // this also frees the tree
  }
}

#endif // useroot
