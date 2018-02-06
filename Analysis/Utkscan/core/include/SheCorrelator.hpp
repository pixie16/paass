/** \file SheCorrelator.hpp
 *
 */

#ifndef __SHECORRELATOR_HPP_
#define __SHECORRELATOR_HPP_

#include <vector>
#include <deque> 
#include <sstream>

///An enumeration of the different super heavy event types
enum SheEventType {
    alpha,
    heavyIon,
    fission,
    lightIon,
    unknown
};

///A class to define information for a super-heavy element event
class SheEvent {
public:
    /** Default Constructor */
    SheEvent();
    /** Constructor taking arguments */
    SheEvent(double energy, double time, int mwpc, 
	     bool has_beam, bool has_veto, bool has_escape,
	     SheEventType type=unknown);
    /** Default destructor */
    ~SheEvent() {}
    
    /** \return true if we had beam */
    bool get_beam() const {return has_beam_;}
    /** \return true if we had a veto */
    bool get_veto() const {return has_veto_;}
    /** \return true if we had an escape */
    bool get_escape() const {return has_escape_;}
    
    /** \return the energy */ 
    double get_energy() const {return(energy_);}
    /** \return the time */
    double get_time() const {return time_;}
    
    /** \return the mwpc */
    int get_mwpc() const {return mwpc_;}
    
    /** \return The type of event we had */
    SheEventType get_type() const {return type_;}
    
    /** \param [in] has_beam : sets if we had beam */
    void set_beam(bool has_beam) {has_beam_ = has_beam;}
    /** \param [in] energy : sets the energy  */
    void set_energy(double energy) {energy_  = energy;}
    /** \param [in] time : sets the time */
    void set_time(double time) {time_ = time;}
    /** \param [in] mwpc : sets if we had an mwpc */
    void set_mwpc(int mwpc) {mwpc_ = mwpc;}
    /** \param [in] has_veto : sets if we had a veto */
    void set_veto(bool has_veto) {has_veto_ = has_veto;}
    /** \param [in] has_escape : sets if we had an escape */
    void set_escape(bool has_escape) {has_escape_ = has_escape;}
    /** \param [in]  type : sets the type of event we had*/
    void set_type(SheEventType type) {type_ = type;}
    
private:
    /** Total (reconstructed) energy, may include escape **/
    double energy_;
    /** Shortest time of all subevents (e.g. back and front) */
    double time_;
    /** Number of MWPC chambers hits */
    int mwpc_;
    /** Veto hit flag **/
    bool has_veto_;
    /** Beam on flag **/
    bool has_beam_;
    /** If reconstructed energy includes escape **/
    bool has_escape_;
    /** Type of event decided by Correlator **/
    SheEventType type_;
};

///Class to handle correlations for super heavy event experiments
class SheCorrelator {
public:
    /** Constructor taking x and y size */ 
    SheCorrelator(int size_x, int size_y);
    /** Default Destructor */
    ~SheCorrelator();
    /** adds an event to the deque */
    bool add_event(SheEvent& event, int x, int y);
    /** provides human readable event info */
    void human_event_info(SheEvent& event, std::stringstream& ss, 
			  double clockStart);
private:
    int size_x_; //!< size in the x direction
    int size_y_; //!< size in the y direction 
    std::deque<SheEvent>** pixels_; //!< deque of the pixels hit
    /** flushes the chain */
    bool flush_chain(int x, int y); 
};
#endif
