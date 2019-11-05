///@file Trace.hpp
///@brief A simple class to store the traces.
///@author S. Liddick, D. T. Miller, S. V. Paualuskas
///@date Long time ago.
#ifndef __TRACE_HPP__
#define __TRACE_HPP__

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <cmath>

/// @brief This defines a more extensible implementation of a digitized trace.
/// The class is derived from a vector of unsigned integers. This is the basic
/// form of a trace from most digitizers. The Trace class enables processed
/// information about the trace (such as the baseline, integral, etc.).
///
/// We also store information about the Waveform. The waveform is the part
/// of the trace that actually contains information about the signal that
/// was captured. This excludes the baseline.
class Trace : public std::vector<unsigned int> {
public:
    ///Default constructor
    Trace() : std::vector<unsigned int>() {}

    ///An automatic conversion for the trace
    ///@param [in] x : the trace to store in the class
    Trace(const std::vector<unsigned int> &x) : std::vector<unsigned int>(x) {}

    ///@return Returns a std::pair<double,double> containing the average and
    /// standard deviation of the baseline as the .first and .second
    /// respectively.
    std::pair<double, double> GetBaselineInfo() const { return baseline_; }

    ///@return Returns the energy sums that were set.
    std::vector<double> GetEnergySums() const { return esums_; }

    ///@return Returns a std::pair<unsigned int, double> containing the
    /// position of the maximum value in the trace and the amplitude of the
    /// maximum that's been extrapolated and baseline subtracted as the .first
    /// and .second respectively.
    std::pair<unsigned int, double> GetExtrapolatedMaxInfo() const { return extrapolatedMax_; }

    ///@return The baseline that goes with the filtered energies
    double GetFilteredBaseline() const { return filteredBaseline_; }

    ///@return The energies found by filtering the trace.
    std::vector<double> GetFilteredEnergies() const { return filteredEnergies_; }

    ///@return Returns a std::pair<unsigned int, double> containing the
    /// position of the maximum value in the trace and the amplitude of the
    /// maximum that's been baseline subtracted as the .first and .second
    /// respectively.
    std::pair<unsigned int, double> GetMaxInfo() const { return max_; }

    ///@return The number of triggers that were found in the trace
    unsigned int GetNumberOfTriggers() const { return triggerPositions_.size(); }

    ///@return The phase of the trace.
    double GetPhase() const { return phase_; }

    ///@return The value of the QDC for the waveform
    double GetQdc() const { return qdc_; }

    ///@returns the Signal to noise ratio of the trace
    double GetSignalToNoiseRatio() const { return 20 * std::log10(max_.second / baseline_.second); }

    ///@return The value of the tail-ratio for the waveform.
    double GetTailRatio() const { return tailRatio_; }

    ///@return The value of tau that was calculated from the trace.
    double GetTau() const { return tau_; }

    ///@return Returns the waveform sans baseline
    std::vector<double> GetTraceSansBaseline() const { return traceSansBaseline_; }

    ///@return Returns the Trigger Filter that was set.
    std::vector<double> GetTriggerFilter() const { return trigFilter_; }

    ///@return Returns a vector containing all of the found triggers
    std::vector<unsigned int> GetTriggerPositions() const { return triggerPositions_; }

    ///@return Returns the baseline subtracted waveform found inside the trace.
    std::vector<double> GetWaveform() {
        return std::vector<double>(traceSansBaseline_.begin() + waveformRange_.first,
                                   traceSansBaseline_.begin() + waveformRange_.second);
    }

    ///@return The bounds of the waveform in the trace
    std::pair<unsigned int, unsigned int> GetWaveformRange() const { return waveformRange_; }

    ///@return Returns the waveform with the baseline
    std::vector<unsigned int> GetWaveformWithBaseline() {
        return std::vector<unsigned int>(begin() + waveformRange_.first, begin() + waveformRange_.second);
    }
    
    ///@return True if the Waveform Analysis was completed successfully, this is a legacy carryover. Please use HasValidWaveformAnalysis().  
    bool HasValidAnalysis() const {return hasValidWaveformAnalysis_; }

    ///@return True if the fit of the trace was completed.
    bool HasValidTimingAnalysis() const { return hasValidTimingAnalysis_; }

    ///@return True if we were able to successfully analyze the trace.
    bool HasValidWaveformAnalysis() const { return hasValidWaveformAnalysis_; }

    ///@return True if the trace was saturated
    bool IsSaturated() { return isSaturated_; }

    ///Sets the baseline information for the trace (average and standard
    /// deviation)
    ///@param[in] a : The pair<double,double> containing the average and
    /// standard deviation.
    void SetBaseline(const std::pair<double, double> &a) { baseline_ = a; }

    ///sets the energy sums vector if we are using the TriggerFilterAnalyzer
    ///@param [in] a : the vector of energy sums
    void SetEnergySums(const std::vector<double> &a) { esums_ = a; }

    ///Sets the maximum information for the trace (position and baseline
    /// subtracted extraploated value)
    ///@param[in] a : The pair<unsigned int ,double> containing the position and
    /// baseline subtracted extrapolated value.
    void SetExtrapolatedMax(const std::pair<unsigned int, double> &a) { extrapolatedMax_ = a; }

    ///Sets the baseline value found via a filter of the trace. This value is
    /// in the "filtered" units.
    void SetFilteredBaseline(const double &a) { filteredBaseline_ = a; }

    ///Sets the vector containing the energys found by performing a
    /// trapezoidal filter on the trace.
    ///@param[in] a : The vector containing the energies.
    void SetFilteredEnergies(const std::vector<double> &a) { filteredEnergies_ = a; }

    ///Sets the value of hasValidWaveformAnalysis_
    ///@param[in] a : True if we were able to successfully fit this trace
    /// for information about the maximum, baseline, phase, etc.
    void SetHasValidTimingAnalysis(const bool &a) { hasValidTimingAnalysis_ = a; }

    ///Sets the value of hasValidWaveformAnalysis_
    ///@param[in] a : True if we were able to successfully analyze this waveform
    /// for information about the maximum, baseline, phase, etc.
    void SetHasValidWaveformAnalysis(const bool &a) { hasValidWaveformAnalysis_ = a; }

    ///Sets the isSaturated_ private variable.
    ///@param[in] a : Sets to true if the trace was flagged as saturated by
    /// the electronics.
    void SetIsSaturated(const bool &a) { isSaturated_ = a; }

    ///Sets the maximum information for the trace (position and baseline
    /// subtracted value)
    ///@param[in] a : The pair<unsigned int ,double> containing the position and
    /// baseline subtracted value.
    void SetMax(const std::pair<unsigned int, double> &a) { max_ = a; }

    ///Sets the sub-sampling phase of the trace in units of samples.
    ///@param[in] a : The value of the phase that we want to set. This
    /// comes from a fit or CFD analysis of the trace.
    void SetPhase(const double &a) { phase_ = a; }

    ///Sets the value of the QDC that was calculated from the waveform
    ///@param[in] a : The value that we are going to set
    void SetQdc(const double &a) { qdc_ = a; }

    ///Sets the baseline subtracted trace.
    ///@param[in] a : The vector that we are going to assign.
    void SetTraceSansBaseline(const std::vector<double> &a) { traceSansBaseline_ = a; }

    ///Sets the value of the tail-ratio method used for doing discrimination
    /// on signals that have a varying decay constant. This is generally
    /// defined as the integral of the "tail" of the waveform divided by the
    /// QDC of the waveform.
    ///@param[in] a : The value that we're going to set.
    void SetTailRatio(const double &a) { tailRatio_ = a; }

    ///Sets the value of Tau as calculated from the waveform
    ///@param[in] a : The value that we are going to assign.
    void SetTau(const double &a) { tau_ = a; }

    ///sets the trigger filter if we are using the TriggerFilterAnalyzer
    ///@param [in] a : the vector with the trigger filter
    void SetTriggerFilter(const std::vector<double> &a) { trigFilter_ = a; }

    ///Sets the trigger positions that were located by a trapezoidal filter
    /// of the trace.
    ///@param[in] a : The vector containing the trigger positions.
    void SetTriggerPositions(const std::vector<unsigned int> &a) { triggerPositions_ = a; }

    ///Sets the bounds for the waveform
    ///@param[in] a : the range we want to set
    void SetWaveformRange(const std::pair<unsigned int, unsigned int> &a) { waveformRange_ = a; }

private:
    bool isSaturated_; ///< True if the trace was flagged as saturated.
    bool hasValidWaveformAnalysis_;///< True if the analysis of the waveform was successful (no fitting)
    bool hasValidTimingAnalysis_;///< True if the fit/cfd of the trace was completed

    double phase_; ///< The sub-sampling phase of the trace.
    double qdc_; ///< The qdc that was calculated from the waveform.
    double tailRatio_; ///< The tail-ratio of the trace.
    double tau_; ///< The tau as calculated from the waveform
    double filteredBaseline_; ///< Baseline calculated from filtering the trc.

    unsigned int numTriggers_; ///< The number of triggers in the trace.

    std::pair<double, double> baseline_; ///< Baseline Average and Std. Dev.
    std::pair<unsigned int, double> max_; ///< Max position and value sans baseline
    std::pair<unsigned int, double> extrapolatedMax_; ///< Max position and extrapolated value
    std::pair<unsigned int, unsigned int> waveformRange_; ///< Waveform Range

    std::vector<double> filteredEnergies_; ///< Energies from filtering the trc.
    std::vector<double> traceSansBaseline_; ///< Baseline subtracted trace
    std::vector<double> trigFilter_; ///< The trigger filter for the trace
    std::vector<double> esums_; ///< The Energy sums calculated from the trace

    std::vector<unsigned int> triggerPositions_; ///< Trigger positions in trc.
};

#endif // __TRACE_H_