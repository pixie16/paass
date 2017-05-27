///Copyright S. V. Paulauskas 2017
///
///This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation, version 3.0 License.
///
///This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
///MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
///
///You should have received a copy of the GNU General Public License along with this program.  If not, see
/// <http://www.gnu.org/licenses/>.

/*! \file CsiFunction.hpp
 *  \brief A class to handle the processing of traces
 *  \author S. V. Paulauskas
 *  \date April 9, 2015
 */
#ifndef __CSIFUNCITON__HPP__
#define __CSIFUNCITON__HPP__

class CsiFunction {
public:
    CsiFunction() {};

    ~CsiFunction() {};

    double operator()(double *x, double *p);
};

#endif
