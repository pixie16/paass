/** \file IonChamberProcessor.hpp
 * \brief Processor for ion chamber
 * @authors D. Miller
 */
#ifndef __IONCHAMBERPROCESSOR_HPP_
#define __IONCHAMBERPROCESSOR_HPP_

#include <deque>

#include "EventProcessor.hpp"

//! Processor to handle ion chambers
class IonChamberProcessor : public EventProcessor {
private:
    static const size_t noDets = 6;//!< Number of detectors
    static const size_t timesToKeep = 1000;//!< Number of times to keep

    //! A Data structure to handle all of the info for the chamber
    struct Data {
        double raw[noDets];//!< raw energies
        double cal[noDets];//!< calibrated energies
        int mult;//!< multiplicity

        /** Clears the data structure */
        void Clear(void);
    } data;//!< Creates instance of the data structure for storage

    double lastTime[noDets];//!< The last time in the chamber
    std::deque<double> timeDiffs[noDets];//!< Time difference between events
public:
    /** Default Constructor */
    IonChamberProcessor();

    /** Default Destructor */
    ~IonChamberProcessor() {};

    /** Process an event
    * \param [in] event : the event to process
    * \return true if the processing was successful */
    virtual bool Process(RawEvent &event);

    /** Declare plots for processor */
    virtual void DeclarePlots(void);

#ifdef useroot

    /** Add the branch to the tree
    * \param [in] tree : the tree to add the branch to
    * \return true if you could do it */
    bool AddBranch(TTree *tree);

    /** Fill the branch */
    void FillBranch(void);

#endif
};

#endif // __IONCHAMBERPROCSSEOR_HPP_
