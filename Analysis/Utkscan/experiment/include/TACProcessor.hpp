/** \file TACProcessor.hpp
 * \brief A Simple Processor for TAC signals in Pixie.
 * \author D. Perez-Loureiro
 * \date June 2, 2017
 */
#ifndef __TACPROCESSOR_HPP__
#define __TACPROCESSOR_HPP__

#include "EventProcessor.hpp"

#ifdef useroot
#include <TFile.h>
#include <TTree.h>
#include <TH2D.h>
#include <TH1D.h>
#endif


//! A generic processor to be used as a template for others
class TACProcessor : public EventProcessor {
public:
    /** Default Constructor */
    TACProcessor();
    /** Constructor Accepting an argument */
    TACProcessor(const double & a);
    /** Default Destructor */
    ~TACProcessor();
    /** Declares the plots for the processor */
    virtual void DeclarePlots(void);
    /** Performs the preprocessing, which cannot depend on other processors
    * \param [in] event : the event to process
    * \return true if preprocessing was successful */
    virtual bool PreProcess(RawEvent &event);
    /** Performs the main processsing, which may depend on other processors
    * \param [in] event : the event to process
    * \return true if processing was successful */
    virtual bool Process(RawEvent &event);
    /** \return The processed TAC events */
    std::vector<ChanEvent*> GetTACEvents(void) const {
        return(evts_);
    }
private:
    double a_=1.; //!< a variable global to the class
    std::vector<ChanEvent*> evts_; //!< vector of events for people to get
};
#endif // __TACPROCESSOR_HPP__
