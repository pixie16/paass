///@file ChannelConfiguration.hpp
///@brief Class that holds information about channel specific information. Including things like the trace delay,
///@author S. V. Paulauskas
///@date May 31, 2017
#ifndef PAASS_CHANNELCONFIGURATION_HPP
#define PAASS_CHANNELCONFIGURATION_HPP

#include <iomanip>
#include <set>
#include <string>

/// This class holds all of the information that can be set on a channel by channel basis. Fitting parameters, trace delays,
/// detector type, subtypes, tag lists, etc. This class replaces the Identifier class.
class ChannelConfiguration {
public:
    ChannelConfiguration() {}
    
    ~ChannelConfiguration() {}

    ///Constructor taking arguments and setting values
    ///@param [in] atype : the type to set
    ///@param [in] subType : the subType to set
    ///@param [in] loc : the location to set */
    ChannelConfiguration(const std::string &atype, const std::string &subType, const unsigned int &loc) {
        type_ = atype;
        subtype_ = subType;
        location_ = loc;
    }

    ///Sets the type
    ///@param [in] a : the type to set */
    void SetType(const std::string &a) { type_ = a; }

    ///Sets the subtype of the channel
    ///@param [in] a : the subtype to set */
    void SetSubtype(const std::string &a) { subtype_ = a; }

    ///Sets the location
    ///@param [in] a : sets the location for the channel */
    void SetLocation(const unsigned int &a) { location_ = a; }

    ///@return The value of the private variable type
    const std::string &GetType() const { return type_; }

    ///@return the value of the private variable subtype
    const std::string &GetSubtype() const { return subtype_; }

    ///@return The value of the private variable location
    unsigned int GetLocation() const { return location_; }

    ///Insert a tag to the ChannelConfiguration
    ///@param [in] s : the name of the tag to insert
    ///@param [in] n : the value of the tag to insert */
    void AddTag(const std::string &s) { tags_.insert(s); }

    ///Check if an identifier has a tag
    ///@param [in] s : the tag to search for
    ///@return true if the tag is in the identifier */
    bool HasTag(const std::string &s) const { return tags_.find(s) != tags_.end(); }

    ///@return Get the tag list
    std::set<std::string> GetTags() const { return tags_; }

    ///Zeroes some of the variables in the class. The dammid and detector location variable are reset to -1 and the detector
    /// type and sub type are both reset to "" when an identifier object is zeroed.
    void Zero() {
        location_ = 9999;
        type_ = "";
        subtype_ = "";
        tags_.clear();
    }

    ///Print the headers for the ChannelConfiguration */
    static void PrintHeaders(void) {
        std::cout << std::setw(10) << "Type" << std::setw(10) << "Subtype" << std::setw(4) << "Loc" << std::setw(6)
                  << "DammID" << "    TAGS" << std::endl;
    };

    ///Print all of the info for the ChannelConfiguration */
    void Print(void) const {
        std::cout << std::setw(10) << type_ << std::setw(10) << subtype_ << std::setw(4) << location_ << "    ";
        for (std::set<std::string>::const_iterator it = tags_.begin(); it != tags_.end(); it++) {
            if (it != tags_.begin())
                std::cout << ", ";
            std::cout << *it;
        }
        std::cout << std::endl;
    };

    ///Equality operator for identifier
    ///@param [in] x : the ChannelConfiguration to compare to
    ///@return true if this is equal to x */
    bool operator==(const ChannelConfiguration &x) const {
        return type_ == x.type_ && subtype_ == x.subtype_ && location_ == x.location_;
    }

    ///Not - Equality operator for identifier
    ///@param [in] x : the ChannelConfiguration to compare to
    ///@return true if this is not equal to x */
    bool operator!=(const ChannelConfiguration &x) const {
        return !operator==(x);
    }

    ///Less-then operator needed for map container in WalkCorrector.hpp
    ///@param [in] x : the ChannelConfiguration to compare
    ///@return true if this is less than x */
    bool operator<(const ChannelConfiguration &x) const {
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
    bool operator>(const ChannelConfiguration &rhs) const {
        return !operator<(rhs);
    }

    ///\return The name of the place associated with the channel */
    std::string GetPlaceName() const {
        std::stringstream ss;
        ss << type_ << "_" << subtype_ << "_" << location_;
        return ss.str();
    }

private:
    std::string type_;      /**< Specifies the detector type */
    std::string subtype_;   /**< Specifies the detector sub type */
    unsigned int location_; /**< Specifies the real world location of the channel. */
    std::set<std::string> tags_;  /**< A list of tags associated with the ChannelConfiguration */
};


#endif //PAASS_CHANNELCONFIGURATION_HPP
