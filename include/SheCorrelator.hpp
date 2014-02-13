/** \file SHECorrelator.hpp
 *
 */

#ifndef __SHECORRELATOR_HPP_
#define __SHECORRELATOR_HPP_

#include <vector>
#include <deque> 

enum SheEventType {
    decay,
    heavyIon,
    fission,
    ligthIon,
    unknown
};

struct SheEvent {
    SheEvent() {
        energy = -1.0;
        time = -1.0;
        mwpc = -1.0;
        beam = false;
        type = Unknown;
    }
    /** Total (reconstructed) energy, may include escape **/
    double energy;
    /** Shortest time of all subevents (e.g. back and front) */
    double time;
    /** Number of MWPC chambers hits */
    int mwpc;
    /** Veto hit flag **/
    bool has_veto;
    /** Beam on flag **/
    bool has_beam;
    /** Type of event decided by Correlator **/
    SheEventType type;
};

class SheCorrelator {
    public:
        SheCorrelator(int size_x, int size_y);
        void add_event(SheEvent event, int x, int y);

    private:
        int size_x_;
        int size_y_;
        std::deque<SheEvent>** pixels_;
        double e_cut_ = 15000.0;

        void save_and_flush_chain(int x, int y);
};


#endif
