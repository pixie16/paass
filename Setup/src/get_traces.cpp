/********************************************************************/
/*	trace_gplt.cpp						    */
/*	KM 01/12/2012 */	
/*			       					    */
/********************************************************************/
#include <bitset>
#include <fstream>
#include <iostream>
#include <vector>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// linux iuncludes
#include <getopt.h>

// pixie includes
#include "utilities.h"
#include "PixieInterface.h"
#include "Utility.h"

using namespace std;

class TraceGrabber : public PixieFunction<> {
public:
    TraceGrabber(unsigned short* data_, int trigger_, int maxTries_, size_t loopLength_, bool doFit_ = false) :
	data(data_), trigger(trigger_), maxTries(maxTries_), loopLength(loopLength_), ready() {
	doFit = doFit_;
	for (int i = 0; i < NUMBER_OF_CHANNELS; ++i) {
	    attempts[i] = 0;
	    baseline[i] = NAN;
	}
    }
    bool operator()(PixieFunctionParms<> &par);
    bool Ready(void) {return ready.count() == loopLength;}
private:
    unsigned short* data;   // 2*16*8192 data points
    /**Tau fitting on/off */
    bool doFit;
    /** Trigger level */
    int trigger;
    /** Number of attempts to get trace above trigger level. */
    int maxTries;
    /** Number of channels read (loop length 1 or 16) */
    size_t loopLength;
    /** Number of attempts per channel */
    int attempts[NUMBER_OF_CHANNELS];
    /** Whether this channel is done or not */
    bitset<NUMBER_OF_CHANNELS> ready;
    /** Calculated baseline for each channel */
    float baseline[NUMBER_OF_CHANNELS];

    /** Save data to array */
    void StoreData(short unsigned int* trace,
		   const size_t size, unsigned ch);
    double FitTau(const unsigned short* trace, 
		  unsigned b0, unsigned b1,
		  unsigned x0, unsigned x1);

};

void TraceGrabber::StoreData(short unsigned int* trace, const size_t size, unsigned ch)
{
    int start = 2 * ch * PixieInterface::GetTraceLength();
    unsigned short *writePtr = &data[start];
    
    //? this won't write the last trace value
    for (unsigned int i = 0; i < size; ++i) {
	*writePtr++ = i;
	*writePtr++ = trace[i];
    }
}

double TraceGrabber::FitTau(const unsigned short* trace, size_t b0, size_t b1, size_t x0, size_t x1)
{    
    double baseline = 0;
    for (unsigned i = b0; i < b1; ++i)
        baseline += trace[i];
    baseline = baseline / double(b1 - b0);
 
    //calculate logarithm 
    vector<double> logtrace;
    for (size_t i = x0; i < x1; ++i) {
	double val = trace[i] - baseline;
        if (val > 0)
            logtrace.push_back( log(val) );
        else
            logtrace.push_back( -6.66E-6 ); //such an evil number!
    }

    // fit a line
    double S = (x1 - x0);
    double Sx = ( pow(x1,2) - x1 - pow(x0,2) + x0) / 2.0;
    double Sxx = ( 2.0 * pow(x1,3) - 3.0 * pow(x1,2) + x1
		   -2.0 * pow(x0,3) + 3.0 * pow(x0,2) - x0) / 6.0;
    double Sy = 0;
    double Sxy = 0;

    unsigned sz = logtrace.size();
    for (unsigned i = 0; i < sz; i++) {
        Sy += logtrace[i];
        Sxy += logtrace[i] * i;
    }
    double D = S * Sxx - pow(Sx,2);
    double a1 = (S * Sxy - Sx * Sy) / D;
    //double a0 = (Sxx * Sy - Sx * Sxy) / D;
    
    if ( a1 != 0.0)
        return (-1.0 / a1);
    else
        return -6.66e6;
}

bool TraceGrabber::operator()(PixieFunctionParms<> &par) {
    /* Grab traces the first time around */
    static size_t loopCount = loopLength;
    // Reads traces from whole module
    // Should be run only once per 'loop' (when all individual channels
    // are processed)
    if (loopCount >= loopLength) {
        par.pif.AcquireTraces(par.mod);
        loopCount = 0;
    }
    // If channel ready skip the rest
    if ( ready[par.ch] ) 
        return true;

    const size_t size = PixieInterface::GetTraceLength();
    unsigned short trace[size];
    usleep(10);
    if (par.pif.ReadSglChanTrace(trace, size, par.mod, par.ch)) {
        unsigned long sum = 0;
        if (isnan(baseline[par.ch]) && trigger > 0.0) {
	    size_t baselineSamples = size / 10;
            for (unsigned i = 0; i < baselineSamples; ++i)
                sum += trace[i];
            baseline[par.ch] = float(sum)/float(baselineSamples);
            cout << "CH: " << par.ch << " average " << baseline[par.ch];
            cout << ", trig level = " << trigger + baseline[par.ch] << endl;            
        }

        if (trigger > 0.0 && attempts[par.ch] < maxTries) {
            attempts[par.ch]++;

            bool trig = false;
            int triggerLevel = trigger + baseline[par.ch];

	    size_t x0;
            for (unsigned i = 0; i < size; ++i) {
                if (trace[i] > triggerLevel) {
                    trig = true;
                    x0 = i;
                    break;
                }
            }
            if (trig) {
                ready[par.ch] = true;
                StoreData(trace, size, par.ch);
                cout << "Channel " << par.ch << " ready " << " trig " << trace[x0] << " at " << x0 << endl;
                if (doFit) {
                    size_t x1 = 0;
                    for (size_t i = size - 1; i > x0; --i) {
                        if (trace[i] > triggerLevel - trigger/2) {
                            x1 = i;
                            break;
                        }
                    }
                    // Arbitrary 10 samples between x0 and x1 
                    if (x1 > x0 + 10) {
                        size_t b0, b1;

                        if (x0 > 30) {
                            b0 = 0;
                            b1 = 20;
                        } else {
                            b0 = 8171;
                            b1 = 8191;
                        }
                        // x0 + 5 in order to get rid of growin part
                        //   might be not enough
                        double tau = FitTau(trace, b0, b1, x0+5, x1);

                        cout << "Tau = " << tau << endl;
                        cout << " x0 = " << x0 << ", x1 = " << x1 << endl;
                        cout << " b0 = " << x0 << ", b1 = " << x1 << endl;
                    } else {
			// not enough samples
                        cout << "Could not find tau " << endl;
		    }
                }
            }
        } else if (attempts[par.ch] >= maxTries) {
            attempts[par.ch]++;
            ready[par.ch] = true;
            StoreData(trace, size, par.ch);
            cout << "Channel " << par.ch << " exceeded max number of attempts " << endl;
        } else if (trigger == 0.0 ) {
            ready[par.ch] = true;
            StoreData(trace, size, par.ch);
            cout << "Channel " << par.ch << " ready, triggerless" << endl;
        }
        return true;
    } else {
	// Did not successfully read trace
        return false;
    }
}

int main(int argc, char *argv[]) {
    int mod = -1;
    int ch  = -1;
    int trig = 0;
    int maxAttempts = 100;
    bool tau = false;

    const option longOptions[] = {
	{"mod", required_argument, 0, 'm'},
	{"ch", required_argument, 0, 'c'},
	{"trig", required_argument, 0, 't'},
	{"tau", no_argument, 0, 'u'},
	{"help", no_argument, 0, 'h'},
	{0, 0, 0, 0}
    };
    int flag, optionsIndex;

    // getopt_long returns -1 when all is done
    while ( (flag = getopt_long (argc, argv, "m:c:t:n:uh", longOptions, &optionsIndex)) != -1) {
        switch (flag) {
	case 'm':
	    mod = atoi(optarg);
	    break;
	case 'c':
	    ch = atoi(optarg);
	    break;
	case 't':
	    trig = atoi(optarg);
	    break;
	case 'n':
	    maxAttempts = atoi(optarg);
	    break;
	case 'u':	    
	    tau = true;
	    break;
	case 'h':	    
	case '?':
	    ;
	    cout << "Usage: " << argv[0] << " -m <module_number> [-c <ch_number> -t <trig_level> -n <number_of_attempts>] " << endl;
	    cout << "Flags: " << endl;
	    cout << " -m, --mod   -  module number (required) " << endl;
	    cout << " -c, --ch    -  channel number (0-15 or -1), -1 (default) = all channels " << endl;
	    cout << " -t, --trig  -  trigger level in number of samples above baseline, default = 0 (no trigger) " << endl;
	    cout << " -n          -  number of attempts to catch good trigger, default = 100 " << endl;
	    cout << " -h, --help  -  shows this help " << endl;
	    if (flag == 'h')
		exit(EXIT_SUCCESS);
	    exit(EXIT_FAILURE);
        }
    }

    PixieInterface pif("pixie.cfg");
    pif.GetSlots();

    // Only one module supported
    if (!inRange(mod, (int)pif.GetNumberCards()) ) {
        cout << "Wrong module number" << endl;
        cout << "See " << argv[0] << " -h  for help" << endl;
        exit(EXIT_FAILURE);
    }
    // Only 1 from 16 channels (or all) available
    if ( !inRange(ch, -1, (int)pif.GetNumberChannels()) ) {
	cout << "Wrong channel number" << endl;
	cout << "See " << argv[0] << " -h  for help" << endl;
	exit(EXIT_FAILURE);
    }
    
    pif.Init();
            
    usleep(200);
    pif.Boot(PixieInterface::DownloadParameters |
	     PixieInterface::ProgramFPGA |
	     PixieInterface::SetDAC, true);

    const unsigned size = 2 * pif.GetNumberChannels() * PixieInterface::GetTraceLength();
    unsigned short data[size];
    memset(data, 0, sizeof(data));

    int loopLength = 0;
    if (ch == -1) {
        loopLength = pif.GetNumberChannels();
    } else {
        loopLength = 1;
        for (size_t i = 0; i < pif.GetNumberChannels(); ++i) {
            //This prevents gnuplot from empty range warnings
            data[(2 * i + 2) * PixieInterface::GetTraceLength() - 2] = PixieInterface::GetTraceLength() - 1;
            data[(2 * i + 2) * PixieInterface::GetTraceLength() - 1] = 1;
        }
    }
    TraceGrabber grabber(data, trig, maxAttempts, loopLength, tau);

    int counter = 0;
    while (!grabber.Ready()) {
        forChannel(pif, mod, ch, grabber);
        cout << "|" << ++counter << " |\r" << flush; 
    }
    cout << "Counter : " << counter << endl;

    ofstream fout("/tmp/traces.dat", ios::binary);
    if (!fout.good()) {
        fprintf(stderr, "Could not open traces.dat\n");
        exit(EXIT_FAILURE);
    }
    fout.write((char* )data, sizeof(data));
    fout.close();

    system("gnuplot 'plotTraces' ");
    cout << "Traces data are in '/tmp/traces.dat' file." << endl;

    return EXIT_SUCCESS;
}

