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
    Identifier(){Zero();};
    /** Default Destructor */
    ~Identifier(){};
    /** Constructor taking arguments and setting values
     * \param [in] type : the type to set
     * \param [in] subType : the subType to set
     * \param [in] loc : the location to set */
    Identifier(const std::string &type, const std::string &subType,
               const int &loc);

    /** Sets the DAMM ID
     * \param [in] a : the id to set */
    void SetDammID(int a) {dammID = a;};
    /** Sets the type
     * \param [in] a : the type to set */
    void SetType(const std::string &a) {type = a;};
    /** Sets the subtype of the channel
     * \param [in] a : the subtype to set */
    void SetSubtype(const std::string &a) {subtype = a;};
    /** Sets the location
     * \param [in] a : sets the location for the channel */
    void SetLocation(int a) {location = a;};

    int GetDammID() const                 {return dammID;}   /**< \return Get the dammid */
    const std::string& GetType() const    {return type;}     /**< \return Get the detector type */
    const std::string& GetSubtype() const {return subtype;}  /**< \return Get the detector subtype */
    int GetLocation() const               {return location;} /**< \return Get the detector location */

    /** Insert a tag to the Identifier
     * \param [in] s : the name of the tag to insert
     * \param [in] n : the value of the tag to insert */
    void AddTag(const std::string &s, int n) {tag[s] = n;}
    /** Check if an identifier has a tag
     * \param [in] s : the tag to search for
     * \return true if the tag is in the identifier */
    bool HasTag(const std::string &s) const {
        if(tag.count(s) > 0)
            return(true);
        return(false);};
    /** \return Get the requested tag
     * \param [in] s : the name of the tag to get */
    int GetTag(const std::string &s) const;

    /** \return The map with the list of tags */
    std::map<std::string, int> GetTagMap(void) const {return (tag);};

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
        return (type == x.type &&
            subtype == x.subtype &&
            location == x.location);
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
       if (type.compare(x.type) > 0)
           return false;
       else if (type.compare(x.type) < 0)
           return true;
       else {
           if (subtype.compare(x.subtype) > 0)
               return false;
           else if (subtype.compare(x.subtype))
               return true;
           else {
               return (location < x.location);
           }
       }
    }

    /** \return The name of the place associated with the channel */
    std::string GetPlaceName() const {
        std::stringstream ss;
        ss << GetType() << "_" << GetSubtype() << "_" << GetLocation();
        return ss.str();
    }
private:
    std::string type;      /**< Specifies the detector type */
    std::string subtype;   /**< Specifies the detector sub type */
    int dammID;            /**< Damm spectrum number for plotting calibrated energies */
    int location;          /**< Specifies the real world location of the channel.
                                For the DSSD this variable is the strip number */
    std::map<std::string, int> tag;  /**< A list of tags associated with the Identifier */
};
#endif
