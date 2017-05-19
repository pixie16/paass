///@file ScopeUnpacker.cpp
///@brief
///@author S. V. Paulauskas
///@date May 19, 2017
#include <ctime>
#include <vector>
#include <deque>
#include <cmath>
#include <string>

// Root files
#include <TSystem.h>
#include <TStyle.h>
#include <TMath.h>
#include <TGraph.h>
#include <TH2F.h>
#include <TAxis.h>
#include <TFile.h>
#include <TF1.h>
#include <TLine.h>
#include <TProfile.h>
#include <TPaveStats.h>

#include "HelperFunctions.hpp"
#include "ScopeUnpacker.hpp"

using namespace std;
using namespace TraceFunctions;

#define ADC_TIME_STEP 4 // ns

/// Default constructor.
ScopeUnpacker::ScopeUnpacker(const unsigned int &mod/*=0*/,
                             const unsigned int &chan/*=0*/) : Unpacker() {
    mod_ = mod;
    chan_ = chan;
    threshLow_ = 0;
    threshHigh_ = -1;
    resetGraph_ = false;

    graph = new TGraph();
    hist = new TH2F("hist", "", 256, 0, 1, 256, 0, 1);

    cfdLine = new TLine();
    cfdLine->SetLineColor(kRed);
    cfdPol3 = new TF1("cfdPol3", "pol3");
    cfdPol3->SetLineColor(kGreen + 1);
    cfdPol2 = new TF1("cfdPol2", "pol2");
    cfdPol2->SetLineColor(kMagenta + 1);

    SetupFunc();

    gStyle->SetPalette(51);

    //Display the stats: Integral
    gStyle->SetOptStat(1000000);

    //Display Fit Stats: Fit Values, Errors, and ChiSq.
    gStyle->SetOptFit(111);
}

ScopeUnpacker::~ScopeUnpacker() {
    delete graph;
    delete cfdLine;
    delete cfdPol3;
    delete cfdPol2;
    delete hist;
    delete fittingFunction_;
    delete vandleTimingFunction_;
}

void ScopeUnpacker::ResetGraph(unsigned int size) {
    delete graph;

    graph = new TGraph(size);
    graph->SetMarkerStyle(kFullDotSmall);

    if (size != x_vals.size()) {
        cout << "ScopeUnpacker::ResetGraph : " << "Changing trace length from "
             << x_vals.size() << " to " << size
             << " ns.\n";
        x_vals.resize(size);
        for (size_t index = 0; index < x_vals.size(); index++)
            x_vals[index] = index;
    }

    hist->SetBins(x_vals.size(), x_vals.front(), x_vals.back(),
                  1, 0, 1);

    stringstream stream;
    stream << "M" << GetMod() << "C" << GetChan();
    graph->SetTitle(stream.str().c_str());
    hist->SetTitle(stream.str().c_str());

    resetGraph_ = false;
}

TF1 *ScopeUnpacker::SetupFunc() {
    vandleTimingFunction_ = new VandleTimingFunction();
    fittingFunction_ =
            new TF1("func", vandleTimingFunction_, 0., 1.e6, 5);
    fittingFunction_->SetParNames("phase", "amplitude", "beta", "gamma",
                                  "baseline");
    return fittingFunction_;
}

/** Process all events in the event list.
  * \param[in]  addr_ Pointer to a location in memory.
  * \return Nothing.
  */
void ScopeUnpacker::ProcessRawEvent() {
    XiaData *current_event = NULL;

    // Fill the processor event deques with events
    while (!rawEvent.empty()) {
        if (!running)
            break;

        //Get the first event in the FIFO.
        current_event = rawEvent.front();
        rawEvent.pop_front();

        // Safety catches for null event or empty ->GetTrace().
        if (!current_event || current_event->GetTrace().empty()) {
            continue;
        }

        // Pass this event to the correct processor
        double maximum =
                FindMaximum(current_event->GetTrace(),
                            current_event->GetTrace().size()).second;
        if (current_event->GetModuleNumber() == mod_ &&
            current_event->GetChannelNumber() == chan_) {
            //Check threhsold.
            if (maximum < threshLow_) {
                delete current_event;
                continue;
            }
            if (threshHigh_ > threshLow_ && maximum > threshHigh_) {
                delete current_event;
                continue;
            }

            //Store the waveform in the stack of waveforms to be displayed.
            if (AddEvent(current_event))
                ProcessEvents();
        }
    }
}

void ScopeUnpacker::Plot() {
    static float histAxis[2][2];

    if (chanEvents_.size() < numAvgWaveforms_)
        return;

    unsigned int traceSize = chanEvents_.front()->GetTrace().size();

    if (traceSize != x_vals.size())
        resetGraph_ = true;

    if (resetGraph_) {
        ResetGraph(traceSize);
        //ResetZoom();
        for (int i = 0; i < 2; i++) {
            histAxis[i][0] = 1E9;
            histAxis[i][1] = -1E9;
        }
    }

    if (numAvgWaveforms_ == 1) {
        int index = 0;
        for (size_t i = 0; i < traceSize; ++i, index++)
            graph->SetPoint(index, x_vals[i],
                            chanEvents_.front()->GetTrace().at(i));

        //UpdateZoom();

        graph->Draw("AP0");

        float lowVal = (chanEvents_.front()->GetTrace().GetMaxInfo().first -
                        fitLow_);
        float highVal = (chanEvents_.front()->GetTrace().GetMaxInfo().first +
                         fitHigh_);

        ///@TODO Renable the CFD with the proper functionality.
        /*
        if(performCfd_){
            ProcessedXiaData *evt = chanEvents_.front();

            // Find the zero-crossing of the cfd waveform.
            float cfdCrossing = evt->AnalyzeCFD(cfdF_);

            // Draw the cfd crossing line.
            cfdLine->DrawLine(cfdCrossing*ADC_TIME_STEP, userZoomVals[1][0], cfdCrossing*ADC_TIME_STEP, userZoomVals[1][1]);

            // Draw the 3rd order polynomial.
            cfdPol3->SetParameter(0, evt->cfdPar[0]);
            cfdPol3->SetParameter(1, evt->cfdPar[1]/ADC_TIME_STEP);
            cfdPol3->SetParameter(2, evt->cfdPar[2]/pow(ADC_TIME_STEP, 2.0));
            cfdPol3->SetParameter(3, evt->cfdPar[3]/pow(ADC_TIME_STEP, 3.0));
            // Find the pulse maximum by fitting with a third order polynomial.
            if(evt->event->adcTrace[evt->max_index-1] >= evt->event->adcTrace[evt->max_index+1]) // Favor the left side of the pulse.
                cfdPol3->SetRange((evt->max_index - 2)*ADC_TIME_STEP, (evt->max_index + 1)*ADC_TIME_STEP);
            else // Favor the right side of the pulse.
                cfdPol3->SetRange((evt->max_index - 1)*ADC_TIME_STEP, (evt->max_index + 2)*ADC_TIME_STEP);
            cfdPol3->Draw("SAME");

            // Draw the 2nd order polynomial.
            cfdPol2->SetParameter(0, evt->cfdPar[4]);
            cfdPol2->SetParameter(1, evt->cfdPar[5]/ADC_TIME_STEP);
            cfdPol2->SetParameter(2, evt->cfdPar[6]/pow(ADC_TIME_STEP, 2.0));
            cfdPol2->SetRange((evt->cfdIndex - 1)*ADC_TIME_STEP, (evt->cfdIndex + 1)*ADC_TIME_STEP);
            cfdPol2->Draw("SAME");
        }
         */

        if (performFit_) {
            fittingFunction_->SetParameters(lowVal,
                                            0.5 *
                                            chanEvents_.front()->GetTrace().GetQdc(),
                                            0.3, 0.1);
            fittingFunction_->FixParameter(
                    4, chanEvents_.front()->GetTrace().GetBaselineInfo().first);
            graph->Fit(fittingFunction_, "WRQ", "", lowVal, highVal);
        }
    } else {
        //For multiple events with make a 2D histogram and plot the profile on top.
        //Determine the maximum and minimum values of the events.
        for (unsigned int i = 0; i < numAvgWaveforms_; i++) {
            ProcessedXiaData *evt = chanEvents_.at(i);
            float evtMin = *min_element(evt->GetTrace().begin(),
                                        evt->GetTrace().end());
            float evtMax = *max_element(evt->GetTrace().begin(),
                                        evt->GetTrace().end());
            evtMin -= fabs(0.1 * evtMax);
            evtMax += fabs(0.1 * evtMax);
            if (evtMin < histAxis[1][0]) histAxis[1][0] = evtMin;
            if (evtMax > histAxis[1][1]) histAxis[1][1] = evtMax;
        }

        //Reset the histogram
        hist->Reset();

        //Rebin the histogram
        hist->SetBins(x_vals.size(), x_vals.front(),
                      x_vals.back(),
                      histAxis[1][1] - histAxis[1][0],
                      histAxis[1][0], histAxis[1][1]);

        //Fill the histogram
        for (unsigned int i = 0; i < numAvgWaveforms_; i++) {
            ProcessedXiaData *evt = chanEvents_.at(i);
            for (size_t i = 0; i < evt->GetTrace().size(); ++i) {
                hist->Fill(x_vals[i], evt->GetTrace()[i]);
            }
        }

        prof = hist->ProfileX("AvgPulse");
        prof->SetLineColor(kRed);
        prof->SetMarkerColor(kRed);

        double lowVal = prof->GetBinCenter(prof->GetMaximumBin() - fitLow_);
        double highVal = prof->GetBinCenter(prof->GetMaximumBin() + fitHigh_);

        if (performFit_) {
            fittingFunction_->SetParameters(lowVal,
                                            0.5 *
                                            chanEvents_.front()->GetTrace().GetQdc(),
                                            0.3, 0.1);
            fittingFunction_->FixParameter(
                    4, chanEvents_.front()->GetTrace().GetBaselineInfo().first);
            hist->Fit(fittingFunction_, "WRQ", "", lowVal, highVal);
        }

        hist->SetStats(false);
        hist->Draw("COLZ");
        prof->Draw("SAMES");

        //UpdateZoom();

        //GetCanvas()->Update();
        TPaveStats *stats = (TPaveStats *) prof->GetListOfFunctions()->FindObject(
                "stats");
        if (stats) {
            stats->SetX1NDC(0.55);
            stats->SetX2NDC(0.9);
        }
    }

    // Remove the events from the deque.
    for (unsigned int i = 0; i < numAvgWaveforms_; i++) {
        delete chanEvents_.front();
        chanEvents_.pop_front();
    }

    // Update the canvas.
//    GetCanvas()->Update();
//
//    // Save the TGraph to a file.
//    if (saveFile_ != "") {
//        TFile f(saveFile_.c_str(), "RECREATE");
//        graph->Clone("trace")->Write();
//        f.Close();
//        saveFile_ = "";
//    }

    num_displayed++;
}

/** Add a channel event to the deque of events to send to the processors.
  * This method should only be called from skeletonUnpacker::ProcessRawEvent().
  * \param[in]  event_ The raw XiaData to add to the channel event deque.
  * \return True if events are ready to be processed and false otherwise.
  */
bool ScopeUnpacker::AddEvent(XiaData *event_) {
    if (!event_) { return false; }

    //Get the firs
    // t event int the FIFO.
    ProcessedXiaData *channel_event = new ProcessedXiaData(*event_);

    //Process the waveform.
    ///@TODO This needs cleaned up quite a bit to make it more generalized
    /// and remove some of the magic numbers.
    channel_event->GetTrace().SetBaseline(CalculateBaseline
                                                  (channel_event->GetTrace(),
                                                   make_pair(0, 10)));

    channel_event->GetTrace().SetMax(FindMaximum
                                             (channel_event->GetTrace(),
                                              channel_event->GetTrace().size()));

    channel_event->GetTrace().SetQdc(CalculateQdc
                                             (channel_event->GetTrace(),
                                              make_pair(5, 15)));

    //Push the channel event into the deque.
    chanEvents_.push_back(channel_event);

    // Handle the individual XiaData.
    if (chanEvents_.size() >= numAvgWaveforms_)
        return true;

    return false;
}

/** Process all channel events read in from the rawEvent.
  * This method should only be called from skeletonUnpacker::ProcessRawEvent().
  * \return True if events were processed and false otherwise.
  */
bool ScopeUnpacker::ProcessEvents() {
    //Check if we have delayed the plotting enough
//    time_t cur_time;
//    time(&cur_time);
//    while (difftime(cur_time, last_trace) < delay_) {
//        //If in shm mode and the plotting time has not alloted the events are cleared and this function is aborted.
//        if (ShmMode()) {
//            ClearEvents();
//            return false;
//        } else {
//            IdleTask();
//            time(&cur_time);
//        }
//    }

    //When we have the correct number of waveforms we plot them.
    Plot();

    //If this is a single capture we stop the plotting.
    if (singleCapture_) running = false;

    //Update the time.
    time(&last_trace);

    return true;
}

void ScopeUnpacker::ClearEvents() {
    while (!chanEvents_.empty()) {
        delete chanEvents_.front();
        chanEvents_.pop_front();
    }
}