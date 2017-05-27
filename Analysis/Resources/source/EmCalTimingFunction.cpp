/**************************************************************************
 *  Copyright S. V. Paulauskas 2014                                       *
 *                                                                        *
 *  This program is free software: you can redistribute it and/or modify  *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation, version 3.0 License.                    *
 *                                                                        *
 *  This program is distributed in the hope that it will be useful,       *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *  GNU General Public License for more details.                          *
 *                                                                        *
 *  You should have received a copy of the GNU General Public License     *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *************************************************************************/
/*! \file EmCalTimingFunction.hpp
 *  \brief A class to handle the processing of traces
 *  \author S. V. Paulauskas
 *  \date 03 October 2014
 */
#include <cmath>

#include "EmCalTimingFunction.hpp"

double EmCalTimingFunction::operator()(double *x, double *par) {
    double phase = par[0];
    double amp = par[1];
    double n = par[2];
    double tau = par[3];
    double xprime = (x[0] - phase) / tau;

    double val;
    if (x[0] < phase)
        val = baseline_;
    else
        val = amp * pow(xprime, n) * exp(-xprime) + baseline_;

    return (val);
}
