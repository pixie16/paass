///@file RootInterface.cpp
///@brief Provides an interface to ROOT for scan programs
///@author K. Smith and S. V. Paulauskas
///@date May 21, 2017
#ifndef __ROOTINTERFACE_HPP__
#define __ROOTINTERFACE_HPP__

#include <map>

#include "TCanvas.h"
#include "TApplication.h"

//Singleton class that handles everybody's access to ROOT
class RootInterface {
public:
    ///@return only instance of XmlInterface class
    static RootInterface *get();

    ~RootInterface();

    TCanvas *GetCanvas() { return canvas_; }

    void IdleTask();

    void UpdateZoom(TVirtualPad *pad = gPad);

    void ResetZoom(TVirtualPad *pad = gPad);

private:
    static RootInterface *instance_;
    RootInterface();
    void operator=(RootInterface const &); //!< copy constructor

    TCanvas *canvas_;

    static const int numAxes_ = 3;
    struct AxisInfo {
        double rangeUserMin[numAxes_];
        double rangeUserMax[numAxes_];
        double limitMin[numAxes_];
        double limitMax[numAxes_];
        bool reset;
    };

    std::map<TVirtualPad *, AxisInfo> zoomInfo_;
};

#endif //RootInterface_H
