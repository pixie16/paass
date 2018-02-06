/***************************************************************************
  *  Copyright S. V. Paulauskas 2014-2016                                       *
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

/** \file TraceFilter.cpp
 *  \brief A class to perform trapezoidal filtering
 *  \author S. V. Paulauskas
 *  \date 23 April 2014
 *
 *  Copyright S. V. Paulauskas 2014-2016
 *
 *  This code is based off of the IGOR macro energy.ipf
 *  written by H. Tan of XIA LLC and parts of the nscope
 *  program written at the NSC written by C.Prokop.
 *
 *  Published under the GNU GPL v. 3.0
 *
 */
#include <algorithm>
#include <iostream>

#include <cmath>

#include "TraceFilter.hpp"

using namespace std;

TraceFilter::TraceFilter(const unsigned int &adc,
                         const TrapFilterParameters &tFilt,
                         const TrapFilterParameters &eFilt,
                         const bool &analyzePileup/*=false*/,
                         const bool &verbose/*= false*/) {
    e_ = eFilt;
    t_ = tFilt;
    nsPerSample_ = adc;
    isVerbose_ = verbose;
    analyzePileup_ = analyzePileup;
}

void TraceFilter::CalcBaseline(void) {
    double l = t_.GetRisetime();
    int offset = trigs_[0] - l - 5;

    if(offset < 0)
        throw(EARLY_TRIG);
    
    for(unsigned int i = 0; i < (unsigned int) offset; i++)
        baseline_ += sig_->at(i);
    baseline_ /= offset;
    
    if(isVerbose_) 
        cout << "********** CalcBaseline **********" << endl
             << "  Range:" << " Low = 0  High = " << offset << endl
             << "  Value: " << baseline_ << endl << endl;
}

unsigned int TraceFilter::CalcFilters(const Trace *sig) {
    try{
        Reset();
        sig_ = sig;
        
        if(!isConverted_)
            ConvertToClockticks();
        CalcTriggerFilter();
        CalcBaseline();
        CalcEnergyFilterCoeffs();

        for(vector<unsigned int>::iterator it = trigs_.begin(); it!= trigs_.end(); it++) {
            CalcEnergyFilterLimits((*it));
            CalcEnergyFilter();
            
            if(!analyzePileup_)
                break;                
        }
    } catch(ErrTypes errcode) {
        switch(errcode) {
        case(NO_TRIG) :
            cerr << "We could not find a trigger in the trace." << endl;
            break;
        case(LATE_TRIG) :
            cerr << "The trigger came too late in the trace! I cannnot perform "
                 << "the sums over the necessary ranges, giving zero energy!!"
                 << endl;
            break;
        case(EARLY_TRIG) :
            cerr << "The trigger was too early. Could not calculate baseline "
                 << "or Energy Filter Limits." << endl;
            break;
        case(BAD_FILTER_COEFF):
            cerr << "One of the energy filter coefficients was nan." << endl;
            break;
        case(BAD_FILTER_LIMITS):
            cerr << "The Energy filter was too long for the trace." << endl;
            break;
        }
        return(errcode);
    }
    return(0);
}

void TraceFilter::CalcEnergyFilter(void) {
    double partA = 0, partB = 0, partC = 0;

    for(unsigned int i = limits_[0]; i < limits_[1]; i++)
        partA += sig_->at(i);
    for(unsigned int i = limits_[2]; i < limits_[3]; i++)
        partB += sig_->at(i);
    for(unsigned int i = limits_[4]; i < limits_[5]; i++)
        partC += sig_->at(i);
    esums_.push_back(partA);
    esums_.push_back(partB);
    esums_.push_back(partC);

    en_.push_back(coeffs_[0]*partA + coeffs_[1]*partB + coeffs_[2]*partC - baseline_);
    if(isVerbose_)
        cout << "********** CalcEnergyFilter **********" << endl
             << "Calculated Energy : " << en_.back() << endl << endl;
}

void TraceFilter::CalcEnergyFilterCoeffs(void) {
    double l = e_.GetRisetime();
    double beta = exp(-1.0/ e_.GetT());
    double cg = 1-beta;
    double ctmp = 1-pow(beta,l);

    if(std::isnan(beta) || std::isnan(cg) || std::isnan(ctmp))
        throw(BAD_FILTER_COEFF);
    
    coeffs_.push_back(-(cg/ctmp)*pow(beta,l));
    coeffs_.push_back(cg);
    coeffs_.push_back(cg/ctmp);

    if(isVerbose_)
        cout << "********** CalcEnergyFilterCoeffs **********" << endl
             << "  beta  : " << beta << endl
             << "  CRise : " << coeffs_[0] << endl
             << "  CGap  : " << coeffs_[1] << endl
             << "  CFall : " << coeffs_[2] << endl << endl;
}

void TraceFilter::CalcEnergyFilterLimits(const unsigned int &tpos) {
    limits_.clear();
    double l = e_.GetRisetime(), g = e_.GetFlattop();

    double p0 = tpos - l - 10;
    double p1 = p0 +l - 1;
    double p2 = p0 +l;
    double p3 = p0 + l + g - 1;
    double p4 = p0 + l + g;
    double p5 = p0 + 2*l + g - 1;
    double p7 = tpos + l + g;

    if(p0 < 0)
        throw(EARLY_TRIG);
    
    if(p7 > sig_->size())
        throw(LATE_TRIG);
        
    if(isVerbose_)
        cout << "********** CalcEnergyFilterLimits **********" << endl
             << "The limits for the Energy filter sums: " << endl
             << "  Rise Sum : Low = " << p0 << " High = " << p1 << endl
             << "  Gap Sum  : Low = " << p2 << " High = " << p3 << endl
             << "  Fall Sum : Low = " << p4 << " High = " << p5 << endl << endl;
    
    limits_.push_back(p0);      // beginning of  sum E0
    limits_.push_back(p1);      // end of sum E0
    limits_.push_back(p2);      // beginning of gap sum
    limits_.push_back(p3);      // end of gap sum
    limits_.push_back(p4);      // beginning of sum E1
    limits_.push_back(p5);      // end of sum E1
}

void TraceFilter::CalcTriggerFilter(void) {
    bool hasRecrossed = false;

    int l = t_.GetRisetime(), g = t_.GetFlattop();
    for(int i = 0; i < (int)sig_->size(); i++) {
        double sum1 = 0, sum2 = 0;
        if( (i-2*l-g+1) >= 0 ) {
            for(int a = i-2*l-g+1; a < i-l-g+1; a++)
                sum1 += sig_->at(a);
            for(int a = i-l+1; a < i+1; a++)
                sum2 += sig_->at(a);
            
            if((sum2 - sum1)/l >= t_.GetT()) {
                if(trigs_.size() == 0) 
                    trigs_.push_back(i);
                if(hasRecrossed) {
                    trigs_.push_back(i);
                    hasRecrossed = false;
                }
            }else {
                if(trigs_.size() != 0)
                    hasRecrossed = true;
            }
            
            trigFilter_.push_back((sum2 - sum1)/l);
        } else
            trigFilter_.push_back(0.0);
    }

    if(trigs_.size() == 0)
        throw(NO_TRIG);
    
    if(isVerbose_ ) {
        cout << "********** CalcTriggerFilter **********" << endl;
        cout << "The First Trigger Position : " << trigs_[0] << endl;
        if(trigs_.size() > 1)
            cout << "There was(were) " << trigs_.size() << " trigger(s) in the trace." << endl;
        cout << endl;
    }
}

void TraceFilter::ConvertToClockticks(void) {
    if(fmod(t_.GetRisetime(),nsPerSample_) != 0) {
        if(isVerbose_)
            cout << "TriggerRisetime is NOT an integer number of samples. Fixing."
                 << endl;
        t_.SetRisetime(ceil(t_.GetRisetime() / nsPerSample_));
    } else
        t_.SetRisetime(t_.GetRisetime() / nsPerSample_);

    if(t_.GetRisetime() !=0 && fmod(t_.GetFlattop(),nsPerSample_) != 0) {
        if(isVerbose_)
            cout << "TriggerGap is NOT an integer numnber of samples. Fixing."
                 << endl;
        t_.SetFlattop(ceil(t_.GetFlattop() / nsPerSample_));
    } else
        t_.SetFlattop(t_.GetFlattop() / nsPerSample_);

    if(fmod(e_.GetRisetime(),nsPerSample_) != 0) {
        if(isVerbose_)
            cout << "EnergyRisetime is NOT an integer number of samples.  Fixing."
                 << endl;
        e_.SetRisetime(ceil(e_.GetRisetime() / nsPerSample_));
    } else
        e_.SetRisetime(e_.GetRisetime() / nsPerSample_);

    if(e_.GetFlattop() != 0 && fmod(e_.GetFlattop(),nsPerSample_) != 0) {
        if(isVerbose_)
            cout << "EnergyGap is NOT an integer numnber of samples. Fixing."
                 << endl;
        e_.SetFlattop(ceil(e_.GetFlattop() / nsPerSample_));
    } else
        e_.SetFlattop(e_.GetFlattop() / nsPerSample_);

    e_.SetT(e_.GetT() / nsPerSample_);

    if(isVerbose_) {
        cout << "********** ConvertToClockticks **********" << endl;
        cout << "Trigger Filter:" << endl
             << "  RiseTime (ns):   " << t_.GetRisetime()*nsPerSample_ <<  endl
             << "  Flattop(ns):   " << t_.GetFlattop()*nsPerSample_ <<  endl
             << "  Thresh (ADC units) : " << t_.GetT() << endl
             << "Energy Filter: " << endl
             << "  Energy rise (ns): " << e_.GetRisetime()*nsPerSample_ <<  endl
             << "  Energy flat (ns): " << e_.GetFlattop()*nsPerSample_ <<  endl
             << "  Tau(ns) :         " << e_.GetT()*nsPerSample_ << endl << endl;
    }
    isConverted_ = true;
}

void TraceFilter::Reset(void) {
    en_.clear();
    baseline_ = 0;
    trigFilter_.clear();
    trigs_.clear();
    limits_.clear();
}
