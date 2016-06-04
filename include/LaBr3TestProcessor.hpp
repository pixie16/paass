/** \file LaBr3TestProcessor.hpp
 * \brief A Labr3Test class to be used to build others.
 * \author S. V. Paulauskas
 * \date October 26, 2014
 */
#ifndef __LABR3TESTPROCESSOR_HPP__
#define __LABR3TESTPROCESSOR_HPP__

#include "EventProcessor.hpp"

//! A generic processor to be used as a template for others
class Labr3TestProcessor : public EventProcessor {
public:
    /** Default Constructor */
    Labr3TestProcessor();
    /** Default Destructor */
    ~Labr3TestProcessor(){};
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
private:

};
#endif // __LABR3TESTPROCESSOR_HPP__
