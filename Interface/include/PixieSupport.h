#ifndef PIXIE_SUPPORT_H
#define PIXIE_SUPPORT_H

#include <string>

#include "PixieInterface.h"

#define NUM_TOGGLE_BITS 19

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

/*template<typename T>
bool forModule(PixieInterface &pif, int mod, 
	       PixieFunction<T> &f, T par = T() );
template<typename T>
bool forChannel(PixieInterface &pif, int mod, int ch, 
		PixieFunction<T> &f, T par = T() );*/

// implementation follows
// perform the function for the specified module and channel
//   except if the channel < 0, do for all channels
//   or if the module < 0, do for all modules

template<typename T>
bool forChannel(PixieInterface *pif, int mod, int ch, PixieFunction<T> &f, T par){
	PixieFunctionParms<T> parms(pif, par);

	bool hadError = false;

	if (mod < 0) {
		for (parms.mod = 0; parms.mod < pif->GetNumberCards(); parms.mod++) {
			if (ch < 0) {
				for (parms.ch = 0; parms.ch < pif->GetNumberChannels(); parms.ch++) {
					if (!f(parms)){ hadError = true; }
				}
			} 
			else {
				parms.ch = ch;
				if (!f(parms))
					hadError = true;
			}
		}
	} 
	else {
		parms.mod = mod;
		if (ch < 0) {
			for (parms.ch = 0; parms.ch < pif->GetNumberChannels(); parms.ch++) {
				if (!f(parms)){ hadError = true; }
			}
		} 
		else {
			parms.ch = ch;
			hadError = !f(parms);
		}
	}

	return !hadError;
}

template<typename T>
bool forModule(PixieInterface *pif, int mod, PixieFunction<T> &f, T par)
{
	PixieFunctionParms<T> parms(pif, par);
	bool hadError = false;

	if (mod < 0) {
		for (parms.mod = 0; parms.mod < pif->GetNumberCards(); parms.mod++) {
			if (!f(parms)){ hadError = true; }
		}
	} 
	else {
		parms.mod = mod;
		hadError = !f(parms);
	}

	return !hadError;
}

class BitFlipper : public PixieFunction<std::string>{
  private:
	unsigned int bit;

	bool operator()(PixieFunctionParms<std::string> &par);

  public:
	static const std::string toggle_names[NUM_TOGGLE_BITS];
	static const std::string csr_txt[NUM_TOGGLE_BITS];

	BitFlipper(){ bit = 0; }
	
	BitFlipper(unsigned int bit_){ bit = bit_; }
	
	void Help();
	
	void SetBit(unsigned int bit_){ bit = bit_; }
	
	void SetBit(char *bit_);	
	
	void SetBit(std::string bit_);
	
	void CSRA_test(unsigned int input_);
};

class ParameterChannelWriter : public PixieFunction< std::pair<std::string, float> >{
  public:
	bool operator()(PixieFunctionParms< std::pair<std::string, float> > &par);
};

class ParameterModuleWriter : public PixieFunction< std::pair<std::string, unsigned long> >{
  public:
	bool operator()(PixieFunctionParms< std::pair<std::string, unsigned long> > &par);
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
