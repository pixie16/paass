/** \file TACProcessor.hpp
 * \brief A Simple Processor for an Array of Sensl J series SiPMs signals in Pixie.
 * \author D. Perez-Loureiro
 * \date June 21, 2017
 */
#ifndef __ARRAYJPROCESSOR_HPP__
#define __ARRAYJPROCESSOR_HPP__

#include "EventProcessor.hpp"

#ifdef useroot
#include <TFile.h>
#include <TTree.h>
#include <TH2D.h>
#include <TH1D.h>
#endif


//! A generic processor to be used as a template for others
class ArrayJProcessor : public EventProcessor {
public:
    /** Default Constructor */
    ArrayJProcessor();
    /** Constructor Accepting an argument */
    ArrayJProcessor(const double & a);
    /** Default Destructor */
    ~ArrayJProcessor();
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
    /** \return The processed ArrayJ events */
    std::vector<ChanEvent*> GetArrayJEvents(void) const {
        return(evts_);
    }
private:
    double a_=1.; //!< a variable global to the class
    std::vector<ChanEvent*> evts_; //!< vector of events for people to get
};
#endif // __ArrayJPROCESSOR_HPP__
