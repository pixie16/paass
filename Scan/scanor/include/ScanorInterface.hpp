///@file ScanorInterface.cpp
///@authors C. R. Thornsberry and S. V. Paulauskas
///@date September 16, 2016
#ifndef SCANOR_INTERFACE_HPP
#define SCANOR_INTERFACE_HPP

#include "Unpacker.hpp"

class ScanorInterface {
public:
    static ScanorInterface *get();

    ~ScanorInterface(){};

    void Hissub(unsigned short *sbuf[],unsigned short *nhw);

    ///@param [in] a : A pointer to an Unpacker object.
    void SetUnpacker(Unpacker *a) {unpacker_ = a;}

private:
    ///Default Constructor
    ScanorInterface(){};
    ScanorInterface(const ScanorInterface &){}; //!< Overloaded constructor
    ScanorInterface &operator=(ScanorInterface const &);//!< Equality constructor

    bool MakeModuleData(const uint32_t *data, unsigned long nWords, unsigned
                            int maxWords);
    static ScanorInterface *instance_;//!< The only instance of ScanorInterface
    Unpacker *unpacker_;//!< The unpacker object that we are going to use to
                       //!< decode the data.
};

#endif //#ifndef SCANOR_INTERFACE_HPP
