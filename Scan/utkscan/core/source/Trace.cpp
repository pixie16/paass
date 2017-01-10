/** \file Trace.cpp
 * \brief Implement how to do our usual tricks with traces
 */
#include <algorithm>
#include <iostream>
#include <cmath>
#include <numeric>

#include "Trace.hpp"

using namespace std;

namespace dammIds {
    namespace trace {
    }
}

///This creates the static instance of the Plots class before main. This may
///cause a static initialization order fiasco. Be AWARE!!
Plots Trace::histo(dammIds::trace::OFFSET, dammIds::trace::RANGE, "traces");

void Trace::Plot(int id) {
    for (size_type i=0; i < size(); i++) {
        histo.Plot(id, i, 1, (int)at(i));
    }
}

void Trace::Plot(int id, int row) {
    for (size_type i=0; i < size(); i++) {
        histo.Plot(id, i, row, (int)at(i));
    }
}

void Trace::ScalePlot(int id, double scale) {
    for (size_type i=0; i < size(); i++) {
        histo.Plot(id, i, 1, abs((int)at(i)) / scale);
    }
}

void Trace::ScalePlot(int id, int row, double scale) {
    for (size_type i=0; i < size(); i++) {
        histo.Plot(id, i, row, abs((int)at(i)) / scale);
    }
}

void Trace::OffsetPlot(int id, double offset) {
    for (size_type i=0; i < size(); i++) {
        histo.Plot(id, i, 1, max(0., (int)at(i) - offset));
    }
}

void Trace::OffsetPlot(int id, int row, double offset) {
    for (size_type i=0; i < size(); i++) {
        histo.Plot(id, i, row, max(0., (int)at(i) - offset));
    }
}
