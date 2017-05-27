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

/*! \file CrystalBallFunction.hpp
 *  \brief A class to handle the processing of traces
 *  \author S. V. Paulauskas
 *  \date November 27, 2016
 */
#include <cmath>

#include "CrystalBallFunction.hpp"

double CrystalBallFunction::operator()(double *x, double *par) {
    double mu = par[0];
    double amplitude = par[1];
    double alpha = par[2];
    double n = par[3];
    double sigma = par[4];

    double t = (x[0] - mu) / sigma;

    if (alpha < 0)
        t = -t;

    double absAlpha = fabs(alpha);

    if (t >= -absAlpha) {
        return amplitude * exp(-0.5 * t * t) + baseline_;
    } else {
        double a = pow(n / absAlpha, n) * exp(-0.5 * absAlpha * absAlpha);
        double b = n / absAlpha - absAlpha;
        return amplitude * a / pow(b - t, n) + baseline_;
    }
}
