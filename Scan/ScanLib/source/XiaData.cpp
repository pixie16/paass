///@file XiaData.cpp
///@brief A class that holds information from the XIA LLC. Pixie-16 List
/// Mode Data
///@authors C. R. Thornsberry and S. V. Paulauskas
#include "XiaData.hpp"

///@TODO These methods are commented out since I'm not sure that we need them
/// . This should be verified at some point.
//void XiaData::reserve(const size_t &size){
//	if(size == 0){ return; }
//	trace_.reserve(size);
//}
//
//void XiaData::assign(const size_t &size, const unsigned int &input){
//	trace_.assign(size, input);
//}
//
//void XiaData::push_back(const unsigned int &input){
//	trace_.push_back(input);
//}

///Clears all of the varaibles. For values where 0 is a valid option
/// (crateNum_, chanNum_, etc.) the values are set to 9999. The vectors
/// are all cleared using the clear() method. This method is called when the
/// class is first initiatlized so that it has some default values for the
/// software to use in the event that they are needed.
void XiaData::Clear(){
    cfdForceTrig_ = false;
    cfdTrigSource_ = false;
    isPileup_ = false;
    isSaturated_ = false;
    isVirtualChannel_ = false;

    energy_ = 0.0;

	chanNum_ = 9999;
	cfdTime_ = 0;
    crateNum_ = 9999;
    eventTimeHigh_ = 0;
	eventTimeLow_ = 0;
    slotNum_ = 9999;

    qdc_.clear();
    trace_.clear();
}