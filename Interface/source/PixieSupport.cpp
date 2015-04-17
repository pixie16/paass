#include <iostream>
#include <iomanip>

#include "PixieSupport.h"
#include "pixie16app_export.h"

bool ParameterChannelWriter::operator()(PixieFunctionParms< std::pair<std::string, float> > &par){
	if(par.pif->WriteSglChanPar(par.par.first.c_str(), par.par.second, par.mod, par.ch)){
		par.pif->PrintSglChanPar(par.par.first.c_str(), par.mod, par.ch);
		return true;
	}
	return false;
}

bool ParameterModuleWriter::operator()(PixieFunctionParms< std::pair<std::string, unsigned long> > &par){
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

void CSRA_test(int input_){
	const size_t num_bits = 19;

	std::string CSR_TXT[num_bits];

#ifdef PIF_REVA
	CSR_TXT[0] = "Respond to group triggers only";
	CSR_TXT[1] = "Measure individual live time";
	CSR_TXT[3] = "Read always";
	CSR_TXT[4] = "Enable trigger";
	CSR_TXT[6] = "GFLT";
#else  // Rev. A
	CSR_TXT[CCSRA_TRACEENA] = "Enable trace capture";
	CSR_TXT[CCSRA_QDCENA]   = "Enable QDC sums capture";
	CSR_TXT[10] = "Enable CFD trigger mode";
	CSR_TXT[11] = "Enable global trigger validation";
	CSR_TXT[12] = "Enable raw energy sums capture";
	CSR_TXT[13] = "Enable channel trigger validation";
	CSR_TXT[15] = "Pileup rejection control";
	CSR_TXT[16] = "Hybrid bit";
	CSR_TXT[18] = "SHE single trace capture";
#endif // (else) Rev.A

	CSR_TXT[CCSRA_GOOD]     = "Good Channel";
	CSR_TXT[CCSRA_POLARITY] = "Trigger positive";
	CSR_TXT[CCSRA_ENARELAY] = "HI/LO gain";

	std::cout << "  Input: " << std::dec << input_ << " (0x" << std::hex << input_ << ")\n\n";
	std::cout << "   CSRA bits:\n";

	size_t max_len = 0;
	for (size_t k = 0; k < num_bits; k++){ 
		if(CSR_TXT[k].length() > max_len)
			max_len = CSR_TXT[k].length();
	}

	for(size_t k = 0; k < num_bits; k++){
		int retval = APP32_TstBit(k, input_);
		std::cout << "   " << retval << " " << std::setw(max_len) << CSR_TXT[k] << "  " << std::setw(2) << k << "  " << (1 << k) * retval << std::endl;
	}
}
