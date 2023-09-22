/** @file MtasImplantSipmProcessor.hpp
 * @brief  Basic Processor for the SiPm based Implant detector used with MTAS at FRIB
 * @authors T.T. King
 * @date 05/16/2022
 */
#ifndef PAASS_MtasImplantSipmProcessor_H
#define PAASS_MtasImplantSipmProcessor_H

#include "EventProcessor.hpp"
#include "Globals.hpp"
// #include "PaassRootStruct.hpp"
#include <utility>
#include <vector>

#include "RawEvent.hpp"

class MtasImplantSipmProcessor : public EventProcessor {
   public:
    /**Constructor */
    MtasImplantSipmProcessor();

       /**Constructor */
    MtasImplantSipmProcessor(double yso_scale_, double yso_offset_, double yso_thresh_, double oqdc_yso_thresh_, double dyh_thresh_, double dyh_qdc_thresh_, double dyl_thresh_, double dyl_qdc_thresh_, double dyh_upperThresh_);

    /** Deconstructor */
    ~MtasImplantSipmProcessor() = default;

    /** Preprocess the event
		 * \param [in] event : the event to preprocess
		 * \return true if successful
		 */
    bool PreProcess(RawEvent& event);

    /** Process the event
		 * \param [in] event : the event to process
		 * \return true if successful
		 */
    bool Process(RawEvent& event);

    /** Declares the plots for the class */
    void DeclarePlots(void);

    double CalOnboardQDC(int bkg, int waveform, const std::vector<unsigned> oqdcs, double bkgTimeScaling = 1, double waveformTimeScaling = 1);

    std::pair<int, int> ComputeSiPmPixelLoc(int xmlLocation_);

    void FillRootStruct(ChanEvent* evt, double& onboardqdc, const std::pair<int,int> &positions = {0,0});

    std::pair<double,double> CalculatePosition(std::vector<std::vector<double>>& data);

   private:
    std::string PixieRevision;  //! pixie revision
    processor_struct::MTASIMPLANT mtasImplStruct;
    double EandQDC_down_scaling_;
    std::pair<int, int> dammSiPm_pixelShifts;
    double yso_thresh,yso_scale,yso_offset,oqdc_yso_thresh;
    double dyh_thresh;
    double dyh_qdc_thresh;
    double dyl_thresh;
    double dyl_qdc_thresh;
    double dyh_upperThresh;
};

#endif