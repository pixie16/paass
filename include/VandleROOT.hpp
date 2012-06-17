/** \file VandleROOT.hpp
 *
 * Root functionality for Vandle Processor
 */

#ifndef __VANDLEROOT_HPP_
#define __VANDLEROOT_HPP_

class TTree;

#include "VandleProcessor.hpp"

class VandleROOT : public VandleProcessor
{
 public: 
    bool AddBranch(TTree *tree);
    void FillBranch(void);

 private:
    DataRoot smallRight;
    DataRoot smallLeft;
    DataRoot bigRight;
    DataRoot bigLeft;

    bool isSmall;
    bool isBig;

    virtual void FillRoot(const TimingDataMap &endMap, const std::string &barType);
}; // class VandleROOT
#endif // __VANDLEROOT_HPP_
