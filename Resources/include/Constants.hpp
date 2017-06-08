///@file Constants.hpp
///@brief A header that contains various useful constants for the software.
///@author S. V. Paulauskas
///@date February 09, 2017

#ifndef PIXIESUITE_CONSTANTS_HPP
#define PIXIESUITE_CONSTANTS_HPP

///Namespace containing information about Pixie16 modules.
namespace Pixie16 {
    ///This is the maximum number of channels that a Pixie-16 module can have.
    static const unsigned int maximumNumberOfChannels = 16;
    static const unsigned int maximumNumberOfModulesPerCrate = 13;
}

///Namespace containing physics constants that can be used during the analysis.
namespace Physical {
    static const double neutronMassInMeVcc = 939.565560;
    static const double speedOfLightInCmPerNs = 29.9792458;
}

///Namespace containing information relevant for VANDLE detectors. These
/// values are generally values that the users should not be editing, i.e.
/// the length of a medium bar does not change.
namespace Vandle {
    static const double lengthOfSmallBarInCm = 60;
    static const double lengthOfMediumBarInCm = 120;
    static const double lengthOfBigBarInCm = 200;
}

#endif //PIXIESUITE_CONSTANTS_HPP
