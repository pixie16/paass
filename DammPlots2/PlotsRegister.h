#ifndef PLOTSREGISTER_H
#define PLOTSREGISTER_H

#include <cstdlib>
#include <vector>
#include <utility>


using namespace std;

/** Holds ranges and offsets of all plots. Singleton class. */
class PlotsRegister {
    public:
        static PlotsRegister* R() {
            return &plots_;
        }

        bool checkrange (int offset, int range) {
            bool exists = false;
            unsigned sz = register_.size();
            for (unsigned id = 0; id < sz; ++id) {
                if (min > register_[id].second && max < register_[id].first)
                    continue;
                if ( (min > register_[id].first && min < register_[id].second) || 
                     (max > register_[id].first && max < register_[id].second) ) {
                    exists = true;
                    break;
                }
            }
            return exists;
        }

        void add (int offset, int range) {
            int min = offset;
            int max = offset + range - 1;

            if (max < min) {
                cerr << "Attempt to register incorrect histogram ids range: " << min << " to" << max << endl;
                exit(1);
            }

            if (min < 1 || max > 7999) {
                cerr << "Attempt to register histogram ids: " << min << " to " << max << endl;
                cerr << "Valid range is 1 to 7999" << endl;
                exit(1);
            }

            bool exists = false;
            unsigned sz = register_.size();
            unsigned id = 0;
            for (; id < sz; ++id) {
                if (min > register_[id].second && max < register_[id].first)
                    continue;
                if ( (min > register_[id].first && min < register_[id].second) || 
                     (max > register_[id].first && max < register_[id].second) ) {
                    exists = true;
                    break;
                }
            }
            if (exists) {
                cerr << "Attempt to register histogram ids: " << min << " to " << max << endl;
                cerr << "This range is already registered: " << register_[id].first << " to " << register_[id].second << endl;
                exit(1);
            }

            register_.push_back( pair<int, int>(min, max));
        }

    private:

        /** Vector of min, max of histogram numbers */
        vector< pair<int, int> > register_;

        /** Singleton synax stuff */
        static PlotsRegister plotsReg_;
        /** Dummy variable, because it doesn't work with empty constr (?). */
        PlotsRegister(bool create) {}
        /** Private copy-contructor */
        PlotsRegister(const PlotsRegister&);
};

namespace dammIds {
    // Diagnostic spectra (raw, scalers, etc.)
    namespace diagnostic {
        const int OFFSET = 1;
        const int RANGE = 1999;
    }

    namespace mcp {	
        const int OFFSET = 2000;
        const int RANGE = 10;
    }

    namespace scint {
        const int OFFSET = 2100;
        const int RANGE = 100;
    } 

    // in GeProcessor.cpp
    namespace ge {
        const int OFFSET = 2500;
        const int RANGE = 500;
    } 
    namespace logic {
        const int OFFSET = 3000;
        const int RANGE = 80;
    }
    namespace triggerlogic {
        const int OFFSET = 3080;
        const int RANGE = 20;
    }

    namespace vandle{ //in VandleProcessor.cpp
        const int OFFSET = 3100;
        const int RANGE = 900;
    }

    namespace waveformanalyzer{
        const int OFFSET = 4000;
        const int RANGE = 100;
    }

    // in SsdProcessor.cpp
    namespace ssd {
        const int OFFSET = 4100;
        const int RANGE = 100;
    } 

    // in ImplantSsdProcessor.cpp
    namespace implantSsd {
        const int OFFSET = 4200;
        const int RANGE = 100;
    }

    // in MtcProcessor.cpp
    namespace mtc {
        const int OFFSET = 4500;
        const int RANGE = 100;
    } 

    namespace pulserprocessor{ //in PulserProcessor.cpp 
        const int OFFSET = 5000;
        const int RANGE = 100;
    } 

    // in correlator.cpp
    namespace correlator {
        const int OFFSET = 6000;
        const int RANGE = 100;
    } 

    // in dssd_sub.cpp
    namespace dssd {
        const int OFFSET = 7000;
        const int RANGE = 500;
    } 

    namespace trace {
        const int OFFSET = 7500;
        const int RANGE = 200;
    } 
}

#endif
