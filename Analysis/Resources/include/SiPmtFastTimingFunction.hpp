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
#ifndef __SIPMTFASTTIMINGFUNCITON__HPP__
#define __SIPMTFASTTIMINGFUNCITON__HPP__

class SiPmtFastTimingFunction {
public:
    SiPmtFastTimingFunction() {};

    ~SiPmtFastTimingFunction() {};

    double operator()(double *x, double *p);

    void SetBaseline(const double &a) { baseline_ = a; };
private:
    double baseline_;
};

#endif
