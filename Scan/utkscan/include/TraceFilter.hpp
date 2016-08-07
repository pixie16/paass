
/***************************************************************************
  *  Copyright S. V. Paulauskas 2014-2016                                  *
  *                                                                        *
  *  This program is free software: you can redistribute it and/or modify  *
  *  it under the terms of the GNU General Public License as published by  *
  *  the Free Software Foundation, version 3.0 License.                    *
  *                                                                        *
  *  This program is distributed in the hope that it will be useful,       *
  *  but WITHOUT ANY WARRANTY; without even the implied warranty of        *
  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
  *  GNU General Public License for more details.                          *
  *                                                                        *
  *  You should have received a copy of the GNU General Public License     *
  *  along with this program.  If not, see <http://www.gnu.org/licenses/>. *
  **************************************************************************
*/
/** \file TraceFilter.hpp
 *  \brief A class to perform trapezoidal filtering
 *  \author S. V. Paulauskas
 *  \date 23 April 2014
 *
 *  This code is based off of the IGOR macro energy.ipf
 *  written by H. Tan of XIA LLC and parts of the nscope
 *  program written at the NSCL written by C.Prokop.
 *
 */
#ifndef __TRACEFILTER_HPP__
#define __TRACEFILTER_HPP__
#include <vector>
#include <utility>

#include "Trace.hpp"
#include "TrapFilterParameters.hpp"

/*! The class to perform the filtering */
class TraceFilter {
public:
    /** Default Constructor */
    TraceFilter(){};
    /** Constructor 
     * \param [in] nsPerSample : The ns/Sample for the ADC */
    TraceFilter(const int &nsPerSample){nsPerSample_ = nsPerSample;}
    /** Constructor 
     * \param [in] nsPerSample : The ns/Sample for the ADC 
     * \param [in] tFilt : Parameters for the trigger filter
     * \param [in] eFilt : Paramters for the energy filter
     * \param [in] analyzePileup : True if we want additional analysis for pileups. 
     * \param [in] verbose : true if we want verbose output from the filter */
    TraceFilter(const unsigned int &nsPerSample, 
                const TrapFilterParameters &tFilt,
                const TrapFilterParameters &eFilt,
                const bool &analyzePileup = false,
                const bool &verbose = false);
    /** Default Destructor */
    ~TraceFilter(){}

    /** \return True if there was a pileup in the trace */
    bool GetHasPileup(void){return(trigs_.size() > 1);}

    /** \return the average value of the baseline */
    double GetBaseline(void){return(baseline_);}
    /** \return The energy calculated from the first trigger. */
    double GetEnergy(void){return(en_[0]);}

    /** This is the main method that will be used to calculate the filters and 
     * other necessary information. 
     * \param [in] sig : The trace that we are going to be filtering  */
    unsigned int CalcFilters(const Trace *sig);
    /** \return The number of triggers that were found */
    unsigned int GetNumTriggers(void) {return(trigs_.size());}
    /** \return The position in the trace of the first trigger found by the trigger 
     * filter */
    unsigned int GetTrigger(void){return(trigs_[0]);}

    /** \return The trigger filter */
    std::vector<double> GetTriggerFilter(void) {return(trigFilter_);}
    /** \return The list of energies that were found if we chose to analyze 
     * pileup events. */
    std::vector<double> GetEnergies(void){return(en_);}
    /** There will be three coefficients per identified trigger if we chose to 
     * analyze pileups. This means the first three elements belong to the first 
     * trigger, the next three to the second trigger, etc. The size will always 
     * be 3*NumTriggers.
     *  \return The list of energy filter coefficients.  */
    std::vector<double> GetEnergyFilterCoefficients(void) {return(coeffs_);}
    /** There will be three energy sums per identified trigger if we chose to 
     * analyze pileups. This means the first three elements belong to the first 
     * trigger, the next three to the second trigger, etc. The size will always 
     * be 3*NumTriggers.
     *  \return The list of energy filter coefficients.  */
    std::vector<double> GetEnergySums(void) {return(esums_);}

    /** \return List of the triggers found in the trace.*/
    std::vector<unsigned int> GetTriggers(void){return(trigs_);}
    /** This will always have 6 elements. If analyzing pileups it will be the 
     * limits for the last identified pileup. 
     *  \return List of the limits for the energy sums */
    std::vector<unsigned int> GetEnergySumLimits(void){return(limits_);}

    /** Sets the value of the ns/Sample for the ADC */
    void SetAdcSample(const double &a){nsPerSample_ = a;}
    /** Sets the trapezoidal filter parameters for the energy (slow) filter */
    void SetEnergyParams(const TrapFilterParameters &a) {
        e_ = a;
        ConvertToClockticks();
    }
    /** Sets the trace that we are going to use to filter */
    void SetSig(const Trace *sig){sig_ = sig;}
    /** Sets the trapezoidal filter parameters for the trigger (fast) filter */
    void SetTriggerParams(const TrapFilterParameters &a) {
        t_ = a;
        ConvertToClockticks();
    }
    /** Sets the verbosity, useful for debugging issues with the filter.  */
    void SetVerbose(const bool &a){isVerbose_ = a;}

private:
    bool isVerbose_; //!< True if we want verbose output
    bool isConverted_; //!< True if Filter Pars converted to clockticks
    bool analyzePileup_; //!< True if we want to analyze pileups

    double baseline_; //!< the value of the baseline of the trace

    ///Enumeration containing error statuses 
    enum ErrTypes{NO_TRIG=1,EARLY_TRIG,LATE_TRIG,BAD_FILTER_COEFF,BAD_FILTER_LIMITS};

    TrapFilterParameters e_; //!< The energy filter parameters
    TrapFilterParameters t_; //!< The trigger filter parameters 
    
    unsigned int nsPerSample_; //!< The number of ns per sample

    const Trace *sig_; //!< the signal to filter

    std::vector<double> en_; //!< the calculated energies
    std::vector<double> coeffs_; //!< the calculated energy coefficients
    std::vector<double> trigFilter_; //!< the calculated trigger filter
    std::vector<double> esums_; //!< the caluclated energy sums
    
    std::vector<unsigned int> limits_; //!< the limits for the energy filter
    std::vector<unsigned int> trigs_; //!< the identified triggers

    void CalcBaseline(void); //!< calculates the baseline
    void CalcEnergyFilterCoeffs(void); //!< calculates energy filter coeffs
    void CalcEnergyFilterLimits(const unsigned int &tpos); //!< calc energy filter limits
    void CalcEnergyFilter(void); //!< calculate the energy filter
    void CalcTriggerFilter(void); //!< calculate trigger filter
    void ConvertToClockticks(void); //!< convert from ns to clockticks
    void Reset(void); //!< Reset values for repeated calls. 
};
#endif //__TRACEFILTER_HPP__
