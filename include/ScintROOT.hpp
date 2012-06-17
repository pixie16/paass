/** \file ScintROOT.hpp
 *
 * Root Functionality for ScintProcessor
 */

#ifndef __SCINTROOT_HPP_
#define __SCINTROOT_HPP_

class TTree;

#include "ScintProcessor.hpp"

class ScintROOT : public ScintProcessor
{
 public:
    bool AddBranch (TTree *tree);
    void FillBranch(void);

 private:
    DataRoot beta;
    DataRoot liquid;

    virtual void FillRoot(const TimingDataMap &endMap);
}; // class ScintROOT
#endif //__SCINTROOT_HPP_
