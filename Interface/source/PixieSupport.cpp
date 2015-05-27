#include <iostream>
#include <iomanip>
#include <stdlib.h>

#include "Display.h"

#include "PixieSupport.h"
#include "pixie16app_defs.h"
#include "pixie16app_export.h"

std::string PadStr(const std::string &input_, int width_){
	std::string output = input_;
	for(int i = output.size(); i < width_; i++){
		output += ' ';
	}
	
	return output;
}

bool BitFlipper::operator()(PixieFunctionParms<std::string> &par){
	if(bit >= num_toggle_bits){ return false; }

	bool *active_bits = new bool[num_toggle_bits];
	int *bit_values = new int[num_toggle_bits];
	int count = 1;

	double value;
	par.pif->ReadSglChanPar(par.par.c_str(), &value, par.mod, par.ch);
	
	int old_csra = (int)value;
	for(unsigned int i = 0; i < num_toggle_bits; i++){
		bit_values[i] = count; count *= 2;
		if(old_csra & (1 << i)){ active_bits[i] = true; }
		else{ active_bits[i] = false; }
	}
	
	int new_csra;
	if(active_bits[bit]){ // need to toggle bit off (subtract)
		new_csra = old_csra - bit_values[bit];
	}
	else{ // need to toggle bit on (add)
		new_csra = old_csra + bit_values[bit];
	}
	
	if(par.pif->WriteSglChanPar(par.par.c_str(), new_csra, par.mod, par.ch)){
		par.pif->PrintSglChanPar(par.par.c_str(), par.mod, par.ch);
		return true;
	}

	delete[] active_bits;
	delete[] bit_values;
		
	return false;
}

#ifdef PIF_REVA
const std::string BitFlipper::toggle_names[19] = {"group", "live", "good", "read", "trigger", "polarity", "GFLT", "", "", 
												  "", "", "", "", "", "gain", "", "", "", ""};
const std::string BitFlipper::csr_txt[19] = {"Respond to group triggers only", "Measure individual live time", "Good Channel", "Read always", "Enable trigger", 
											 "Trigger positive", "GFLT", "", "", "", "", "", "", "", "HI/LO gain", "", "", "", ""};
#else
const std::string BitFlipper::toggle_names[19] = {"", "", "good", "", "", "polarity", "", "", "trace", "QDC", "CFD", 
											      "global", "raw", "trigger", "gain", "pileup", "catcher", "", "SHE"};
const std::string BitFlipper::csr_txt[19] = {"", "", "Good Channel", "", "", "Trigger positive", "", "", "Enable trace capture", "Enable QDC sums capture", 
										     "Enable CFD trigger mode", "Enable global trigger validation", "Enable raw energy sums capture", 
											 "Enable channel trigger validation", "HI/LO gain", "Pileup rejection control", "Hybrid bit", "", 
											 "SHE single trace capture"};
#endif

void BitFlipper::Help(){
	std::cout << " Valid CSRA bits:\n";
	for(unsigned int i = 0; i < num_toggle_bits; i++){
		if(toggle_names[i] != ""){
			if(i < 10){ std::cout << "  0" << i << " - " << toggle_names[i] << std::endl; }
			else{ std::cout << "  " << i << " - " << toggle_names[i] << std::endl; }
		}
		else{
			if(i < 10){ std::cout << "  0" << i << " - " << toggle_names[i] << std::endl; }
			else{ std::cout << "  " << i << " - " << toggle_names[i] << std::endl; }
		}
	}
}

void BitFlipper::SetCSRAbit(std::string bit_){
	SetBit(atoi(bit_.c_str()));
	
	for(unsigned int i = 0; i < num_toggle_bits; i++){
    	if(bit_ == toggle_names[i]){
    		SetBit(i);
    		break;
    	}
    }
}

void BitFlipper::SetBit(std::string bit_){
	SetBit(std::strtoul(bit_.c_str(), NULL, 0));
}

void BitFlipper::CSRAtest(unsigned int input_){
	Test(19, input_, csr_txt);
}

bool BitFlipper::Test(unsigned int num_bits_, unsigned int input_, const std::string *text_/*=NULL*/){
	if(num_bits_ > 32){ return false; } // Too many bits for unsigned int
	
	bool *active_bits = new bool[num_bits_];
	unsigned int *bit_values = new unsigned int[num_bits_];
	unsigned int *running_total = new unsigned int[num_bits_];
	
	unsigned int total = 0;
	unsigned int count = 1;

	for(unsigned int i = 0; i < num_bits_; i++){
		bit_values[i] = count;
		if(input_ & (1 << i)){ 
			active_bits[i] = true; 
			
			total += count;
			running_total[i] = total;
		}
		else{ 
			active_bits[i] = false; 
			running_total[i] = 0;
		}
		count *= 2;
	}
	
	std::cout << " Input: 0x" << std::hex << input_ << " (" << std::dec << input_ << ")\n";
	if(text_ != NULL){ std::cout << "  Bit   On?    Value       Total    Bit Function\n"; }
	else{ std::cout << "  Bit   On?    Value       Total\n"; }

	std::string bit_function;
	for(unsigned int i = 0; i < num_bits_; i++){
		if(text_ != NULL){ bit_function = csr_txt[i]; }
		else{ bit_function = ""; }
		
		if(active_bits[i]){ 
			if(Display::hasColorTerm){
				if(i < 10){ 
					std::cout << TermColors::DkGreen << "   0" << i << "    1  " << PadStr(bit_values[i], 12);
					std::cout << PadStr(running_total[i], 12) << bit_function << TermColors::Reset << std::endl; 
				}
				else{ 
					std::cout << TermColors::DkGreen << "   " << i << "    1  " << PadStr(bit_values[i], 12);
					std::cout << PadStr(running_total[i], 12) << bit_function << TermColors::Reset << std::endl;
				}
			}
			else{
				if(i < 10){ std::cout << "   " << i << "    1  " << PadStr(bit_values[i], 12) << PadStr(running_total[i], 12) << bit_function << std::endl; }
				else{ std::cout << "   " << i << "    1  " << PadStr(bit_values[i], 12) << PadStr(running_total[i], 12) << bit_function << std::endl; }
			}
		}
		else{ 
			if(i < 10){ 
				std::cout << "   0" << i << "    0  " << PadStr(bit_values[i], 12);
				std::cout << PadStr(running_total[i], 12) << bit_function << std::endl; 
			}
			else{ 
				std::cout << "   " << i << "    0  " << PadStr(bit_values[i], 12);
				std::cout << PadStr(running_total[i], 12) << bit_function << std::endl; 
			}
		}
	}

	delete[] active_bits;
	delete[] bit_values;
	delete[] running_total;
	
	return true;
}

bool ParameterChannelWriter::operator()(PixieFunctionParms< std::pair<std::string, double> > &par){
	if(par.pif->WriteSglChanPar(par.par.first.c_str(), par.par.second, par.mod, par.ch)){
		par.pif->PrintSglChanPar(par.par.first.c_str(), par.mod, par.ch);
		return true;
	}
	return false;
}

bool ParameterModuleWriter::operator()(PixieFunctionParms< std::pair<std::string, unsigned int> > &par){
	if(par.pif->WriteSglModPar(par.par.first.c_str(), par.par.second, par.mod)){
		par.pif->PrintSglModPar(par.par.first.c_str(), par.mod);
		return true;
	} 
	return false;
}

bool ParameterChannelReader::operator()(PixieFunctionParms<std::string> &par){
	par.pif->PrintSglChanPar(par.par.c_str(), par.mod, par.ch);
	return true;
}

bool ParameterModuleReader::operator()(PixieFunctionParms<std::string> &par){
	par.pif->PrintSglModPar(par.par.c_str(), par.mod);
	return true;
}

bool ParameterChannelDumper::operator()(PixieFunctionParms<std::string> &par){
	double value;
	par.pif->ReadSglChanPar(par.par.c_str(), &value, (int)par.mod, (int)par.ch);
	*file << par.mod << "\t" << par.ch << "\t" << par.par << "\t" << value << std::endl;
	return true;
}

bool ParameterModuleDumper::operator()(PixieFunctionParms<std::string> &par){
	PixieInterface::word_t value;
	par.pif->ReadSglModPar(par.par.c_str(), &value, (int)par.mod);
	*file << par.mod << "\t" << par.par << "\t" << value << std::endl;
	return true;
}

bool OffsetAdjuster::operator()(PixieFunctionParms<int> &par){
	bool hadError = par.pif->AdjustOffsets(par.mod);
	for(size_t ch = 0; ch < par.pif->GetNumberChannels(); ch++){
		par.pif->PrintSglChanPar("VOFFSET", par.mod, ch);
	}

	return hadError;
}

bool TauFinder::operator()(PixieFunctionParms<> &par){
	double tau[16];
  
	int errorNum = Pixie16TauFinder(par.mod, tau);
	if(par.ch < 16){
		std::cout << "TAU: " << tau[par.ch] << std::endl;
	}
	std::cout << "Errno: " << errorNum << std::endl;

	return (errorNum >= 0);
}
