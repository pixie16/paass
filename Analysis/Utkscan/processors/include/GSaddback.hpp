#ifndef GSAddback_h
#define GSAddback_h

class GSAddback {
public:
    /** Default constructor setting things to zero */
    GSAddback() {
        energy = time = multiplicity = abevtnum=ftime = 0;
    }
    /** Default deconstructor */

    ~GSAddback(){};

    /** Default constructor setting default values
     * \param [in] ienergy : the initial energy
     * \param [in] itime : the initial time
     * \param [in] imultiplicity : multiplicity of the event
     * \param [in] iEvtNum: Pixie event number of the event (comes from DetectorDriver)*/
    GSAddback(double ienergy, double itime, double iftime, unsigned imultiplicity, unsigned long iEvtNum) {
        energy = ienergy;
        time = itime;
        multiplicity = imultiplicity;
        abevtnum= iEvtNum;
        ftime = iftime;
    }

    double energy;//!< Energy of the addback event
    double time;//!< time of the addback event
    unsigned multiplicity;//!< multiplicity of the event
    unsigned long abevtnum;//!<pixie event number of the last det event added to the addback calc
    double ftime;//<! first time in the event
};

#endif //end include guard
