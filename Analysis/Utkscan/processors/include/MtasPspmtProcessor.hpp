//
// Created by darren on 2/21/19.
//

#ifndef _MTASPSPMTPROCESSOR_HPP_
#define _MTASPSPMTPROCESSOR_HPP_

#include <string>
#include <map>

#include "EventProcessor.hpp"
#include "RawEvent.hpp"
#include "ProcessorRootStruc.hpp"

/// Class to handle processing of MTAS implant and beam diagnostics detector
class MtasPspmtProcessor : public EventProcessor {
public:
    ///Default Constructor */
    MtasPspmtProcessor(void){};

    ///@brief Constructor that sets the scale and offset for histograms
    ///@param[in] scale: The multiplicative scaling factor
    ///@param[in] offset: The additive offset for the histogram
    MtasPspmtProcessor(const std::string &dt, const double &scale, const unsigned int &offset,
                        const double &threshold);

    ///Default Destructor
    ~MtasPspmtProcessor() {};

    ///Declare the plots used in the analysis
    void DeclarePlots(void);

    ///Preprocess the PSPMT data
    ///@param [in] event: the event to preprocess
    ///@return true if successful */
    bool PreProcess(RawEvent &event);

    ///Enumeration that describes the different detectors that have been used. Each board
    /// will have a different method to calculate the position.
    enum DTTYPES {
        implant, diagnostic, UNKNOWN
    };

    ///@return The x,y position of the interaction as calculated from the energy passed by pixie
    std::pair<double, double> GetPosition(const std::string &type) {
        if (type == "mtas_type")
            return position_mtas;
        else
            return std::pair<double, double>(0.,0.);
    }

    ///@return The MtasPspmt Processor's TNAMED header. The order is DT type then the
    ///software-based anode threshold.
    std::pair<std::string,std::string> GetMtasPSPMTHeader(){
        return (make_pair(DTtypeStr, ThreshStr));
    }

private:

    std::pair<double, double> position_mtas;

    ///@brief A method to calculate the x position of the interaction with
    /// the scintillator
    ///@param[in] map : The map containing the set of x and y points that we
    /// will use to calculate the position.
    ///@return The x,y position of the interaction
    std::pair<double, double> CalculatePosition(double &xa, double &xb, double &ya,
                                                double &yb, const DTTYPES &dttype);


    DTTYPES dttype_; ///< Local variable to store the type of detector we're using.
    double positionScale_; ///< The scale that we need for the DAMM output
    unsigned int positionOffset_; ///< The offset that we need for the the DAMM output
    double threshold_; ///< The threshold that the energy calculated by
    ///< the Pixie-16 trapezoidal filter needs to reach
    ///< before we can analyze the signals.

    processor_struct::MTASPSPMT MTASPSstruct; //!< PSPMT root Struct

    std::string DTtypeStr; //!< VD Type as a string
    std::string ThreshStr; //!< Threshold as a string

};
#endif //_MTASPSPMTPROCESSOR_HPP_
