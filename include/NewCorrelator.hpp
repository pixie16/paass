#ifndef NEWCORRELATOR_H
#define NEWCORRELATOR_H

#include <vector>
#include <iostream>

using namespace std;

/* Forward declaration */
class Child;

/** A pure abstract class to define a "place" for correlator.
 * A place has physical or abstract meaning, might be a detector, 
 * a condition or a group of detectors.
 */
class Place {
    public:
        /** C'tor */
        Place() {
            status_ = false;
        }

        virtual ~Place() {
        }

        /** Defines 'child' of place. A child will report any
         * changes of its status to parent. Parent will change its
         * status depending on status of children but parent status will
         * not affect status of children.
         */
        virtual void addChild (Place* child, bool coin = true);

        /** Unary operator, changes status to true.*/
        virtual void operator+() {
            status_ = true;
            report_();
        }

        /** Function version of operator +*/
        virtual void activate() {
            status_ = true;
            report_();
        }

        /** Unary operator, changes status to false.*/
        virtual void operator-() {
            status_ = false;
            report_();
        }

        /** Function version of operator -*/
        virtual void deactivate() {
            status_ = false;
            report_();
        }

        /** Unary operator, toggles status. */
        virtual void operator~() {
            status_ = !status_;
            report_();
        }

        /** Function version of operator ~*/
        virtual void toggle() {
            status_ = !status_;
            report_();
        }

        /** Logical AND operator. */
        virtual bool operator&& (const Place& right) const {
            return (*this)()&&right();
        }

        /** Logical OR operator. */
        virtual bool operator|| (const Place& right) const {
            return (*this)()||right();
        }

        /** Returns status of place.*/
        virtual bool operator() () const {
            return status_;
        }

        /** Returns status of place. Function version of operator ()*/
        virtual bool status() const {
            return status_;
        }

        /** Pure virutal function. The check function should decide how
         * to change status depending on status of children. Should be 
         * implemented for a given derived place class.
         */
        virtual void check() = 0;

    protected:
        /** Status is true if given place is in active state (e.g. detector
         * recorded an event).*/
        bool status_;

        /** Vector keeping a list of parents to whom the change of status
         * should be reported.
         */
        vector<Place*> parents_;
        /** Vector keeping a list of children on whos status should depend.
         */
        vector<Child> children_;

        /** Adds parent to the list. User should be able to call addChild 
         * function only. The addParent function should be called by addChild.
         */
        virtual void addParent (Place* parent) {
            parents_.push_back(parent);
        }

        /** Reports change of status to parents. 
         * Calls check function for all parents.
         */
        virtual void report_() {
            vector<Place*>::iterator it;
            for (it = parents_.begin(); it != parents_.end(); ++it)
                (*it)->check();
        }
};

/** Class that keeps pointer to place and stores method of coincidence
 * inclusion for parent (true for coin., false for anti-coin.) */
class Child {
    public:
        /** C'tor. */
        Child(Place* p, bool c = true) {
            place = p;
            coin = c;
        }

        /** Returns true if status is true and coincidence was set to true OR when status is false and coincidence set to false. (XNOR gate)*/
        bool getCoin() {
            if ( (coin == true && (*place)() )
               || (coin == false && !(*place)()) )
                    return true;
            else
                    return false; 
        }
        /** Pointer to place. */
        Place* place;

        /** coin (coincidence) : true if child is supposed to be in coincidence,
        * false for anti-coincidence. */
        bool coin;
};

class PlaceBasic : public Place {
    public:
        PlaceBasic() : Place() {} 
    protected:
        virtual void check();
};

class PlaceOR : public Place {
    public:
        PlaceOR() : Place() {}
    protected:
        virtual void check();
};

class PlaceAND : public Place {
    public:
        PlaceAND() : Place() {}
    protected:
        virtual void check();
};

#endif
