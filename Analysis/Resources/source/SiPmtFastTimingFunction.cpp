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
 **************************************************************************
*/
/*! \file SiPmtFastTimingFunction.hpp
 *  \brief Definition for the fitting function for the Si PMT fast output
 *  \author S. V. Paulauskas
 *  \date 14 October 2014
 */
#include <cmath>

#include "SiPmtFastTimingFunction.hpp"

double SiPmtFastTimingFunction::operator()(double *x, double *par) {
    double phase = par[0];
    double amp = par[1];
    double sigma = par[2];
    double diff = x[0] - phase;

    double val =
            (amp / (sigma * sqrt(2 * M_PI))) *
            exp(-diff * diff / (2 * sigma * sigma)) + baseline_;

    return (val);
}
