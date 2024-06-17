///@file PidProcessor.cpp
///@A dedicated processor for constructing PID information
///@author A. Keeler, R. Yokoyama
///@date July 29, 2019
// Updated for E19044 experiment.

#include "PidProcessor.hpp"

#include <limits.h>
#include <signal.h>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>

#include "DammPlotIds.hpp"
#include "DetectorDriver.hpp"
#include "Globals.hpp"
#include "HelperFunctions.hpp"
#include "Messenger.hpp"

using namespace std;
using namespace dammIds::pid;


namespace dammIds {
	namespace pid {

		const int D_IMAGEL_MULT = 0;    //! Multiplicity of the image plastic L
		const int D_DISPL_MULT = 1;     //! Multiplicity of the dispersive PPAC L

		const int DD_PINS_DE = 4;    //! Energy deposits in the PINs
		const int DD_PINS_MULT = 5;  //! Multiplicities of the Pins
		const int DD_PIN0_1 = 6;     //! Pin0 vs Pin1 dE
		const int D_DISPLR = 7;                //! Time difference between dispersive left and right
		const int D_DISPUD = 8;                //! Time difference between dispersive up and down
		const int DD_DISP_PLANE = 9;                //! 2-D image of dipersive up-down vs left-right

		const int DD_TOF2_PIN1 = 14;            //! TAC0 vs Pin1 dE
		const int DD_TOF2_PIN1_GATED_FIT = 15;  //! GATED TAC0 vs Pin1 dE
		const int DD_TOF2_PIN1_GATED_YSO = 16;  //! GATED TAC0 vs Pin1 dE
		const int DD_TOF2_PIN1_GATED_RIT = 17;  //! GATED TAC0 vs Pin1 dE

		const int DD_TOF3_PIN1 = 24;            //! TAC1 vs Pin1 dE
		const int DD_TOF3_PIN1_GATED_FIT = 25;  //! GATED TAC1 vs Pin1 dE
		const int DD_TOF3_PIN1_GATED_YSO = 26;  //! GATED TAC1 vs Pin1 dE
		const int DD_TOF3_PIN1_GATED_RIT = 27;  //! GATED TAC1 vs Pin1 dE

		const int DD_TOF0_PIN0 = 30;             //! ToF vs Pin0 dE
		const int DD_TOF0_PIN0_GATED_FIT = 31;  //! GATED ToF vs Pin0 dE
		const int DD_TOF0_PIN0_GATED_YSO = 32;   //! GATED ToF vs Pin0 dE
		const int DD_TOF0_PIN0_GATED_RIT = 33;  //! GATED ToF vs Pin0 dE

		const int DD_TOF1_PIN0 = 40;            //! ToF vs Pin0 dE
		const int DD_TOF1_PIN0_GATED_FIT = 41;  //! GATED ToF vs Pin0 dE
		const int DD_TOF1_PIN0_GATED_YSO = 42;  //! GATED ToF vs Pin0 dE
		const int DD_TOF1_PIN0_GATED_RIT = 43;  //! GATED ToF vs Pin0 dE

		const int DD_TOF2_PIN0 = 50;            //! ToF vs Pin0 dE
		const int DD_TOF2_PIN0_GATED_FIT = 51;  //! GATED ToF vs Pin0 dE
		const int DD_TOF2_PIN0_GATED_YSO = 52;  //! GATED ToF vs Pin0 dE
		const int DD_TOF2_PIN0_GATED_RIT = 53;  //! GATED ToF vs Pin0 dE

		const int DD_TOF3_PIN0 = 60;            //! ToF vs Pin0 dE
		const int DD_TOF3_PIN0_GATED_FIT = 61;  //! GATED ToF vs Pin0 dE
		const int DD_TOF3_PIN0_GATED_YSO = 62;  //! GATED ToF vs Pin0 dE
		const int DD_TOF3_PIN0_GATED_RIT = 63;  //! GATED ToF vs Pin0 dE

		const int D_RANGE = 70; //implantation range gated by PID (right now hard coded)

		const int DD_PIN2_3 = 71;     //! Pin2 vs Pin3 dE
		const int DD_PIN0_2 = 72;     //! Pin0 vs Pin2 dE
		const int DD_PIN1_3 = 73;     //! Pin1 vs Pin3 dE
		const int DD_TOF4_PIN2 = 74;            //! ToF4 vs Pin2 dE
		const int DD_TOF5_PIN2 = 75;            //! ToF5 vs Pin2 dE
		const int DD_TOF4_PIN3 = 76;            //! ToF4 vs Pin3 dE
		const int DD_TOF5_PIN3 = 77;            //! ToF5 vs Pin3 dE

		const int DD_CROSS_SCINT_POS = 80;      //! position information from scint in the 2nd cross (From Ben Kreider).

	}  // namespace pid
}  // namespace dammIds

void PidProcessor::DeclarePlots(void) {
	DeclareHistogram1D(D_IMAGEL_MULT, S5, "Multiplicity of image L");
	DeclareHistogram1D(D_DISPL_MULT, S5, "Multiplicity of dispersive PPAC L");
	DeclareHistogram2D(DD_PINS_DE, S2, SD, "Pin dE");
	DeclareHistogram2D(DD_PINS_MULT, S2, S5, "Pins Multiplicity");
	DeclareHistogram2D(DD_PIN0_1, SC, SC, "Pin0 vs Pin1 dE ");
	DeclareHistogram1D(D_DISPLR, SD, "Dispersive L - R");
	DeclareHistogram1D(D_DISPUD, SD, "Dispersive U - D");
	DeclareHistogram2D(DD_DISP_PLANE, SB, SB, "Disp U_D vs Disp L_R");

	DeclareHistogram2D(DD_TOF2_PIN1, SB, SD, "TOF2 vs Pin1 dE ");
	DeclareHistogram2D(DD_TOF2_PIN1_GATED_FIT, SB, SD, "FIT: TOF2 vs Pin1 dE ");
	DeclareHistogram2D(DD_TOF2_PIN1_GATED_YSO, SB, SD, "YSO: TOF2 vs Pin1 dE ");
	DeclareHistogram2D(DD_TOF2_PIN1_GATED_RIT, SB, SD, "RIT: TOF2 vs Pin1 dE ");

	DeclareHistogram2D(DD_TOF3_PIN1, SB, SD, "TOF3 vs Pin1 dE ");
	DeclareHistogram2D(DD_TOF3_PIN1_GATED_FIT, SB, SD, "FIT: TOF3 vs Pin1 dE ");
	DeclareHistogram2D(DD_TOF3_PIN1_GATED_YSO, SB, SD, "YSO: TOF3 vs Pin1 dE ");
	DeclareHistogram2D(DD_TOF3_PIN1_GATED_RIT, SB, SD, "RIT: TOF3 vs Pin1 dE ");

	DeclareHistogram2D(DD_TOF0_PIN0, SB, SD, "Tof0 vs Pin0 dE ");
	DeclareHistogram2D(DD_TOF0_PIN0_GATED_FIT, SB, SD, "FIT: Tof0 vs Pin0 dE ");
	DeclareHistogram2D(DD_TOF0_PIN0_GATED_YSO, SB, SD, "YSO: Tof0 vs Pin0 dE ");
	DeclareHistogram2D(DD_TOF0_PIN0_GATED_RIT, SB, SD, "RIT: Tof0 vs Pin0 dE ");

	DeclareHistogram2D(DD_TOF1_PIN0, SB, SD, "Tof1 vs Pin0 dE ");
	DeclareHistogram2D(DD_TOF1_PIN0_GATED_FIT, SB, SD, "FIT: Tof1 vs Pin0 dE ");
	DeclareHistogram2D(DD_TOF1_PIN0_GATED_YSO, SB, SD, "YSO: Tof1 vs Pin0 dE ");
	DeclareHistogram2D(DD_TOF1_PIN0_GATED_RIT, SB, SD, "RIT: Tof1 vs Pin0 dE ");

	DeclareHistogram2D(DD_TOF2_PIN0, SB, SD, "Tof2 vs Pin0 dE ");
	DeclareHistogram2D(DD_TOF2_PIN0_GATED_FIT, SB, SD, "FIT: Tof2 vs Pin0 dE ");
	DeclareHistogram2D(DD_TOF2_PIN0_GATED_YSO, SB, SD, "YSO: Tof2 vs Pin0 dE ");
	DeclareHistogram2D(DD_TOF2_PIN0_GATED_RIT, SB, SD, "RIT: Tof2 vs Pin0 dE ");

	DeclareHistogram2D(DD_TOF3_PIN0, SB, SD, "Tof3 vs Pin0 dE ");
	DeclareHistogram2D(DD_TOF3_PIN0_GATED_FIT, SB, SD, "FIT: Tof3 vs Pin0 dE ");
	DeclareHistogram2D(DD_TOF3_PIN0_GATED_YSO, SB, SD, "YSO: Tof3 vs Pin0 dE ");
	DeclareHistogram2D(DD_TOF3_PIN0_GATED_RIT, SB, SD, "RIT: Tof3 vs Pin0 dE ");

	DeclareHistogram1D(D_RANGE, S3, "Range distribution with PID gate");

	DeclareHistogram2D(DD_PIN2_3, SC, SC, "Pin2 vs Pin3 dE ");
	DeclareHistogram2D(DD_PIN0_2, SC, SC, "Pin0 vs Pin2 dE ");
	DeclareHistogram2D(DD_PIN1_3, SC, SC, "Pin1 vs Pin3 dE ");
	DeclareHistogram2D(DD_TOF4_PIN2, SB, SD, "Tof4 vs Pin2 dE ");
	DeclareHistogram2D(DD_TOF5_PIN2, SB, SD, "Tof5 vs Pin2 dE ");
	DeclareHistogram2D(DD_TOF4_PIN3, SB, SD, "Tof4 vs Pin3 dE ");
	DeclareHistogram2D(DD_TOF5_PIN3, SB, SD, "Tof5 vs Pin3 dE ");

	DeclareHistogram2D(DD_CROSS_SCINT_POS, SB, SB, "Pos. of 2nd Cross Scint ");
}  // Declare plots

PidProcessor::PidProcessor(const double &YSO_Implant_thresh, const double &FIT_thresh, const double &RIT_thresh, const bool &tofflip) : EventProcessor(OFFSET, RANGE, "PidProcessor") {
	associatedTypes.insert("pid");
	associatedTypes.insert("pin");

	yso_threshold_ = YSO_Implant_thresh;
	fit_threshold_ = FIT_thresh;
	rit_threshold_ = RIT_thresh;
	tofflip_ = tofflip;

	yso_energy_prev = 0;
	fit_energy_prev = 0;
	fit_energy_prev = 0;
}

bool PidProcessor::PreProcess(RawEvent &event) {
	if (!EventProcessor::PreProcess(event))
		return false;

	// A flag for ROOT output
	const bool root_output = DetectorDriver::get()->GetSysRootOutput();

	if (root_output) {
		// Initialization of the pid_struct
		pid_struct = processor_struct::PID_DEFAULT_STRUCT;
	}

	const vector<ChanEvent *> &cross_scint_b1_vec = event.GetSummary("pid:cross_scint_b1", true)->GetList();
	const vector<ChanEvent *> &cross_scint_t1_vec = event.GetSummary("pid:cross_scint_t1", true)->GetList();
	const vector<ChanEvent *> &cross_scint_v1_vec = event.GetSummary("pid:cross_scint_v1", true)->GetList(); // These correspond to the fp2 cross scint
	const vector<ChanEvent *> &cross_scint_v2_vec = event.GetSummary("pid:cross_scint_v2", true)->GetList();// These correspond to the fp2 cross scint
	const vector<ChanEvent *> &cross_scint_v3_vec = event.GetSummary("pid:cross_scint_v3", true)->GetList();// These correspond to the fp2 cross scint
	const vector<ChanEvent *> &cross_scint_v4_vec = event.GetSummary("pid:cross_scint_v4", true)->GetList();// These correspond to the fp2 cross scint
	const vector<ChanEvent *> &cross_pin0_vec = event.GetSummary("pid:cross_pin0", true)->GetList();
	const vector<ChanEvent *> &cross_pin1_vec = event.GetSummary("pid:cross_pin1", true)->GetList();
	const vector<ChanEvent *> &cross_pin2_vec = event.GetSummary("pid:cross_pin2", true)->GetList();
	const vector<ChanEvent *> &cross_pin3_vec = event.GetSummary("pid:cross_pin3", true)->GetList();

	const vector<ChanEvent *> &db3_ppac_up_vec = event.GetSummary("pid:db3_ppac_logic_up", true)->GetList();
	const vector<ChanEvent *> &db3_ppac_down_vec = event.GetSummary("pid:db3_ppac_logic_down", true)->GetList();
	const vector<ChanEvent *> &db5_ppac_L_vec = event.GetSummary("pid:db5_ppac_L_logic", true)->GetList();
	const vector<ChanEvent *> &db5_ppac_R_vec = event.GetSummary("pid:db5_ppac_R_logic", true)->GetList();

	const vector<ChanEvent *> &dispL_vec = event.GetSummary("pid:disp_L_logic", true)->GetList();
	const vector<ChanEvent *> &dispR_vec = event.GetSummary("pid:disp_R_logic", true)->GetList();
	const vector<ChanEvent *> &dispU_vec = event.GetSummary("pid:disp_U_logic", true)->GetList();
	const vector<ChanEvent *> &dispD_vec = event.GetSummary("pid:disp_D_logic", true)->GetList();

	const vector<ChanEvent *> &imageL_vec = event.GetSummary("pid:image_L_logic", true)->GetList();
	const vector<ChanEvent *> &imageR_vec = event.GetSummary("pid:image_R_logic", true)->GetList();
	const vector<ChanEvent *> &pinLogic_vec = event.GetSummary("pid:cross_pin0_logic", true)->GetList();
	const vector<ChanEvent *> &b2Logic_vec = event.GetSummary("pid:cross_scint_b2_logic", true)->GetList();

	// Function that compares energies in two ChanEvent objects
	auto compare_energy = [](ChanEvent *x1, ChanEvent *x2) { return x1->GetCalibratedEnergy() < x2->GetCalibratedEnergy(); };

	// Function that returns time in ns
	// Note: this function returns time WITHOUT Pixie onboard CFD

	//* Tof between rfq and beamline FocalPlane */

	double tof0 = 0, tof1 = 0, tof2 = 0, tof3 = 0, pin0_energy = 0, pin1_energy = 0,tac0_energy = 0, tac1_energy = 0, tac2_energy=0, tac3_energy=0;
	double tof4 = 0, tof5 = 0, pin2_energy = 0, pin3_energy = 0;
	double cross_scint_b1_energy = 0, cross_scint_t1_energy = 0;
	double cross_scint_v1_energy = 0, cross_scint_v2_energy = 0, cross_scint_v3_energy = 0, cross_scint_v4_energy = 0;
	double cross_scint_v1_qdc = 0, cross_scint_v2_qdc = 0, cross_scint_v3_qdc = 0, cross_scint_v4_qdc = 0;
	double tof0_flip = 0, tof1_flip = 0, tof2_flip = 0, tof3_flip = 0, tof4_flip = 0, tof5_flip = 0;
	double disp_LR = 0;
	double disp_UD = 0;
	double db5_ppac_LR = 0;

	if (root_output){

		if(!imageL_vec.empty() ){
			auto imageL = std::max_element(imageL_vec.begin(), imageL_vec.end(), compare_energy);
			if((*imageL))
				pid_struct.image_scint_L_logic_time = (*imageL)->GetTimeInNs() ;
		}
		if(!imageR_vec.empty() ){
			auto imageR = std::max_element(imageR_vec.begin(), imageR_vec.end(), compare_energy);
			if((*imageR))
				pid_struct.image_scint_R_logic_time = (*imageR)->GetTimeInNs() ;
		}
		if(!pinLogic_vec.empty() ){
			auto pinLogic = std::max_element(pinLogic_vec.begin(), pinLogic_vec.end(), compare_energy);
			if((*pinLogic))
				pid_struct.cross_pin_0_logic_time = (*pinLogic)->GetTimeInNs();
		}
		if(!b2Logic_vec.empty() ){
			auto b2Logic = std::max_element(b2Logic_vec.begin(), b2Logic_vec.end(), compare_energy);
			if((*b2Logic))
				pid_struct.cross_scint_b2_logic_time = (*b2Logic)->GetTimeInNs() ;
		}
		//std::cout<<"come to line 251"<<std::endl;
		if (!cross_pin0_vec.empty() ) {
			// Get elements with the largest energy in this event 
			auto pin = std::max_element(cross_pin0_vec.begin(), cross_pin0_vec.end(), compare_energy);
			// Check for nullptr
			if (!(*pin)->GetTrace().empty()) {
				pid_struct.cross_pin_0_tracemax = (*pin)->GetTrace().GetMaxInfo().second;
				pid_struct.cross_pin_0_traceqdc = (*pin)->GetTrace().GetQdc();
			}
		}
		//std::cout<<"come to line 260"<<std::endl;
		if (!cross_pin1_vec.empty() ) {
			// Get elements with the largest energy in this event 
			auto pin = std::max_element(cross_pin1_vec.begin(), cross_pin1_vec.end(), compare_energy);
			// Check for nullptr
			if (!(*pin)->GetTrace().empty()) {
				pid_struct.cross_pin_1_tracemax = (*pin)->GetTrace().GetMaxInfo().second;
				pid_struct.cross_pin_1_traceqdc = (*pin)->GetTrace().GetQdc();
			}
		}
		if (!cross_pin2_vec.empty() ) {
			// Get elements with the largest energy in this event 
			auto pin = std::max_element(cross_pin2_vec.begin(), cross_pin2_vec.end(), compare_energy);
			// Check for nullptr
			if (!(*pin)->GetTrace().empty()) {
				pid_struct.cross_pin_2_tracemax = (*pin)->GetTrace().GetMaxInfo().second;
				pid_struct.cross_pin_2_traceqdc = (*pin)->GetTrace().GetQdc();
			}
		}
		if (!cross_pin3_vec.empty() ) {
			// Get elements with the largest energy in this event 
			auto pin = std::max_element(cross_pin3_vec.begin(), cross_pin3_vec.end(), compare_energy);
			// Check for nullptr
			if (!(*pin)->GetTrace().empty()) {
				pid_struct.cross_pin_3_tracemax = (*pin)->GetTrace().GetMaxInfo().second;
				pid_struct.cross_pin_3_traceqdc = (*pin)->GetTrace().GetQdc();
			}
		}
	}
	//std::cout<<"come to line 269"<<std::endl;
	if (!db3_ppac_up_vec.empty()) {
		// Get elements with the largest energy in this event for dispL
		auto ppac_up = std::max_element(db3_ppac_up_vec.begin(), db3_ppac_up_vec.end(), compare_energy);
		//double internalTAC_Convert_Tick_adc = Globals::get()->GetAdcClockInSeconds((*ppac_up)->GetChanID().GetModFreq()) * 1e9;

		// ROOT outputs
		if (root_output) {
			pid_struct.db3_ppac_logic_up_time = (*ppac_up)->GetHighResTimeInNs();
			//pid_struct.db3_ppac_logic_up_time = -100;
		}
	}

	if (!db3_ppac_down_vec.empty()) {
		// Get elements with the largest energy in this event for dispL
		auto ppac_down = std::max_element(db3_ppac_down_vec.begin(), db3_ppac_down_vec.end(), compare_energy);

		// ROOT outputs
		if (root_output) {
			pid_struct.db3_ppac_logic_down_time = (*ppac_down)->GetHighResTimeInNs();
		}
	}

	//! TOF 0 group (db3 ppac downstream to cross plastic)
	if (!db3_ppac_down_vec.empty() && !cross_scint_t1_vec.empty()) {
		// Get elements with the largest energy in this event for image L
		auto ppac_down = std::max_element(db3_ppac_down_vec.begin(), db3_ppac_down_vec.end(), compare_energy);

		// Get elements with the largest energy in this event for pinLogic
		auto cross_scint = std::max_element(cross_scint_t1_vec.begin(), cross_scint_t1_vec.end(), compare_energy);
		// Check for nullptr
		if ((*ppac_down) && (*cross_scint)) {
			// Calculate tof
			tof0 = ((*cross_scint)->GetHighResTimeInNs() - (*ppac_down)->GetHighResTimeInNs());
			tof0_flip = ((*ppac_down)->GetHighResTimeInNs() - (*cross_scint)->GetHighResTimeInNs());

			// ROOT outputs
			if (root_output) {
				pid_struct.tof0 = tof0;
			}
		}
	}
	//
	//! TOF 1 group (image L to cross pin0 highrestime)
	if (!imageL_vec.empty() && !cross_pin0_vec.empty()) {
		// Get elements with the largest energy in this event for image L
		auto imageL = std::max_element(imageL_vec.begin(), imageL_vec.end(), compare_energy);

		// Get elements with the largest energy in this event for pinLogic
		auto pin = std::max_element(cross_pin0_vec.begin(), cross_pin0_vec.end(), compare_energy);

		// Check for nullptr
		if ((*imageL) && (*pin)) {
			// Calculate tof
			tof1 = (*pin)->GetHighResTimeInNs() - ((*imageL)->GetTimeInNs());
			tof1_flip = ((*imageL)->GetTimeInNs()) - (*pin)->GetHighResTimeInNs();

			// ROOT outputs
			if (root_output) {
				pid_struct.tof1 = tof1;
			}
		}
	}
	//
	//! TOF 2 group (image L to cross scint onboard cfd)
	if (!imageL_vec.empty() && !b2Logic_vec.empty()) {
		// Get elements with the largest energy in this event for image L
		auto imageL = std::max_element(imageL_vec.begin(), imageL_vec.end(), compare_energy);

		// Get elements with the largest energy in this event for pinLogic
		auto b2Logic = std::max_element(b2Logic_vec.begin(), b2Logic_vec.end(), compare_energy);
		// Check for nullptr
		if ((*imageL) && (*b2Logic)) {
			// Calculate tof
			tof2 = ((*b2Logic)->GetTimeInNs()) - ((*imageL)->GetTimeInNs());
			tof2_flip = ((*imageL)->GetTimeInNs()) - ((*b2Logic)->GetTimeInNs());

			// ROOT outputs
			if (root_output) {
				pid_struct.tof2 = tof2;
			}
		}
	}
	//
	//! TOF 3 group (ppac db4 to cross scint highrestime)
	if (!imageL_vec.empty() && !cross_scint_b1_vec.empty()) {
		// Get elements with the largest energy in this event for image L
		auto imageL = std::max_element(imageL_vec.begin(), imageL_vec.end(), compare_energy);

		// Get elements with the largest energy in this event for pinLogic
		auto cross_scint_b1 = std::max_element(cross_scint_b1_vec.begin(), cross_scint_b1_vec.end(), compare_energy);
		// Check for nullptr
		if ((*imageL) && (*cross_scint_b1)) {
			// Calculate tof
			tof3 = (*cross_scint_b1)->GetHighResTimeInNs() - ((*imageL)->GetTimeInNs());
			tof3_flip = ((*imageL)->GetTimeInNs()) - (*cross_scint_b1)->GetHighResTimeInNs();

			// ROOT outputs
			if (root_output) {
				pid_struct.tof3 = tof3;
			}
		}
	}

	//! dispersive L-R group (diespersive L -> R)
	if (!dispL_vec.empty() && !dispR_vec.empty()) {
		// Get elements with the largest energy in this event for dispL
		auto dispL = std::max_element(dispL_vec.begin(), dispL_vec.end(), compare_energy);

		// Get elements with the largest energy in this event for dispR
		auto dispR = std::max_element(dispR_vec.begin(), dispR_vec.end(), compare_energy);
		// Check for nullptr
		if ((*dispL) && (*dispR)) {
			// Calculate LR
			disp_LR = ((*dispL)->GetTimeInNs()) - ((*dispR)->GetTimeInNs());

			// ROOT outputs
			if (root_output) {
				pid_struct.disp_L_logic_time = (*dispL)->GetTimeInNs();
				pid_struct.disp_R_logic_time = (*dispR)->GetTimeInNs();
				pid_struct.disp_LR = disp_LR;
			}
		}
	}

	//////////////// TOF 4,5 are for Cross 2

	//! TOF 4 group (image L to cross pin2 highrestime)
	if (!imageL_vec.empty() && !cross_pin2_vec.empty()) {
		// Get elements with the largest energy in this event for image L
		auto imageL = std::max_element(imageL_vec.begin(), imageL_vec.end(), compare_energy);

		// Get elements with the largest energy in this event for pinLogic
		auto pin = std::max_element(cross_pin2_vec.begin(), cross_pin2_vec.end(), compare_energy);

		// Check for nullptr
		if ((*imageL) && (*pin)) {
			// Calculate tof
			tof4 = (*pin)->GetHighResTimeInNs() - ((*imageL)->GetTimeInNs());
			tof4_flip = ((*imageL)->GetTimeInNs()) - (*pin)->GetHighResTimeInNs();

			// ROOT outputs
			if (root_output) {
				pid_struct.tof4 = tof4;
			}
		}
	}

	//
	//! TOF 5 group (db3 ppac downstream to cross plastic MTAS crate FP1 logic)
	if (!db3_ppac_down_vec.empty() && !b2Logic_vec.empty()) {
		// Get elements with the largest energy in this event for image L
		auto ppac_down = std::max_element(db3_ppac_down_vec.begin(), db3_ppac_down_vec.end(), compare_energy);
		// Get elements with the largest energy in this event for pinLogic
		auto b2Logic = std::max_element(b2Logic_vec.begin(), b2Logic_vec.end(), compare_energy);
		// Check for nullptr
		if ((*ppac_down) && (*b2Logic)) {
			// Calculate tof
			tof5 = (*b2Logic)->GetTimeInNs() - (*ppac_down)->GetHighResTimeInNs();
			tof5_flip = (*ppac_down)->GetHighResTimeInNs() - (*b2Logic)->GetTimeInNs();

			// ROOT outputs
			if (root_output) {
				pid_struct.tof5 = tof5;
			}
		}
	}
	//! dispersive L-R group (diespersive L -> R)
	if (!db5_ppac_L_vec.empty() && !db5_ppac_R_vec.empty()){
		auto ppac_L = std::max_element(db5_ppac_L_vec.begin(), db5_ppac_L_vec.end(), compare_energy);
		auto ppac_R = std::max_element(db5_ppac_R_vec.begin(), db5_ppac_R_vec.end(), compare_energy);
		if((*ppac_L) && (*ppac_R)){
			db5_ppac_LR = ((*ppac_L)->GetHighResTimeInNs() - (*ppac_R)->GetHighResTimeInNs());
			if(root_output){
				pid_struct.db5_ppac_L_logic_time = (*ppac_L)->GetHighResTimeInNs();
				pid_struct.db5_ppac_R_logic_time = (*ppac_R)->GetHighResTimeInNs();
				pid_struct.db5_ppac_LR = db5_ppac_LR;
			}
		}
	}
	if (!dispL_vec.empty() && !dispR_vec.empty()) {
		// Get elements with the largest energy in this event for dispL
		auto dispL = std::max_element(dispL_vec.begin(), dispL_vec.end(), compare_energy);

		// Get elements with the largest energy in this event for dispR
		auto dispR = std::max_element(dispR_vec.begin(), dispR_vec.end(), compare_energy);
		// Check for nullptr
		if ((*dispL) && (*dispR)) {
			// Calculate LR
			disp_LR = ((*dispL)->GetTimeInNs()) - ((*dispR)->GetTimeInNs());

			// ROOT outputs
			if (root_output) {
				pid_struct.disp_L_logic_time = (*dispL)->GetTimeInNs();
				pid_struct.disp_R_logic_time = (*dispR)->GetTimeInNs();
				pid_struct.disp_LR = disp_LR;
			}
		}
	}
	//! dispersive U-D group (diespersive U -> D)
	if (!dispU_vec.empty() && !dispD_vec.empty()) {
		// Get elements with the largest energy in this event for dispU
		auto dispU = std::max_element(dispU_vec.begin(), dispU_vec.end(), compare_energy);

		// Get elements with the largest energy in this event for dispD
		auto dispD = std::max_element(dispD_vec.begin(), dispD_vec.end(), compare_energy);
		// Check for nullptr
		if ((*dispU) && (*dispD)) {
			// Calculate UD
			disp_UD = ((*dispU)->GetTimeInNs()) - ((*dispD)->GetTimeInNs());

			// ROOT outputs
			if (root_output) {
				pid_struct.disp_U_logic_time = (*dispU)->GetTimeInNs();
				pid_struct.disp_D_logic_time = (*dispD)->GetTimeInNs();
				pid_struct.disp_UD = disp_UD;
			}
		}
	}

	//** Pins */

	if (!cross_pin0_vec.empty()) {
		auto pin = max_element(cross_pin0_vec.begin(), cross_pin0_vec.end(), compare_energy);
		pin0_energy = (*pin)->GetCalibratedEnergy();
		plot(DD_PINS_DE, 0, pin0_energy);
		if (root_output) {
			pid_struct.cross_pin_0_energy = pin0_energy;
			pid_struct.cross_pin_0_time = (*pin)->GetHighResTimeInNs();
		}
	}
	if (!cross_pin1_vec.empty()) {
		auto pin = max_element(cross_pin1_vec.begin(), cross_pin1_vec.end(), compare_energy);
		pin1_energy = (*pin)->GetCalibratedEnergy();
		plot(DD_PINS_DE, 1, pin1_energy);
		if (root_output) {
			pid_struct.cross_pin_1_energy = pin1_energy;
			pid_struct.cross_pin_1_time = (*pin)->GetHighResTimeInNs();
		}
	}
	if (!cross_pin2_vec.empty()) {
		auto pin = max_element(cross_pin2_vec.begin(), cross_pin2_vec.end(), compare_energy);
		pin2_energy = (*pin)->GetCalibratedEnergy();
		plot(DD_PINS_DE, 2, pin2_energy);
		if (root_output) {
			pid_struct.cross_pin_2_energy = pin2_energy;
			pid_struct.cross_pin_2_time = (*pin)->GetHighResTimeInNs();
		}
	}
	if (!cross_pin3_vec.empty()) {
		auto pin = max_element(cross_pin3_vec.begin(), cross_pin3_vec.end(), compare_energy);
		pin3_energy = (*pin)->GetCalibratedEnergy();
		plot(DD_PINS_DE, 3, pin3_energy);
		if (root_output) {
			pid_struct.cross_pin_3_energy = pin3_energy;
			pid_struct.cross_pin_3_time = (*pin)->GetHighResTimeInNs();
		}
	}

	//** Cross plastic *//
	if (!cross_scint_b1_vec.empty()) {
		auto cross_scint = max_element(cross_scint_b1_vec.begin(), cross_scint_b1_vec.end(), compare_energy);
		cross_scint_b1_energy = (*cross_scint)->GetCalibratedEnergy();
		if (root_output) {
			pid_struct.cross_scint_b1_energy = cross_scint_b1_energy;
			pid_struct.cross_scint_b1_time = (*cross_scint)->GetHighResTimeInNs();
		}
	}
	if (!cross_scint_t1_vec.empty()) {
		auto cross_scint = max_element(cross_scint_t1_vec.begin(), cross_scint_t1_vec.end(), compare_energy);
		cross_scint_t1_energy = (*cross_scint)->GetCalibratedEnergy();
		if (root_output) {
			pid_struct.cross_scint_t1_energy = cross_scint_t1_energy;
			pid_struct.cross_scint_t1_time = (*cross_scint)->GetHighResTimeInNs();//in e21069B, trace is not taken for this channel/module in Gamma crate
		}
	}
	if (!cross_scint_v1_vec.empty()) {
		auto cross_scint = max_element(cross_scint_v1_vec.begin(), cross_scint_v1_vec.end(), compare_energy);
		cross_scint_v1_energy = (*cross_scint)->GetCalibratedEnergy();
		cross_scint_v1_qdc = (*cross_scint)->GetTrace().GetQdc();
		if (root_output) {
			pid_struct.cross_scint_v1_energy = cross_scint_v1_energy;
			pid_struct.cross_scint_v1_qdc = cross_scint_v1_qdc;
			pid_struct.cross_scint_v1_time = (*cross_scint)->GetHighResTimeInNs();
		}
	}
	if (!cross_scint_v2_vec.empty()) {
		auto cross_scint = max_element(cross_scint_v2_vec.begin(), cross_scint_v2_vec.end(), compare_energy);
		cross_scint_v2_energy = (*cross_scint)->GetCalibratedEnergy();
		cross_scint_v2_qdc = (*cross_scint)->GetTrace().GetQdc();
		if (root_output) {
			pid_struct.cross_scint_v2_energy = cross_scint_v2_energy;
			pid_struct.cross_scint_v2_qdc = cross_scint_v2_qdc;
			pid_struct.cross_scint_v2_time = (*cross_scint)->GetHighResTimeInNs();
		}
	}
	if (!cross_scint_v3_vec.empty()) {
		auto cross_scint = max_element(cross_scint_v3_vec.begin(), cross_scint_v3_vec.end(), compare_energy);
		cross_scint_v3_energy = (*cross_scint)->GetCalibratedEnergy();
		cross_scint_v3_qdc = (*cross_scint)->GetTrace().GetQdc();
		if (root_output) {
			pid_struct.cross_scint_v3_energy = cross_scint_v3_energy;
			pid_struct.cross_scint_v3_qdc = cross_scint_v3_qdc;
			pid_struct.cross_scint_v3_time = (*cross_scint)->GetHighResTimeInNs();
		}
	}
	if (!cross_scint_v4_vec.empty()) {
		auto cross_scint = max_element(cross_scint_v4_vec.begin(), cross_scint_v4_vec.end(), compare_energy);
		cross_scint_v4_energy = (*cross_scint)->GetCalibratedEnergy();
		cross_scint_v4_qdc = (*cross_scint)->GetTrace().GetQdc();
		if (root_output) {
			pid_struct.cross_scint_v4_energy = cross_scint_v4_energy;
			pid_struct.cross_scint_v4_qdc = cross_scint_v4_qdc;
			pid_struct.cross_scint_v4_time = (*cross_scint)->GetHighResTimeInNs();
		}
	}
	//
	//make the variables proper for DAMM plot (not ROOT file)
	if(tof0!=0){
		tof0 = tof0*10+11000;
		tof0_flip = tof0_flip*10-7100;
	}else{
		tof0 = -999;
		tof0_flip = -999;
	}
	if(tof1!=0){
		tof1 = tof1*10+10500;
		tof1_flip = tof1_flip*10-7500;
	}else{
		tof1 = -999;
		tof1_flip = -999;
	}
	if(tof2!=0){
		tof2 = tof2*50+50000;
		tof2_flip = tof2_flip*10-7500;
	}else{
		tof2 = -999;
		tof2_flip = -999;
	}
	if(tof3!=0){
		tof3 = tof3*50+44500;
		tof3_flip = tof3_flip*50-43500;
	}else{
		tof3 = -999;
		tof3_flip = -999;
	}
	if(tof4!=0){
		tof4 = tof4*50+49500; // I think these parameters might need to be changed
		tof4_flip = tof4_flip*50-47000;
	}else{
		tof4 = -999;
		tof4_flip = -999;
	}
	if(tof5!=0){
		tof5 = tof5*45+47500; // I think these parameters might need to be changed
		tof5_flip = tof5_flip*45-45.6e3;
	}else{
		tof5 = -999;
		tof5_flip = -999;
	}
	if(disp_LR!=0){
		disp_LR = disp_LR*50+5000;
	}else{
		disp_LR = -999;
	}
	if(disp_UD!=0){
		disp_UD = disp_UD*100+500;
	}else{
		disp_UD = -999;
	}

	//will add more here
	//////////////////////////////////////

	TString stop_in = "";
	if(cross_scint_t1_energy>0 || cross_scint_b1_energy>0){
		stop_in = "scint";
	}
	if(pin0_energy>0){
		stop_in = "pin0";
	}
	if(pin1_energy>0){
		stop_in = "pin1";
	}
	double fit_energy = 0, yso_energy = 0, rit_energy=0;
	bool YSO_Implant = false, FIT_Implant = false, RIT_Implant = false;
	if (TreeCorrelator::get()->checkPlace("pspmt_FIT_0")) {
		fit_energy = TreeCorrelator::get()->place("pspmt_FIT_0")->last().energy;
		if(abs(fit_energy-fit_energy_prev)<1e-3){
			fit_energy = 0;
		}else{
			fit_energy_prev = fit_energy;
			if (fit_energy > fit_threshold_) {
				FIT_Implant = true;
				// cout<<"FIT_Implant"<<endl;
				stop_in = "FIT";
			}
		}
	}
	if (TreeCorrelator::get()->checkPlace("pspmt_dynode_low_0")) {
		//std::cout<<"pspmt_dynoe_low_0 = "<<TreeCorrelator::get()->place("pspmt_dynode_low_0")->last().energy<<std::endl;
		yso_energy = TreeCorrelator::get()->place("pspmt_dynode_low_0")->last().energy;
		if(abs(yso_energy-yso_energy_prev)<1e-3){
			yso_energy = 0;
		}else{
			yso_energy_prev = yso_energy;
			if (yso_energy > yso_threshold_) {
				// cout<<"YSO_Implant energy = " << TreeCorrelator::get()->place("pspmt_dynode_low_0")->last().energy<<endl;
				YSO_Implant = true;
				FIT_Implant = false;
				stop_in = "YSO";
			}
		}
	}else if(TreeCorrelator::get()->checkPlace("mtasimplantsipm_dyn_l_0")) {
		//std::cout<<"pspmt_dynoe_low_0 = "<<TreeCorrelator::get()->place("pspmt_dynode_low_0")->last().energy<<std::endl;
		yso_energy = TreeCorrelator::get()->place("mtasimplantsipm_dyn_l_0")->last().energy;
		if(abs(yso_energy-yso_energy_prev)<1e-3){
			yso_energy = 0;
		}else{
			yso_energy_prev = yso_energy;
			if (yso_energy > yso_threshold_) {
				// cout<<"YSO_Implant energy = " << TreeCorrelator::get()->place("pspmt_dynode_low_0")->last().energy<<endl;
				YSO_Implant = true;
				FIT_Implant = false;
				stop_in = "YSO";
			}
		}
	}
	if (TreeCorrelator::get()->checkPlace("pspmt_RIT_0")) {
		rit_energy = TreeCorrelator::get()->place("pspmt_RIT_0")->last().energy;
		if(abs(rit_energy-rit_energy_prev)<1e-3){
			rit_energy = 0;
		}else{
			rit_energy_prev = rit_energy;
			if (rit_energy > rit_threshold_) {
				RIT_Implant = true;
				YSO_Implant = false;
				FIT_Implant = false;
				stop_in = "RIT";
			}
		}
	}

	pid_struct.stop_in = stop_in;
	pid_struct.fit_energy = fit_energy;
	pid_struct.yso_energy = yso_energy;
	pid_struct.rit_energy = rit_energy;

	// Fill the plots
	plot(D_IMAGEL_MULT, imageL_vec.size());
	plot(D_DISPL_MULT, dispL_vec.size());
	plot(DD_PINS_MULT, 0, cross_pin0_vec.size());
	plot(DD_PINS_MULT, 1, cross_pin1_vec.size());
	plot(DD_PINS_MULT, 2, cross_pin2_vec.size());
	plot(DD_PINS_MULT, 3, cross_pin3_vec.size());
	plot(DD_PIN0_1, pin0_energy, pin1_energy);
	plot(DD_PIN2_3, pin2_energy, pin3_energy);
	plot(DD_PIN0_2, pin0_energy, pin2_energy);
	plot(DD_PIN1_3, pin1_energy, pin3_energy);
	plot(D_DISPLR, disp_LR);
	plot(D_DISPUD, disp_UD);
	plot(DD_DISP_PLANE,disp_UD/8.,disp_LR/8.);

	plot(DD_TOF2_PIN1, tof2, pin1_energy);
	plot(DD_TOF3_PIN1, tof3, pin1_energy);
	if(!tofflip_){
		plot(DD_TOF0_PIN0, tof0, pin0_energy);
		plot(DD_TOF1_PIN0, tof1, pin0_energy);
		plot(DD_TOF2_PIN0, tof2, pin0_energy);
		plot(DD_TOF3_PIN0, tof3, pin0_energy);
	}else{
		plot(DD_TOF0_PIN0, tof0_flip, pin0_energy);
		plot(DD_TOF1_PIN0, tof1_flip, pin0_energy);
		plot(DD_TOF2_PIN0, tof2_flip, pin0_energy);
		plot(DD_TOF3_PIN0, tof3_flip, pin0_energy);
	}

	if(!tofflip_){
		plot(DD_TOF4_PIN2, tof4, pin2_energy);
		plot(DD_TOF5_PIN2, tof5, pin2_energy);
		plot(DD_TOF4_PIN3, tof4, pin3_energy);
		plot(DD_TOF5_PIN3, tof5, pin3_energy);
	}else{
		plot(DD_TOF4_PIN2, tof4_flip, pin2_energy);
		plot(DD_TOF5_PIN2, tof5_flip, pin2_energy);
		plot(DD_TOF4_PIN3, tof4_flip, pin3_energy);
		plot(DD_TOF5_PIN3, tof5_flip, pin3_energy);
	}

	if (FIT_Implant) {
		plot(DD_TOF2_PIN1_GATED_FIT, tof2, pin1_energy);
		plot(DD_TOF3_PIN1_GATED_FIT, tof3, pin1_energy);
		if(!tofflip_){
			plot(DD_TOF0_PIN0_GATED_FIT, tof0, pin0_energy);
			plot(DD_TOF1_PIN0_GATED_FIT, tof1, pin0_energy);
			plot(DD_TOF2_PIN0_GATED_FIT, tof2, pin0_energy);
			plot(DD_TOF3_PIN0_GATED_FIT, tof3, pin0_energy);
		}else{
			plot(DD_TOF0_PIN0_GATED_FIT, tof0_flip, pin0_energy);
			plot(DD_TOF1_PIN0_GATED_FIT, tof1_flip, pin0_energy);
			plot(DD_TOF2_PIN0_GATED_FIT, tof2_flip, pin0_energy);
			plot(DD_TOF3_PIN0_GATED_FIT, tof3_flip, pin0_energy);
		}
	}
	if (YSO_Implant) {
		plot(DD_TOF2_PIN1_GATED_YSO, tof2, pin1_energy);
		plot(DD_TOF3_PIN1_GATED_YSO, tof3, pin1_energy);
		if(!tofflip_){
			plot(DD_TOF0_PIN0_GATED_YSO, tof0, pin0_energy);
			plot(DD_TOF1_PIN0_GATED_YSO, tof1, pin0_energy);
			plot(DD_TOF2_PIN0_GATED_YSO, tof2, pin0_energy);
			plot(DD_TOF3_PIN0_GATED_YSO, tof3, pin0_energy);
		}else{
			plot(DD_TOF0_PIN0_GATED_YSO, tof0_flip, pin0_energy);
			plot(DD_TOF1_PIN0_GATED_YSO, tof1_flip, pin0_energy);
			plot(DD_TOF2_PIN0_GATED_YSO, tof2_flip, pin0_energy);
			plot(DD_TOF3_PIN0_GATED_YSO, tof3_flip, pin0_energy);
		}
	}
	if (RIT_Implant) {
		plot(DD_TOF2_PIN1_GATED_RIT, tof2, pin1_energy);
		plot(DD_TOF3_PIN1_GATED_RIT, tof3, pin1_energy);
		if(!tofflip_){
			plot(DD_TOF0_PIN0_GATED_RIT, tof0, pin0_energy);
			plot(DD_TOF1_PIN0_GATED_RIT, tof1, pin0_energy);
			plot(DD_TOF2_PIN0_GATED_RIT, tof2, pin0_energy);
			plot(DD_TOF3_PIN0_GATED_RIT, tof3, pin0_energy);
		}else{
			plot(DD_TOF0_PIN0_GATED_RIT, tof0_flip, pin0_energy);
			plot(DD_TOF1_PIN0_GATED_RIT, tof1_flip, pin0_energy);
			plot(DD_TOF2_PIN0_GATED_RIT, tof2_flip, pin0_energy);
			plot(DD_TOF3_PIN0_GATED_RIT, tof3_flip, pin0_energy);
		}
	}

	if(tof3>0 && tof3<1600 && pin1_energy>500 && pin1_energy<5000){
		if(stop_in=="scint"){
			plot(D_RANGE, 1);
		}else if(stop_in=="pin0"){
			plot(D_RANGE, 2);
		}else if(stop_in=="pin1"){
			plot(D_RANGE, 3);
		}else if(stop_in=="FIT"){
			plot(D_RANGE, 4);
		}else if(stop_in=="YSO"){
			plot(D_RANGE, 5);
		}else if(stop_in=="RIT"){
			plot(D_RANGE, 6);
		}
	}

	pair<double, double> scint_pos = GetCrossScintPosition(cross_scint_v4_qdc,cross_scint_v1_qdc,cross_scint_v2_qdc,cross_scint_v3_qdc);
	double pos_scale = 512;
	double pos_offset = 1024;
	double scint_x = scint_pos.first;
	double scint_y = scint_pos.second;
	if(scint_x>-2 && scint_x<2 && scint_y>-2 && scint_y<2){
		plot(DD_CROSS_SCINT_POS, scint_x*pos_scale+pos_offset, scint_y*pos_scale+pos_offset);
	}

	if (root_output) {
		// Fill the event to the PixeTreeEvent object
		pixie_tree_event_->pid_vec_.emplace_back(pid_struct);
		pid_struct = processor_struct::PID_DEFAULT_STRUCT;
	}

	EndProcess();
	return true;
}

bool PidProcessor::Process(RawEvent &event) {
	if (!EventProcessor::Process(event))
		return false;

	EndProcess();
	return true;
}

double PidProcessor::CorrectTofByPosition(double &tof, double &position, double &slope, double &intercept) {
	return tof - slope * position - intercept;
}

double PidProcessor::ConvertTofToAQ(double &tof) {
	return tof;
}

double PidProcessor::ConvertPinToZ(double &pin) {
	return pin;
}

pair<double, double> PidProcessor::GetCrossScintPosition(double qdc1, double qdc2, double qdc3, double qdc4){
	double xcorr=-999., ycorr=-999.;
	if(qdc1>0 && qdc2>0 && qdc3>0 && qdc4>0){
		double a = 0.85;
		double b = 2.38;
		double c = -2.15;
		double d = 0.53;
		double e = 0.16;
		double f = -1.21;
		double S = 6.2;
		double A1 = 4.00;
		double A2 = 4.00;
		double A3 = 4.00;
		double A4 = 4.00;
		double xcm = (-qdc1/A1+qdc2/A2-qdc3/A3+qdc4/A4)/(qdc1/A1+qdc2/A2+qdc3/A3+qdc4/A4);
		double ycm = (+qdc1/A1+qdc2/A2-qdc3/A3-qdc4/A4)/(qdc1/A1+qdc2/A2+qdc3/A3+qdc4/A4);
		double r = sqrt(pow(xcm,2)+pow(ycm,2));
		double r2 = pow(r,2);
		double r3 = r*r2;
		double r4 = pow(r2,2);
		double theta = atan(ycm/xcm) + (xcm<0? M_PI : 0);
		double cos1 = cos(theta);
		double cos3 = pow(cos1,3);
		double sin1 = sin(theta);
		double sin3 = pow(sin1,3);
		xcorr = S*xcm/(1+a*r+b*r2+c*r3+d*r4) + r*(e*cos1+f*cos3);
		ycorr = S*ycm/(1+a*r+b*r2+c*r3+d*r4) + r*(e*sin1+f*sin3);
	}
	return make_pair(xcorr, ycorr);
}
