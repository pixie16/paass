/** \file SHECorrelator.hpp
 *
 */

#ifndef __SHECORRELATOR_HPP_
#define __SHECORRELATOR_HPP_

#include <vector>
#include <deque> 
#include <sstream>

enum SheEventType {
    alpha,
    heavyIon,
    fission,
    lightIon,
    unknown
};

class SheEvent {
    public:
        SheEvent();
        SheEvent(double energy, double time, int mwpc, 
                 bool has_beam, bool has_veto, bool has_escape,
                 SheEventType type=unknown);

        ~SheEvent() {}

        double get_energy() const {
            return energy_;
        }
        double get_time() const {
            return time_;
        }
        int get_mwpc() const {
            return mwpc_;
        }
        bool get_beam() const {
            return has_beam_;
        }
        bool get_veto() const {
            return has_veto_;
        }
        bool get_escape() const {
            return has_escape_;
        }
        SheEventType get_type() const {
            return type_;
        }

        void set_energy(double energy) {
            energy_  = energy;
        }
        void set_time(double time) {
            time_ = time;
        }
        void set_mwpc(int mwpc) {
            mwpc_ = mwpc;
        }
        void set_beam(bool has_beam) {
            has_beam_ = has_beam;
        }
        void set_veto(bool has_veto) {
            has_veto_ = has_veto;
        }
        void set_escape(bool has_escape) {
            has_escape_ = has_escape;
        }
        void set_type(SheEventType type) {
            type_ = type;
        }

    private:
        /** Total (reconstructed) energy, may include escape **/
        double energy_;
        /** Shortest time of all subevents (e.g. back and front) */
        double time_;
        /** Number of MWPC chambers hits */
        int mwpc_;
        /** Veto hit flag **/
        bool has_veto_;
        /** Beam on flag **/
        bool has_beam_;
        /** If reconstructed energy includes escape **/
        bool has_escape_;
        /** Type of event decided by Correlator **/
        SheEventType type_;
};


class SheCorrelator {
    public:
        SheCorrelator(int size_x, int size_y);
        ~SheCorrelator();
        bool add_event(SheEvent& event, int x, int y);
        void human_event_info(SheEvent& event, std::stringstream& ss, double clockStart);

    private:
        int size_x_;
        int size_y_;
        std::deque<SheEvent>** pixels_;

        bool flush_chain(int x, int y);
};


#endif
