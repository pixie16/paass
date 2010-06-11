/** \file RootProcessor.cpp
 *
 * Implemenation of class to dump event info to a root tree
 * David Miller, Jan 2010
 */

#ifdef useroot

#include <iostream>

#include <TFile.h>
#include <TTree.h>

#include "DetectorDriver.h"
#include "RootProcessor.h"

using std::cout;
using std::endl;

/** Open a file for tree output */
RootProcessor::RootProcessor(const char *fileName, const char *treeName)
    : EventProcessor()
{
    name = "RootProcessor";
    tree = new TTree(treeName, treeName);
    file = new TFile(fileName, "recreate"); //! overwrite tree for now
}

/** Add branches to the tree from the event processors in the driver */
bool RootProcessor::Init(DetectorDriver &driver)
{
    const vector<EventProcessor *>& drvProcess = driver.GetProcessors();

    for (vector<EventProcessor *>::const_iterator it = drvProcess.begin();
	 it != drvProcess.end(); it++) {
	if ((*it)->AddBranch(tree)) {
	    vecProcess.push_back(*it);
	}
    }
    // always use when added independent of detectors

    if (!vecProcess.empty()) {
	initDone = true;
	cout << "processor " << name << " initialized with "
	     << vecProcess.size() << " detectors written to tree." << endl;
	//? add raw event data branches
    }

    return initDone;
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
    cout << "  saving " << tree->GetEntries() << " tree entries" << endl;
    tree->AutoSave()
    file->Close();

    delete file;
    // for some reason, deleting the tree crashes the executable
    //   hopefully the kernel will get the memory back
//    delete tree;
}

#endif // useroot
