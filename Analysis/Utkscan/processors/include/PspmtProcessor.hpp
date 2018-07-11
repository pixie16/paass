///@file PspmtProcessor.cpp
///@brief Processes information from a Position Sensitive PMT.
///@author A. Keeler
///@date July 8, 2018
#ifndef __PSPMTPROCESSOR_HPP__
#define __PSPMTPROCESSOR_HPP__

#include <string>
#include <map>

#include "EventProcessor.hpp"
#include "RawEvent.hpp"
#include "ProcessorRootStruc.hpp"

///Class to handle processing of position sensitive pmts
class PspmtProcessor : public EventProcessor {
public:
    ///Default Constructor */
    PspmtProcessor(void) {};

    ///@brief Constructor that sets the scale and offset for histograms
    ///@param[in] scale : The multiplicative scaling factor
    ///@param[in] offset : The additave offset for the histogram
    PspmtProcessor(const std::string &vd, const double &scale,
                   const unsigned int &offset, const double &threshold);

    ///Default Destructor
    ~PspmtProcessor() {};

    ///Declare the plots used in the analysis
    void DeclarePlots(void);

    ///Preprocess the PSPMT data
    ///@param [in] event : the event to preprocess
    ///@return true if successful */
    bool PreProcess(RawEvent &event);

    ///Enumeration that describes the different voltage dividers that have
    /// been used. Each board may have a different method to calculate the
    /// position of the interaction.
    enum VDTYPES {
        corners, sides, UNKNOWN
    };

    ///@return The x,y position of the interaction as calculated from the QDC
    /// found from the waveform in the WaveformAnalyzer, the on-board
    /// Pixie-16 energy filter, or the TraceFilterAnalyzer applying a
    /// trapezoidal filter to the trace.
    std::pair<double, double> GetPosition(const std::string &type) {
        if (type == "low")
            return position_low;
        else if (type == "high")
            return position_high;
        else
            return std::pair<double, double>(0., 0.);
    }

    ///@return The PSPMT Processor's TNAMED header. The Order is VD type then the software-based anode threshold.
    std::pair<std::string,std::string> GetPSPMTHeader(){
        return (make_pair(VDtypeStr,ThreshStr));
    }

    ///@return The vector of the pspmt events for Root output (should only have 1 entry)
    std::vector<PSPMT> GetPSPMTvector(){
        return PSvec;
    }


private:

    std::pair<double, double> position_low;
    std::pair<double, double> position_high;

    ///@brief A method to calculate the x position of the interaction with
    /// the scintillator
    ///@param[in] map : The map containing the set of x and y points that we
    /// will use to calculate the position.
    ///@return The x,y position of the interaction
    std::pair<double, double> CalculatePosition(double &xa, double &xb, double &ya,
                                                double &yb, const VDTYPES &vdtype);


    VDTYPES vdtype_; ///< Local variable to store the type of voltage divider
    ///< we're using.
    double positionScale_; ///< The scale that we need for the DAMM output
    unsigned int positionOffset_; ///< The offset that we need for the the
    ///< DAMM output
    double threshold_; ///< The threshold that the energy calculated by
    ///< the Pixie-16 trapezoidal filter needs to reach
    ///< before we can analyze the signals.

    PSPMT PSstruct,DefaultStruc; //!< PSPMT root Struct and Default for reseting
    std::vector<PSPMT> PSvec; //!<PSPMT vector for root

    std::string VDtypeStr; //!< VD Type as a string
    std::string ThreshStr; //!< Threshold as a string

};

#endif // __PSPMTPROCESSOR_HPP__
