#ifndef PLOTSREGISTER_H
#define PLOTSREGISTER_H

#include <cstdlib>
#include <iostream>
#include <vector>
#include <utility>

using namespace std;

/** Holds ranges and offsets of all plots. Singleton class. */
class PlotsRegister {
    public:
        static PlotsRegister* R() {
            return &plotsReg_;
        }

        bool checkrange (int offset, int range) {
            bool exists = false;
            int min = offset;
            int max = offset + range - 1;

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

        bool add (int offset, int range) {
            // Special case: empty Plots list
            if (offset == 0 && range == 0)
                return true;

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
            return true;
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
#endif
