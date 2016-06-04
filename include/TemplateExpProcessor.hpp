/** \file TemplateExpProcessor.hpp
 * \brief Example class for experiment specific setups
 *\author S. V. Paulauskas
 *\date May 20, 2016
 */
#ifndef __TEMPLATEEXPPROCESSOR_HPP_
#define __TEMPLATEEXPPROCESSOR_HPP_
#include <fstream>

#include "EventProcessor.hpp"

#ifdef useroot
#include <TFile.h>
#include <TTree.h>
#include <TH2D.h>
#include <TH1D.h>
#endif

/// Working template class for experiment processors
class TemplateExpProcessor : public EventProcessor {
public:
    /** Default Constructor */
    TemplateExpProcessor();
    /** Constructor to take an argument for the gamma cutoff */
    TemplateExpProcessor(const double &gcut);
    /** Default Destructor */
    ~TemplateExpProcessor();
    /** Declare the plots used in the analysis */
    virtual void DeclarePlots(void);

    /** PreProcess does nothing since this is solely dependent on results
     from other Processors*/
    virtual bool PreProcess(RawEvent &event);

    /** Process the event
    * \param [in] event : the event to process
    * \return Returns true if the processing was successful */
    virtual bool Process(RawEvent &event);
private:
    /** Obtain the name of the histogram file */
    void ObtainHisName(void);
    /** Sets the detectors that are associated with this processor */
    void SetAssociatedTypes(void);
    /** Sets up the ASCII output file stream */
    void SetupAsciiOutput(void);

    std::string fileName_; //!< String to hold the file name from command line
    std::ofstream *poutstream_; //!< Pointer to ouptut ASCII file stream.
    double gCutoff_; //!< Variable used to set gamma cutoff energy

#ifdef useroot
    /** Method to setup the ROOT output, tree and histograms */
    void SetupRootOutput(void);
    TFile *prootfile_; //! pointer to root file
    TTree *proottree_; //! pointer to root tree
    TH2D *ptvsge_;    //! pointer to ROOT 2D histogram
    TH1D *ptsize_;     //! pointer to ROOT 1D histogram
#endif
};
#endif
