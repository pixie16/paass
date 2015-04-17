#ifndef MCA_H
#define MCA_H

#include "PixieSupport.h"

class PixieInterface;

///Abstract MCA class
class MCA {
	protected:
		static const size_t HIS_SIZE = 16384;
		static const size_t ADC_SIZE = 32768;

		///Flag indicating if histogram construction was successful.
		bool _isOpen;
		///Pointer to the PixieInterface
		PixieInterface *_pif;
	public:
		///Default constructor
		MCA(PixieInterface *pif) : _pif(pif) {};
		virtual ~MCA() {};
		virtual bool StoreData(int mod, int ch) = 0;
		virtual bool OpenFile(const char *basename) = 0;
		///Flush the current memory to disk.
		virtual void Flush() = 0;
		///Check if the histogram construction was successful.
		virtual bool IsOpen() {return _isOpen;};
		///Start the MCA running.
		virtual void Run(float duration);

};

#endif 
