#ifndef PIXIE_SUPPORT_H
#define PIXIE_SUPPORT_H

#include <vector>
#include <string>
#include <sstream>
#include <bitset>

#include "PixieInterface.h"

extern bool hasColorTerm;

template<typename T=int>
struct PixieFunctionParms{
	PixieInterface *pif;
	unsigned int mod;
	unsigned int ch;
	T par;
  
	PixieFunctionParms(PixieInterface *p, T x) : pif(p) {par=x;}
};

template<typename T=int>
class PixieFunction : public std::unary_function<bool, struct PixieFunctionParms<T> >{
	public:
	virtual bool operator()(struct PixieFunctionParms<T> &par) = 0;
	virtual ~PixieFunction() {};
};

template<typename T>
bool forChannel(PixieInterface *pif, int modStart, int modStop, int chStart, int chStop, PixieFunction<T> &f, T par){
	PixieFunctionParms<T> parms(pif, par);

	bool hadError = false;

	if (modStart < 0) {
		modStart = 0;
		modStop = pif->GetNumberCards() - 1;
	}
	if (chStart < 0) {
		chStart = 0;
		chStop = pif->GetNumberChannels() - 1;
	}

	for (parms.mod = modStart; parms.mod <= modStop; parms.mod++) {
		for (parms.ch = chStart; parms.ch <= chStop; parms.ch++) {
			if (!f(parms)){ hadError = true; }
		}
	} 

	return !hadError;
}

template<typename T>
bool forChannel(PixieInterface *pif, int mod, int ch, PixieFunction<T> &f, T par){
	return forChannel(pif, mod, mod, ch, ch, f, par);
}

template<typename T>
bool forModule(PixieInterface *pif, int modStart, int modStop, PixieFunction<T> &f, T par)
{
	PixieFunctionParms<T> parms(pif, par);
	bool hadError = false;

	if (modStart < 0) {
		modStart = 0;
		modStop = pif->GetNumberCards() -1;
	}
	for (parms.mod = modStart; parms.mod <= modStop; parms.mod++) {
		if (!f(parms)){ hadError = true; }
	}

	return !hadError;
}

template<typename T>
bool forModule(PixieInterface *pif, int mod, PixieFunction<T> &f, T par)
{
	return forModule(pif, mod, mod, f, par);
}


std::string PadStr(const std::string &input_, int width_);

template<typename T>
std::string PadStr(const T &input_, int width_){
	std::stringstream stream;
	stream << input_;
	
	std::string output = stream.str();
	for(int i = output.size(); i < width_; i++){
		output += ' ';
	}
	
	return output;
}

class BitFlipper : public PixieFunction<std::string>{
  private:
  	unsigned int num_toggle_bits;
	unsigned int bit;

	bool operator()(PixieFunctionParms<std::string> &par);

  public:
	const static std::vector<std::string> toggle_names;
	const static std::vector<std::string> csr_txt;

	BitFlipper(){ 
		bit = 0; 
		num_toggle_bits = 19;
	}
	
	BitFlipper(unsigned int bit_, unsigned int num_toggle_bits_=19){ 
		bit = bit_; 
		num_toggle_bits = num_toggle_bits_;
	}
	
	void Help();
	
	void SetBit(unsigned int bit_){ bit = bit_; }
	
	void SetBit(std::string bit_);	
	
	void SetCSRAbit(std::string bit_);
	
	void CSRAtest(unsigned int input_);
	
	bool Test(unsigned int num_bits_, unsigned int input_, const std::vector<std::string> &text_);
};

class GetTraces : public PixieFunction<int>{
  private:
	unsigned short *total_data; /// Array for storing all 16 channels of a module.
	unsigned short *trace_data; /// Array for storing a single trace.

	int threshold; /// The trigger threshold above baseline.

	size_t total_len; /// Length of total data array.
	size_t trace_len; /// Length of trace data array.
	
	int attempts; /// Number of attempts to reach threshold.
	float baseline[NUMBER_OF_CHANNELS]; /// Calculated baseline for each channel.
	float maximum[NUMBER_OF_CHANNELS]; /// The maximum ADC value above baseline for each channel.
	bool status; /// Set to true when a valid trace is found.
	bool correct_baselines; /// Correct the baselines of the output traces.

	/// Fit... tau?
	double FitTau(const unsigned short* trace, size_t b0, size_t b1, size_t x0, size_t x1);
  
  public:
	GetTraces(unsigned short* total_data_, size_t total_size_, unsigned short *trace_data_, size_t trace_size_, int threshold_=0, bool correct_baselines_=false);

	int GetAttempts(){ return attempts; }

	float GetBaseline(size_t chan_){ return (chan_<NUMBER_OF_CHANNELS?baseline[chan_]:-1); }

	float GetMaximum(size_t chan_){ return (chan_<NUMBER_OF_CHANNELS?maximum[chan_]:-1); }
	
	bool GetStatus(){ return status; }

	void Help();
	
	bool operator()(PixieFunctionParms<int> &par);
};

class ParameterChannelWriter : public PixieFunction< std::pair<std::string, double> >{
  public:
	bool operator()(PixieFunctionParms< std::pair<std::string, double> > &par);
};

class ParameterModuleWriter : public PixieFunction< std::pair<std::string, unsigned int> >{
  public:
	bool operator()(PixieFunctionParms< std::pair<std::string, unsigned int> > &par);
};

class ParameterChannelReader : public PixieFunction<std::string>{
  public:
	bool operator()(PixieFunctionParms<std::string> &par);
};

class ParameterModuleReader : public PixieFunction<std::string>{
  public:
	bool operator()(PixieFunctionParms<std::string> &par);
};

class ParameterChannelDumper : public PixieFunction<std::string>{
  public:
	std::ofstream *file;
	ParameterChannelDumper(std::ofstream *file_){ file = file_; }
	bool operator()(PixieFunctionParms<std::string> &par);
};

class ParameterModuleDumper : public PixieFunction<std::string>{
  public:
	std::ofstream *file;
	ParameterModuleDumper(std::ofstream *file_){ file = file_; }
	bool operator()(PixieFunctionParms<std::string> &par);
};

class OffsetAdjuster : public PixieFunction<int>{
  public:
	bool operator()(PixieFunctionParms<int> &par);
};

class TauFinder : public PixieFunction<int>{
  public:
	bool operator()(PixieFunctionParms<int> &par);
};

#endif
