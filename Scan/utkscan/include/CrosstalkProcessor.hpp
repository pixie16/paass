/** \file CrosstalkProcessor.hpp
 * \brief A Crosstalk class to be used to build others.
 * \author S. V. Paulauskas
 * \date October 26, 2014
 */
#ifndef __CROSSTALKPROCESSOR_HPP__
#define __CROSSTALKPROCESSOR_HPP__

#include "EventProcessor.hpp"
#include "RawEvent.hpp"
#include "VandleProcessor.hpp"

//! A generic processor to be used as a template for others
class CrosstalkProcessor : public VandleProcessor {
public:
    /** Default Constructor */
    CrosstalkProcessor();
    /** Default Destructor */
    ~CrosstalkProcessor(){};
    /** Declares the plots for the processor */
    void DeclarePlots(void);
    
    /** Preprocess the data for Crosstalk
     * \param [in] event : the event to preprocess
     * \return true if successful */
    bool PreProcess(RawEvent &event);

    /** Performs the main processsing, which may depend on other processors
    * \param [in] event : the event to process
    * \return true if processing was successful */
    bool Process(RawEvent &event);
private:
    typedef std::pair<unsigned int, unsigned int> CrossTalkKey; //!< Typedef for crosstalk
    typedef std::map<CrossTalkKey, double> CrossTalkMap;//!< typedef for map for crosstalk handling
    std::map<CrossTalkKey, double> crossTalk;//!< map to put in cross talk info

};
#endif // __CROSSTALKPROCESSOR_HPP__
