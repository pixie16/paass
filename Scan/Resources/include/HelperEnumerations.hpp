///@file HelperEnumerations.hpp
///@brief Header containing namespaced enumerations that are useful to
/// different parts of the software.
///@author S. V. Paulauskas
///@date December 6, 2016
#ifndef PIXIESUITE_HELPERENUMERATIONS_HPP
#define PIXIESUITE_HELPERENUMERATIONS_HPP

namespace Timing {
    /// An enum listing the known CFD analysis types
    enum CFD_TYPE {
        POLY, XIA, BASIC
    };
    /// An enum listing the known Fitter types
    enum FITTER_TYPE {
        GSL, UNKNOWN
    };
}

namespace DataProcessing {
    ///An enum for the different firmware revisions for the Pixie-16 modules.
    /// * R17562 is valid from 12/15/2010 to 09/26/2011 (this is primarily a
    /// Rev D firmware, i.e. 100 MS/s)
    /// * R20466 is valid from 09/26/2011 to 06/11/2013
    /// * R27361 is valid from 06/11/2013 to 02/15/2014
    /// * R29432 is valid from 02/15/2014 to 07/28/2014
    /// * R30474, R30980, R30981 is valid from 07/28/2014 to 03/08/2016
    /// * R29432 is valid from 03/08/2016
    /// * UNKNOWN is used for unspecified firmware revisions.
    ///These dates do not imply that the particular data set being analyzed was
    /// taken with the expected firmware. These dates are meant only to help
    /// guide the user if they do not know the particular firmware that was used
    /// to obtain their data set.
    enum FIRMWARE {
        R17562, R20466, R27361, R29432, R30474, R30980, R30981, R34688, UNKNOWN
    };

    ///An enumeration that tells how long headers from the XIA List mode data
    /// are depending on the different options that are enabled. The order
    /// that the components are listed is the order that they appear in the
    /// header. For example, HEADER_W_ESUM_ETS is the length of the header
    /// when we have the default 4 words, plus the Energy Sums (4 words) and
    /// the External Time Stamps (2 words).
    enum HEADER_CODES {
        STATS_BLOCK = 1, HEADER = 4, HEADER_W_ETS = 6, HEADER_W_ESUM = 8,
        HEADER_W_ESUM_ETS = 10, HEADER_W_QDC = 12, HEADER_W_QDC_ETS = 14,
        HEADER_W_ESUM_QDC = 16, HEADER_W_ESUM_QDC_ETS = 18
    };
}


#endif //PIXIESUITE_HELPERENUMERATIONS_HPP
