///@authors C. R. Thornsberry
#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <cmath>
#include <unistd.h>

#include <pixie16/pixie16.h>

#include "Display.h"

#include "PixieSupport.h"

std::string PadStr(const std::string &input_, int width_) {
    std::string output = input_;
    for (int i = output.size(); i < width_; i++) {
        output += ' ';
    }

    return output;
}

bool BitFlipper::operator()(PixieFunctionParms <std::string> &par) {
    if (bit >= num_toggle_bits) { return false; }

    bool *active_bits = new bool[num_toggle_bits];
    int *bit_values = new int[num_toggle_bits];
    int count = 1;

    double value;
    par.pif->ReadSglChanPar(par.par.c_str(), value, par.mod, par.ch);

    int old_csra = (int) value;
    for (unsigned int i = 0; i < num_toggle_bits; i++) {
        bit_values[i] = count;
        count *= 2;
        if (old_csra & (1 << i)) { active_bits[i] = true; }
        else { active_bits[i] = false; }
    }

    int new_csra;
    if (active_bits[bit]) { // need to toggle bit off (subtract)
        new_csra = old_csra - bit_values[bit];
    } else { // need to toggle bit on (add)
        new_csra = old_csra + bit_values[bit];
    }

    if (par.pif->WriteSglChanPar(par.par.c_str(), new_csra, par.mod, par.ch)) {
        par.pif->PrintSglChanPar(par.par.c_str(), par.mod, par.ch, value);
        return true;
    }

    delete[] active_bits;
    delete[] bit_values;

    return false;
}

#ifdef PIF_REVA
const std::vector<std::string> BitFlipper::toggle_names({"group", "live", "good", "read", "trigger", "polarity", "GFLT", "", "",
                                                         "", "", "", "", "", "gain", "", "", "", ""});
const std::vector<std::string> BitFlipper::csr_txt({"Respond to group triggers only", "Measure individual live time", "Good Channel", "Read always", "Enable trigger",
                                                    "Trigger positive", "GFLT", "", "", "", "", "", "", "", "HI/LO gain", "", "", "", ""});
#else
const std::vector<std::string> BitFlipper::toggle_names(
        {"", "", "good", "", "", "polarity", "", "", "trace", "QDC", "CFD",
         "globtrig", "raw", "chantrig", "gain", "pileup", "catcher", "", "SHE","","","ExtTime","ExtFastTrig","PeakSamp"});
const std::vector<std::string> BitFlipper::csr_txt(
        {"", "", "Good Channel", "", "", "Trigger positive", "", "",
         "Enable trace capture", "Enable QDC sums capture",
         "Enable CFD trigger mode", "Enable global trigger validation",
         "Enable raw energy sums capture",
         "Enable channel trigger validation", "LO/HI gain",
         "Pileup Rejection", "Inverse Pileup ", "",
         "SHE single trace capture (grouptrigsel)","","","Enable External Timestamping","Enable External Fast Trigger","Enable Override Peak Sample Point"});
#endif

void BitFlipper::Help() {
    std::cout << " Valid CSRA bits:\n";
    for (unsigned int i = 0; i < num_toggle_bits; i++) {
        if (toggle_names[i] != "") {
            if (i < 10) {
                std::cout << "  0" << i << " - " << toggle_names[i] << std::endl;
            } else {
                std::cout << "  " << i << " - " << toggle_names[i] << std::endl;
            }
        } else {
            if (i < 10) {
                std::cout << "  0" << i << " - " << toggle_names[i] << std::endl;
            } else {
                std::cout << "  " << i << " - " << toggle_names[i] << std::endl;
            }
        }
    }
}

void BitFlipper::SetCSRAbit(std::string bit_) {
    SetBit(atoi(bit_.c_str()));

    for (unsigned int i = 0; i < num_toggle_bits; i++) {
        if (bit_ == toggle_names[i]) {
            SetBit(i);
            break;
        }
    }
}

void BitFlipper::SetBit(std::string bit_) {
    SetBit(std::strtoul(bit_.c_str(), NULL, 0));
}

void BitFlipper::CSRAtest(unsigned int input_) {
    Test(24, input_, csr_txt);
}

bool BitFlipper::Test(unsigned int num_bits_, unsigned int input_,
                      const std::vector<std::string> &text_) {
    if (num_bits_ > 32) { return false; } // Too many bits for unsigned int

    bool *active_bits = new bool[num_bits_];
    unsigned int *bit_values = new unsigned int[num_bits_];
    unsigned int *running_total = new unsigned int[num_bits_];

    unsigned int total = 0;
    unsigned int count = 1;

    for (unsigned int i = 0; i < num_bits_; i++) {
        bit_values[i] = count;
        if (input_ & (1 << i)) {
            active_bits[i] = true;

            total += count;
            running_total[i] = total;
        } else {
            active_bits[i] = false;
            running_total[i] = 0;
        }
        count *= 2;
    }

    std::cout << " Input: 0x" << std::hex << input_ << " (" << std::dec
              << input_ << ")\n";
    if (!text_.empty()) {
        std::cout << "  Bit  On?  Value       Total	  Bit Function\n";
    } else { std::cout << "  Bit   On?	Value	   Total\n"; }

    std::string bit_function;
    for (unsigned int i = 0; i < num_bits_; i++) {
        if (!text_.empty()) { bit_function = csr_txt[i]; }
        else { bit_function = ""; }

        if (active_bits[i]) {
            if (Display::hasColorTerm) {
                if (i < 10) {
                    std::cout << TermColors::DkGreen << "  0" << i << "	1   "
                              << PadStr(bit_values[i], 12);
                    std::cout << PadStr(running_total[i], 10) << bit_function
                              << TermColors::Reset << std::endl;
                } else {
                    std::cout << TermColors::DkGreen << "  " << i << "	1   "
                              << PadStr(bit_values[i], 12);
                    std::cout << PadStr(running_total[i], 10) << bit_function
                              << TermColors::Reset << std::endl;
                }
            } else {
                if (i < 10) {
                    std::cout << "  0" << i << "	1   "
                              << PadStr(bit_values[i], 12)
                              << PadStr(running_total[i], 10) << bit_function
                              << std::endl;
                } else {
                    std::cout << "  " << i << "	1   "
                              << PadStr(bit_values[i], 12)
                              << PadStr(running_total[i], 10) << bit_function
                              << std::endl;
                }
            }
        } else {
            if (i < 10) {
                std::cout << "  0" << i << "	0   "
                          << PadStr(bit_values[i], 12);
                std::cout << PadStr(running_total[i], 10) << bit_function
                          << std::endl;
            } else {
                std::cout << "  " << i << "	0   "
                          << PadStr(bit_values[i], 12);
                std::cout << PadStr(running_total[i], 10) << bit_function
                          << std::endl;
            }
        }
    }

    delete[] active_bits;
    delete[] bit_values;
    delete[] running_total;

    return true;
}

double
GetTraces::FitTau(const unsigned short *trace, size_t b0, size_t b1, size_t x0,
                  size_t x1) {
    double baseline = 0;
    for (unsigned i = b0; i < b1; ++i)
        baseline += trace[i];
    baseline = baseline / double(b1 - b0);

    //calculate logarithm
    std::vector<double> logtrace;
    for (size_t i = x0; i < x1; ++i) {
        double val = trace[i] - baseline;
        if (val > 0)
            logtrace.push_back(std::log(val));
        else
            logtrace.push_back(-6.66E-6); //such an evil number!
    }

    // fit a line
    double S = (x1 - x0);
    double Sx = (std::pow(x1, 2) - x1 - std::pow(x0, 2) + x0) / 2.0;
    double Sxx = (2.0 * std::pow(x1, 3) - 3.0 * std::pow(x1, 2) + x1
                  - 2.0 * std::pow(x0, 3) + 3.0 * std::pow(x0, 2) - x0) / 6.0;
    double Sy = 0;
    double Sxy = 0;

    unsigned sz = logtrace.size();
    for (unsigned i = 0; i < sz; i++) {
        Sy += logtrace[i];
        Sxy += logtrace[i] * i;
    }
    double D = S * Sxx - pow(Sx, 2);
    double a1 = (S * Sxy - Sx * Sy) / D;
    //double a0 = (Sxx * Sy - Sx * Sxy) / D;

    if (a1 != 0.0)
        return (-1.0 / a1);
    else
        return -6.66e6;
}

GetTraces::GetTraces(unsigned short *total_data_, size_t total_size_,
                     unsigned short *trace_data_, size_t trace_size_,
                     int threshold_/*=0*/, bool correct_baselines_/*=false*/) {
    total_data = total_data_;
    total_len = total_size_;
    trace_data = trace_data_;
    trace_len = trace_size_;
    threshold = threshold_;
    correct_baselines = correct_baselines_;

    attempts = 0;
    status = false;

    // Set initial values.
    for (unsigned int i = 0; i < NUMBER_OF_CHANNELS; i++) {
        baseline[i] = -1;
        maximum[i] = -9999;
    }
}

void GetTraces::Help() {
}

bool GetTraces::operator()(PixieFunctionParms<int> &par) {
    float temp_val;

    // Reset parameters.
    for (unsigned int i = 0; i < NUMBER_OF_CHANNELS; i++) {
        baseline[i] = -1;
        maximum[i] = -9999;
    }
    status = false;

    size_t baselineSamples = trace_len / 10;
    unsigned long sum;

    // Get a single channel's trace.
    // Try to find a pulse above the threshold.
    for (attempts = 1; attempts <= 100; attempts++) {
        par.pif->AcquireTraces(par.mod); // Acquire new traces.
        usleep(10);

        if (par.pif->ReadSglChanTrace(trace_data, trace_len, par.mod, par.ch)) {
            // Reset the parameters for the trigger channel.
            baseline[par.ch] = -1;
            maximum[par.ch] = -9999;

            // Calculate the channel baseline.
            sum = 0;

            for (unsigned i = 0; i < baselineSamples; i++)
                sum += trace_data[i];

            baseline[par.ch] = float(sum) / float(baselineSamples);

            // Correct the baseline
            //for (unsigned i = 0; i < trace_len; ++i)
            //data[i] = data[i] - baseline[par.ch];

            // Find the maximum value above baseline.
            for (unsigned i = 0; i < trace_len; i++) {
                temp_val = trace_data[i] - baseline[par.ch];
                if (temp_val > maximum[par.ch]) { maximum[par.ch] = temp_val; }

                if (!correct_baselines) {
                    total_data[(par.ch * trace_len) + i] = trace_data[i];
                } else { total_data[(par.ch * trace_len) + i] = temp_val; }
            }

            // Check that this "trace" is above threshold.
            if (maximum[par.ch] < threshold) { continue; }

            status = true;
            break;
        }
    }

    // Threshold was not reached. Copy the most recent pulse into
    // the module traces array.
    for (unsigned int i = 0; i < NUMBER_OF_CHANNELS; i++) {
        if (i == par.ch) { continue; } // Already did this channel.

        if (par.pif->ReadSglChanTrace(trace_data, trace_len, par.mod, i)) {
            // Calculate the channel baseline.
            sum = 0;

            for (unsigned j = 0; j < baselineSamples; j++)
                sum += trace_data[j];

            baseline[i] = float(sum) / float(baselineSamples);

            // Correct the baseline
            //for (unsigned i = 0; i < trace_len; ++i)
            //data[i] = data[i] - baseline[par.ch];

            // Find the maximum value above baseline.
            for (unsigned j = 0; j < trace_len; j++) {
                temp_val = trace_data[j] - baseline[i];
                if (temp_val > maximum[i]) { maximum[i] = temp_val; }

                if (!correct_baselines) {
                    total_data[(i * trace_len) + j] = trace_data[j];
                } else { total_data[(i * trace_len) + j] = temp_val; }
            }
        }
    }

    return status;
}

bool ParameterChannelWriter::operator()(
        PixieFunctionParms <std::pair<std::string, double>> &par) {
    double previousValue;
    if (par.pif->WriteSglChanPar(par.par.first.c_str(), par.par.second, par.mod,
                                 par.ch, previousValue)) {
        par.pif->PrintSglChanPar(par.par.first.c_str(), par.mod, par.ch,
                                 previousValue);
        return true;
    }
    return false;
}

bool ParameterModuleWriter::operator()(
        PixieFunctionParms <std::pair<std::string, unsigned int>> &par) {
    unsigned int previousValue;
    if (par.pif->WriteSglModPar(par.par.first.c_str(), par.par.second, par.mod,
                                previousValue)) {
        par.pif->PrintSglModPar(par.par.first.c_str(), par.mod, previousValue);
        return true;
    }
    return false;
}

bool ParameterChannelReader::operator()(PixieFunctionParms <std::string> &par) {
    par.pif->PrintSglChanPar(par.par.c_str(), par.mod, par.ch);
    return true;
}

bool ParameterModuleReader::operator()(PixieFunctionParms <std::string> &par) {
    par.pif->PrintSglModPar(par.par.c_str(), par.mod);
    return true;
}

bool ParameterChannelDumper::operator()(PixieFunctionParms <std::string> &par) {
    double value;
    par.pif->ReadSglChanPar(par.par.c_str(), value, (int) par.mod,
                            (int) par.ch);
    *file << par.mod << "\t" << par.ch << "\t" << par.par << "\t" << value
          << std::endl;
    return true;
}

bool ParameterModuleDumper::operator()(PixieFunctionParms <std::string> &par) {
    PixieInterface::word_t value;
    par.pif->ReadSglModPar(par.par.c_str(), value, (int) par.mod);
    *file << par.mod << "\t" << par.par << "\t" << value << std::endl;
    return true;
}

bool OffsetAdjuster::operator()(PixieFunctionParms<int> &par) {
    bool hadError = par.pif->AdjustOffsets(par.mod);
    for (size_t ch = 0; ch < par.pif->GetNumberChannels(); ch++) {
        par.pif->PrintSglChanPar("VOFFSET", par.mod, ch);
    }

    return hadError;
}

bool TauFinder::operator()(PixieFunctionParms<> &par) {
    double tau[16];

    int errorNum = Pixie16TauFinder(par.mod, tau);
    if (par.ch < 16) {
        std::cout << "TAU: " << tau[par.ch] << std::endl;
    }
    std::cout << "Errno: " << errorNum << std::endl;

    return (errorNum >= 0);
}
