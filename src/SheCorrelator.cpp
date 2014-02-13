/*! \file SheCorrelator.cpp
 *
 * The SheCorrelator is designed to recontruct dssd events in a SHE experiment
 * and to correlate chains of decays in dssd pixels
 */

#include "SheCorrelator.hpp"

using namespace std;


SheCorrelator::SheCorrelator(int size_x, int size_y) {
    size_x_ = size_x;
    size_y_ = size_y;
    pixels = new deque<SheEvent>*[size_x];
    for(int i = 0; i < size_y; ++i)
        pixels[i] = new deque<SheEvent>[size_y];
}


SheCorrelator::~SheCorrelator() {
    for(int i = 0; i < size_y_; ++i) {
        delete[] pixels[i];
    }
    delete[] pixels;
}


void SheCorrelator::add_event(SheEvent& event, int x, int y) {
    bool high_energy = false;
    if (event.energy > e_cut_)
        high_energy = true;

    if (high_energy) {
        if (event.has_veto) {
            if (event.mwpc > 0) {
                if (event.has_beam)
                    event.type = lightIon;
                else
                    event.type = unknown;
            } else {
                if (event.has_beam)
                    event.type = unknown;
                else
                    event.type = fission;
            }
        }
        else {
            if (event.mwpc > 0) {
                if (event.has_beam)
                    event.type = lightIon;
                else
                    event.type = fission;
            } else {
                event.type = fission;
            }
        }
    }
    else {
        if (event.has_veto) {
            if (event.mwpc > 0) {
                if (event.has_beam)
                    event.type = lightIon;
                else
                    event.type = unknown;
            } else {
                if (event.has_beam)
                    event.type = lightIon;
                else
                    event.type = unknown;
            }
        }
        else {
            if (event.mwpc > 0) {
                if (event.has_beam)
                    event.type = heavyIon;
                else
                    event.type = decay;
            } else {
                event.type = decay;
            }
        }
    }

    if (event.type == heavyIon)
        save_and_flush_chain(x, y);

    pixels_[x][y].push_back(event);

    if (event.type == fission)
        save_and_flush_chain(x, y);

}

void SheCorrelator::save_and_flush_chain(int x, int y) {
    for (deque<SheEvent>::iterator it = pixels_[x][y].begin();
         it != pixels_[x][y].end();
         ++it)
        cout << it->energy << endl;

    pixels_[x][y].clear();

}
