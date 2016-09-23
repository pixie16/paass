#ifndef SCANOR_INTERFACE_HPP
#define SCANOR_INTERFACE_HPP

#include <vector>
#include "Unpacker.hpp"

class ScanorInterface {
public:
    static ScanorInterface *get();

    ~ScanorInterface();

    void Hissub(unsigned short *sbuf[],unsigned short *nhw);
    virtual void Drrsub(uint32_t& iexist);
    ///@param [in] a : A reference to an Unpacker object.
    void SetUnpacker(Unpacker &a) {unpacker_ = a;}

protected:
/** Constructor that initializes the various processors and analyzers. */
    ScanorInterface();
    ScanorInterface(const ScanorInterface &); //!< Overloaded constructor
    ScanorInterface &operator=(ScanorInterface const &);//!< Equality constructor

private:
    bool MakeModuleData(const uint32_t *data, unsigned long nWords, unsigned
                            int maxWords);
    static ScanorInterface *instance_;//!< The only instance of ScanorInterface
    Unpacker unpacker_;//!< The unpacker object that we are going to use to
                       //!< decode the data.
};

#endif //#ifndef SCANOR_INTERFACE_HPP
