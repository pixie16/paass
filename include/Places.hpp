/** \file Places.hpp
* \brief Defines the various places for the TreeCorrelator
* \author K. A. Miernik
* \date October 22, 2012
*/
#ifndef __PLACES_HPP__
#define __PLACES_HPP__

#include <deque>
#include <iostream>
#include <vector>
#include <utility>

#include <cstdlib>

#include "Globals.hpp"
#include "EventData.hpp"

/** \brief A pure abstract class to define a "place" for correlator.
 *
 * A place has physical or abstract meaning, might be a detector,
 * a condition or a group of detectors.
 */
class Place {
public:
    /** C'tor. By default the Place is resetable, and internal
     * fifo remembers only current and previous event.
     * \param [in] resetable : if the place resets automatically
     * \param [in] max_size : sets the maximum size of the fifo */
    Place(bool resetable = true, unsigned max_size = 2) {
        resetable_ = resetable;
        max_size_ = max_size;
        status_ = false;
    }
    /** Default Destructor */
    virtual ~Place() {};

    /** Defines 'child' of place. A child will report any
     * changes of its status to parent. Parent will change its
     * status depending on status of children but parent status will
     * not affect status of children.
     * \param [in] child : defines a place as the child of another
     * \param [in] coin : sets if child is coincident or not */
    virtual void addChild(Place* child, bool coin = true);

    /** Checks is child is already listed among children.
     * In case it is, False value is returned.
     * \param [in] child : the child to be checked
     * \return true if not listed */
    virtual bool checkChildren(Place* child);

    /** Checks is child is its own parent or his parent parent (
     * uptree) etc. In such a case False is returned.
     * \param [in] child : the child to check
     * \return true if child is not its own parent */
    virtual bool checkParents(Place* child);

    /** Activates Place and reports to the parent if place was not active,
     * always saves event data to the fifo.
     * \param [in] info : the info to use for the activation */
    virtual void activate(EventData& info) {
        if(!status_) {
            status_ = true;
            add_info_(info);
            report_(info);
        } else {
                add_info_(info);
        }
    }
    /** Simplified activation for counter-like detectors, without
     * need of creating EventData object outside.
     * \param [in] time : the time for the activation */
    virtual void activate(double time) {
        EventData info(time);
        activate(info);
    }

    /** Changes status to false. Only time and status change is
     * recorded in fifo.
     * \param [in] time : the time at which to set the place */
    virtual void deactivate(double time) {
        if(status_) {
                status_ = false;
                EventData info(time, status_);
                add_info_(info);
                report_(info);
            }
        else {
                EventData info(time, status_);
                add_info_(info);
            }
    }
    /** Changes status to false without storing correlation
     * data or reporting to parents. Use only when ending current
     * event. For deactivation occuring due to physical conditions of
     * the system use deactivate() method.*/
    virtual void reset() { status_ = false; };

    /** \return Logical AND operator for two Places.
    * \param [in] right : the place to use for comparison */
    virtual bool operator&& (const Place& right) const {
        return (*this)() && right();
    }

    /** \return Logical OR operator for two Places.
    * \param [in] right : the place to use for comparison */
    virtual bool operator|| (const Place& right) const {
        return (*this)() || right();
    }

    /** \return status of the place.*/
    virtual bool operator()() const {
        return status_;
    }

    /** \return status of the place. Function version of operator ()*/
    virtual bool status() const {
        return (*this)();
    }

    /** Easy access to stored data in fifo, notice at() function used
     * (raises exception).
     * \param [in] index : the index of the data to get from the fifo
     * \return event data in fifo */
    virtual EventData& operator [](unsigned index) {
        return info_.at(index);
    }

    /** Easy access to stored data in fifo, notice at() function used
     * (raises exception).
     * \param [in] index : the index to get from the fifo
     * \return data in fifo */
    virtual EventData operator [](unsigned index) const {
        return info_.at(index);
    }

    /** Easy access to last (current) element of fifo. If fifo
     * is empty time=-1 event is returned
     * \return The last EventData entry in the fifo */
    virtual EventData last() {
        if(info_.size() > 0)
            return info_.back();
        else {
                EventData empty(-1);
                return empty;
            }
    }

    /** Easy access to the second to last element of fifo. If fifo
     * has only one event, time=-1 event is returned.
     * \return Second to last event data */
    virtual EventData secondlast() {
        if(info_.size() > 1) {
                unsigned sz = info_.size();
                return info_.at(sz - 2);
            }
        else {
                EventData empty(-1);
                return empty;
            }
    }

    /** Returns true if place should automatically deactivate
    * after the end of event and false if place should stay
    * in current state as long as deactivating
    * signal is explicitly send.
    * \return true if place is resetable automatically */
    virtual bool resetable() const {
        return resetable_;
    }
    /** Pythonic style private field. Use it if you must,
     * but perhaps you should not. Stores information on past
     * events in a given Place.*/
    std::deque<EventData> info_;

protected:
    /** Pure virutal function. The check function should decide how
     * to change status depending on the status of children. Should be
     * implemented in a derived class.
     * \param [in] info : data to use for the check */
    virtual void check_(EventData& info) = 0;
    /** Fifo (info_) depth */
    unsigned max_size_;

    /** Adds parent to the list. User should be able to call addChild
     * function only. The addParent function should be called by addChild.
     * \param [in] parent : the parent to add
     */
    virtual void addParent_(Place* parent) {
        parents_.push_back(parent);
    }

    /** Reports change of status to parents.
     * Calls check() function for all the parents.
     * \param [in] info : the information to report
     */
    virtual void report_(EventData& info) {
        std::vector<Place*>::iterator it;
        for(it = parents_.begin(); it != parents_.end(); ++it)
            (*it)->check_(info);
    }

    /** Add information to the place
    * \param [in] info : the information to add */
    virtual void add_info_(const EventData& info) {
        info_.push_back(info);
        while(info_.size() > max_size_)
            info_.pop_front();
    }

    /** Status is true if given place is in active state (e.g. detector
     * recorded an event).*/
    bool status_;

    /** Indicates if place should be reseted after end of event (true)
     * or if should persist until status is changed explicitly (false).*/
    bool resetable_;

    /** Vector keeping a list of children on which status of the Place depends.
     * Place* is a pointer to the downstream place, bool describes relation
     * (true for coincidence-like, false for anti-coincidence).
     */
    std::vector< std::pair<Place*, bool> > children_;

    /** Vector keeping a list of parents to whom the change of status
     * should be reported.
     */
    std::vector<Place*> parents_;
};

/** \brief "Lazy" Place does not store multiple activation or deactivation events.
 * Abstract class.*/
class PlaceLazy : public Place {
public:
    /** A constructor for the lazy place
    * \param [in] resetable : sets the place resetable or not
    * \param [in] max_size : the maximum size of the place */
    PlaceLazy(bool resetable = true, unsigned max_size = 2) :
        Place(resetable, max_size) {}

    /** Activates Place and saves event data to deque only if place
     * was not active before.
     * \param [in] info : the information to use to activate the place */
    virtual void activate(EventData& info) {
        if(!status_) {
                status_ = true;
                add_info_(info);
                report_(info);
            }
    }
    /** Changes status to false, time and status change is
     * recorded in fifo only if Place was active before.
     * \param [in] time : the time to deactivate the place */
    virtual void deactivate(double time) {
        if(status_) {
                status_ = false;
                EventData info(time, status_);
                add_info_(info);
                report_(info);
            }
    }
};

/** \brief The basic detector which does not depend on children status. Will use
 *  info_ map to store additional information.*/
class PlaceDetector : public Place {
public:
    /** Constructor for the Place
    * \param [in] resetable : sets the place to be resetable or not
    * \param [in] max_size : sets the maximum size of the place */
    PlaceDetector(bool resetable = true, unsigned max_size = 2) :
        Place(resetable, max_size) {}
protected:
    /** This function is empty here - this place does not depend on children status
     * \param [in] info : is not used for anything */
    virtual void check_(EventData& info){};
};

/** \brief Each activation must be within the set thresholds. */
class PlaceThreshold : public Place {
public:
    /** Low and high threshold limit set in constructor.
    *
    * If both set to 0, no threshold is applied.
    * \param [in] low_limit : the low range for the threshold
    * \param [in] high_limit : the high range for the threshold
    * \param [in] resetable : flag to tell if place is resetable
    * \param [in] max_size : the  maximum size of the place */
    PlaceThreshold(double low_limit, double high_limit, bool resetable = true,
                   unsigned max_size = 2) : Place(resetable, max_size) {
        low_limit_ = low_limit;
        high_limit_ = high_limit;
    }

    /** Activate place only if energy is within set limits or both limits are 0 (no threshold).
     * \param [in] info : the event information to use to activate */
    void activate(EventData& info) {
        if(low_limit_ == 0 && high_limit_ == 0)
            Place::activate(info);
        else if(info.energy > low_limit_ && info.energy < high_limit_)
            Place::activate(info);
    }

    /** \return the low limit for the threshold */
    double getLowLimit() {return low_limit_; };

    /** \return the high limit for the threshold */
    double getHighLimit() { return high_limit_; };

protected:
    /** \brief Does not depend on children.
    * If you need some behaviour derive a new class from this one.
    * \param [in] info : the information to use for the threshold check */
    virtual void check_(EventData& info){};

    double low_limit_;//!< low limit for the threshold
    double high_limit_;//!< high limit for the threshold
};

/** \brief Each activation must be within the set thresholds. */
class PlaceThresholdOR : public PlaceThreshold {
public:
    /** Constructor that takes parameters
    *
    * If both set to 0, no threshold is applied.
    * \param [in] low_limit : the low range for the threshold
    * \param [in] high_limit : the high range for the threshold
    * \param [in] resetable : flag to tell if place is resetable
    * \param [in] max_size : the  maximum size of the place */
    PlaceThresholdOR(double low_limit, double high_limit, bool resetable = true,
                     unsigned max_size = 2)
        : PlaceThreshold(low_limit, high_limit, resetable, max_size) {
    };

protected:
    /** \brief Checks if the OR should be activated or not
    * \param [in] info : the information to use for the threshold check */
    virtual void check_(EventData& info);
};

//!Counts number of activations coming from directly or from children.
class PlaceCounter : public Place {
public:
    /** Low and high threshold limit. If both set to 0, no threshold is applied.
    * \param [in] resetable : sets the place resetable or not
    * \param [in] max_size : sets the maximum size of the place */
    PlaceCounter(bool resetable = true, unsigned max_size = 2)
        : Place(resetable, max_size) {
        counter_ = 0;
    }

    /** Activate the place
    * \param [in] info : data to activate the place with */
    void activate(EventData& info) {
        ++counter_;
        Place::activate(info);
    }

    /** Deactive the place
    * \param [in] time : the time to deactivate */
    void deactivate(double time) {
        --counter_;
        Place::deactivate(time);
    }

    /** Resets the place */
    void reset() {
        Place::reset();
        counter_ = 0;
    }

    /** \return the counter for the number of activations */
    virtual int getCounter() const {
        return counter_;
    }

protected:
    int counter_;//!< The counter for the place activation

    /** \brief Checks to see if a place is active
    * \param [in] info : the information to use for the threshold check */
    virtual void check_(EventData& info);
};

/** \brief An abstract place using OR logic to set the activation of places
 *
 * This Place is an abstract place, which status depends on
 * children in OR manner i.e if any of children is activated, the place
 * is activated too.
 * Example: Beta place - if any of physical beta detectors was
 * activated, the place Beta is also active indicating that there was beta
 * registered in the system).*/
class PlaceOR : public Place {
public:
    /** Constructor taking options
    * \param [in] resetable : whether or not the place is resetable
    * \param [in] max_size : the maximum size of the place */
    PlaceOR(bool resetable = true, unsigned max_size = 2) :
        Place(resetable, max_size) {}
protected:
    /** Check to see if we should activate anybody
    *
    * Take first child to get initial state. Then browse through other children,
    * if sum is true break (OR will be true anyway)
    * In order to check childer status we must take into account
    * relation (true for coincidence, false for anticoincidence)
    * Thus we check (children_[i].first->status() == children_[i].second)
    * where first is pointer to child Place, second is bool (relation)
    * \param [in] info : the event data to check for an activation*/
    virtual void check_(EventData& info);
};

/** \brief Similar to PlaceOR but uses AND relation.
 *
 * That makes it a suitable choice for coincidences and anti-coincidences
 * (use relation parameter when adding a child).
 * Example : GammaBeta place storing beta-gamma coincidence,
 * active when both Beta and Gamma children are active.
 * */
class PlaceAND : public Place {
public:
    /** Constructor taking options
    * \param [in] resetable : whether or not the place is resetable
    * \param [in] max_size : the maximum size of the place */
    PlaceAND(bool resetable = true, unsigned max_size = 2) :
        Place(resetable, max_size) {}
protected:
    /** Check the event for the AND
    *
    * Take first child to get initial state. Browse through other children
    * if result is false break (AND will be false anyway)
    * \param [in] info : the event data to search through */
    virtual void check_(EventData& info);
};
#endif
