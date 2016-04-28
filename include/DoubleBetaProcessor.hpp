/** \file DoubleBetaProcessor.hpp
 * \brief A Template class to be used to build others.
 * \author S. V. Paulauskas
 * \date October 26, 2014
 */
#ifndef __DOUBLEBETAPROCESSOR_HPP__
#define __DOUBLEBETAPROCESSOR_HPP__

#include "BarDetector.hpp"
#include "EventProcessor.hpp"
#include "HighResTimingData.hpp"

//! A generic processor to be used as a template for others
class DoubleBetaProcessor : public EventProcessor {
public:
    /** Default Constructor */
    DoubleBetaProcessor();
    /** Default Destructor */
    ~DoubleBetaProcessor(){};

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

    /** \return The map of the bars that had high resolution timing */
    BarMap GetBars(void){return(bars_);}
    /** \return the map of the bars that had low resolution timing */
    std::map<unsigned int, std::pair<double,double> > GetLowResBars(void) {
	return(lrtbars_);
    }
private:
    BarMap bars_;
    std::map<unsigned int, std::pair<double,double> > lrtbars_;
};
#endif // __DOUBLEBETAPROCESSOR_HPP__
