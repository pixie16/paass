/** \file Identifier.hpp
 * \brief Defines identifying information for channels
*/
#ifndef __CHANIDENTIFIER_HPP
#define __CHANIDENTIFIER_HPP

#include <map>
#include <sstream>
#include <string>

/** \brief Channel identification
 *
 * All parameters needed to uniquely specify the detector connected to a
 * specific channel are set here.  A vector of identifiers for all channels is
 * created on initialization in InitID().  Each event that is created in
 * ScanList() in PixieStd.cpp has its channel identifier attached to it in
 * the variable chanID inside RawEvent
 *
 * Identifier is a class that will contain basic channel information that
 * will not change including the damm spectrum number where the raw energies
 * will be plotted, the detector type and subtype, and the detector's physical
 * location (strip number, detector location, ...)  */
class Identifier {
public:
    /** Default constructor
    *
    * The dammid and detector location variable are set to -1
    * and the detector type and sub type are both set to ""
    * when an identifier object is created. */
    Identifier() { Zero(); };

    /** Default Destructor */
    ~Identifier() {};

    /** Constructor taking arguments and setting values
     * \param [in] atype : the type to set
     * \param [in] subType : the subType to set
     * \param [in] loc : the location to set */
    Identifier(const std::string &atype, const std::string &subType,
               const unsigned int &loc) {
        type_ = atype;
        subtype_ = subType;
        location_ = loc;
    }

    /** Sets the DAMM ID
     * \param [in] a : the id to set */
    void SetDammID(unsigned int &a) { dammID_ = a; }

    /** Sets the type
     * \param [in] a : the type to set */
    void SetType(const std::string &a) { type_ = a; }

    /** Sets the subtype of the channel
     * \param [in] a : the subtype to set */
    void SetSubtype(const std::string &a) { subtype_ = a; }

    /** Sets the location
     * \param [in] a : sets the location for the channel */
    void SetLocation(const unsigned int &a) { location_ = a; }

    ///@return The value of the private variable dammID
    unsigned int GetDammID() const { return dammID_; }

    ///@return The value of the private variable type
    const std::string &GetType() const { return type_; }

    ///@return the value of the private variable subtype
    const std::string &GetSubtype() const { return subtype_; }

    ///@return The value of the private variable location
    unsigned int GetLocation() const { return location_; }

    /** Insert a tag to the Identifier
     * \param [in] s : the name of the tag to insert
     * \param [in] n : the value of the tag to insert */
    void AddTag(const std::string &s, int n) { tag_[s] = n; }

    /** Check if an identifier has a tag
     * \param [in] s : the tag to search for
     * \return true if the tag is in the identifier */
    bool HasTag(const std::string &s) const {
        if (tag_.count(s) > 0)
            return (true);
        return (false);
    }

    /** \return Get the requested tag
     * \param [in] s : the name of the tag to get */
    int GetTag(const std::string &s) const;

    /** \return The map with the list of tags */
    std::map<std::string, int> GetTagMap(void) const { return tag_; }

    /** Zeroes an identifier
    *
    * The dammid and detector location variable are reset to -1
    * and the detector type and sub type are both reset to ""
    * when an identifier object is zeroed. */
    void Zero();

    /** Print the headers for the Identifier */
    static void PrintHeaders(void);

    /** Print all of the info for the Identifier */
    void Print(void) const;

    /** Equality operator for identifier
     * \param [in] x : the Identifier to compare to
     * \return true if this is equal to x */
    bool operator==(const Identifier &x) const {
        return (type_ == x.type_ &&
                subtype_ == x.subtype_ &&
                location_ == x.location_);
    }

    /** Not - Equality operator for identifier
     * \param [in] x : the Identifier to compare to
     * \return true if this is not equal to x */
    bool operator!=(const Identifier &x) const {
        return !operator==(x);
    }

    /** Less-then operator needed for map container in WalkCorrector.hpp
     * \param [in] x : the Identifier to compare
     * \return true if this is less than x */
    bool operator<(const Identifier &x) const {
        if (type_.compare(x.type_) > 0)
            return false;
        else if (type_.compare(x.type_) < 0)
            return true;
        else {
            if (subtype_.compare(x.subtype_) > 0)
                return false;
            else if (subtype_.compare(x.subtype_))
                return true;
            else {
                return (location_ < x.location_);
            }
        }
    }

    ///@param[in] rhs : The right hand side that we are comparing with.
    ///@return The negative of the less than operator.
    bool operator>(const Identifier &rhs) const {
        return !operator<(rhs);
    }

    /** \return The name of the place associated with the channel */
    std::string GetPlaceName() const {
        std::stringstream ss;
        ss << type_ << "_" << subtype_ << "_" << location_;
        return ss.str();
    }

private:
    std::string type_;      /**< Specifies the detector type */
    std::string subtype_;   /**< Specifies the detector sub type */
    unsigned int dammID_; /**< Damm spectrum number for plotting
 * calibrated energies */
    unsigned int location_; /**< Specifies the real world location of the
 * channel.
                                For the DSSD this variable is the strip number */
    std::map<std::string, int> tag_;  /**< A list of tags associated with the
 * Identifier */
};

#endif
