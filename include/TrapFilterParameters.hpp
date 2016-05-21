/***************************************************************************
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
/** \file TraceFilter.hpp
 *  \brief A class to hold parameters for a trapezoidal filter
 *  \author S. V. Paulauskas
 *  \date May 21, 2016
 *
 */
#ifndef __TRAPFILTERPARAMETERS_HPP__
#define __TRAPFILTERPARAMETERS_HPP__
//!Simple class that holds trapezoidal filter parameters. 
class TrapFilterParameters {
public:
    //!Default Constructor
    TrapFilterParameters(){};
    //!Constructor accepting risetime, flattop, and tau/threshold parameters
    TrapFilterParameters(const double &l, const double &g, const double &t){
        l_ = l;
        g_ = g;
        t_ = t;
    };
    //!Default Destructor
    ~TrapFilterParameters(){};

    //! Returns the value of the flattop
    double GetFlattop(void){return(g_);}
    //! Returns the value of the risetime
    double GetRisetime(void){return(l_);}
    //! Returns the value of tau/threhsold
    double GetT(void){return(t_);}
    //! Returns the size of the filter
    double GetSize(void) {return(2*l_+g_);}

    //! Sets the value of the flattop
    void SetFlattop(const double &a){g_ = a;}
    //! Sets the value of the risetime
    void SetRisetime(const double &a){l_ = a;}
    //! Sets the value of tau/threhsold
    void SetT(const double &a){t_ = a;}
private:
    double g_;  //!< the flattop of the filer
    double l_;   //!< the risetime for the filter
    double t_;  //!< the tau/threhsold for the filter
};
#endif //__TRAPFILTERPARAMETERS_HPP__
