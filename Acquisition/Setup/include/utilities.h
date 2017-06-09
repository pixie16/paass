///@file utilities.h
/// @author D. T. Miller
/// @date  August 2010

#ifndef __UTILITIES_H_
#define __UTILITIES_H_ 1

#include <functional>

#include "PixieInterface.h"

template<typename T=int>
struct PixieFunctionParms {
    PixieInterface &pif;
    unsigned int mod;
    unsigned int ch;
    T par;

    PixieFunctionParms(PixieInterface &p, T x) : pif(p) { par = x; }
};

template<typename T=int>
class PixieFunction :
        public std::unary_function<bool, struct PixieFunctionParms<T> > {
public:
    virtual bool operator()(struct PixieFunctionParms<T> &par) = 0;

    virtual ~PixieFunction() {};
};

template<typename T>
bool forModule(PixieInterface &pif, int mod,
               PixieFunction<T> &f, T par = T());

template<typename T>
bool forChannel(PixieInterface &pif, int mod, int ch,
                PixieFunction<T> &f, T par = T());

// implementation follows
// perform the function for the specified module and channel
//   except if the channel < 0, do for all channels
//   or if the module < 0, do for all modules

template<typename T>
bool forChannel(PixieInterface &pif, int mod, int ch,
                PixieFunction<T> &f, T par) {
    PixieFunctionParms<T> parms(pif, par);

    bool hadError = false;

    if (mod < 0) {
        for (parms.mod = 0; parms.mod < pif.GetNumberCards(); parms.mod++) {
            if (ch < 0) {
                for (parms.ch = 0;
                     parms.ch < pif.GetNumberChannels(); parms.ch++) {
                    if (!f(parms))
                        hadError = true;
                }
            } else {
                parms.ch = ch;
                if (!f(parms))
                    hadError = true;
            }
        }
    } else {
        parms.mod = mod;
        if (ch < 0) {
            for (parms.ch = 0; parms.ch < pif.GetNumberChannels(); parms.ch++) {
                if (!f(parms))
                    hadError = true;
            }
        } else {
            parms.ch = ch;
            hadError = !f(parms);
        }
    }

    return !hadError;
}

template<typename T>
bool forModule(PixieInterface &pif, int mod, PixieFunction<T> &f, T par) {
    PixieFunctionParms<T> parms(pif, par);
    bool hadError = false;

    if (mod < 0) {
        for (parms.mod = 0; parms.mod < pif.GetNumberCards(); parms.mod++) {
            if (!f(parms))
                hadError = true;
        }
    } else {
        parms.mod = mod;
        hadError = !f(parms);
    }

    return !hadError;
}

#endif // __UTILITIES_H_
