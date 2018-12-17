#ifndef GSAddback_h
#define GSAddback_h

class GSAddback {
public:
    /** Default constructor setting things to zero */
    GSAddback() {
        energy = time = ftime = 0;
        multiplicity = 0;

    }
    /** Default deconstructor */

    ~GSAddback(){};

    /** Default constructor setting default values
     * \param [in] ienergy : the initial energy
     * \param [in] itime : the time
     * \param [in] ftime : the initial time
     * \param [in] imultiplicity : multiplicity of the event*/
    GSAddback(double ienergy, double itime, double iftime, unsigned imultiplicity) {
        energy = ienergy;
        time = itime;
        multiplicity = imultiplicity;
        ftime = iftime;
    }

    double energy;//!< Energy of the addback event
    double time;//!< time of the addback event
    unsigned multiplicity;//!< multiplicity of the event
    double ftime;//<! first time in the event
};

#endif //end include guard
