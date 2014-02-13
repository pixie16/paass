/*! \file SheCorrelator.cpp
 *
 * The SheCorrelator is designed to recontruct dssd events in a SHE experiment
 * and to correlate chains of decays in dssd pixels
 */

#include "SheCorrelator.hpp"
#include <iostream>

using namespace std;

SheEvent::SheEvent() {
    energy_ = -1.0;
    time_= -1.0;
    mwpc_= -1;
    has_beam_= false;
    has_veto_= false;
    has_escape_ = false;
    type_= unknown;
}

SheEvent::SheEvent(double energy, double time, int mwpc, 
                   bool has_beam, bool has_veto, bool has_escape,
                   SheEventType type) {
    set_energy(energy);
    set_time(time);
    set_mwpc(mwpc);
    set_beam(has_beam);
    set_veto(has_veto);
    set_escape(has_escape);
    set_type(type);
}


SheCorrelator::SheCorrelator(int size_x, int size_y) {
    size_x_ = size_x;
    size_y_ = size_y;
    pixels_ = new deque<SheEvent>*[size_x];
    for(int i = 0; i < size_y; ++i)
        pixels_[i] = new deque<SheEvent>[size_y];
}


SheCorrelator::~SheCorrelator() {
    for(int i = 0; i < size_y_; ++i) {
        delete[] pixels_[i];
    }
    delete[] pixels_;
}


void SheCorrelator::add_event(SheEvent& event, int x, int y) {
    cout << "Adding event set_type() " << event.get_type() << endl;

    if (event.get_type() == heavyIon)
        save_and_flush_chain(x, y);

    pixels_[x][y].push_back(event);

    if (event.get_type() == fission)
        save_and_flush_chain(x, y);

}

void SheCorrelator::save_and_flush_chain(int x, int y) {
    cout << "Flush chain at " << x <<  " " << y << endl;
    for (deque<SheEvent>::iterator it = pixels_[x][y].begin();
         it != pixels_[x][y].end();
         ++it)
        cout << it->get_energy() << endl;
    pixels_[x][y].clear();

}
