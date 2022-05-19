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

   private:
    std::string PixieRevision;  //! pixie revision
    double EandQDC_down_scaling_;
    std::pair<int, int> dammSiPm_pixelShifts;
};

#endif