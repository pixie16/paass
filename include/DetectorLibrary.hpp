/** \file DetectorLibrary.hpp
 * \brief This is an extension of the vector of the identifiers that was previously
 *   used.
 */
#ifndef __DETECTORLIBRARY_HPP_
#define __DETECTORLIBRARY_HPP_

#include <map>
#include <set>
#include <string>
#include <vector>

#include "Identifier.hpp"
#include "RawEvent.hpp"

///Predefine the RawEvnent class
class RawEvent;

//! A class to define a library of detectors known to the analysis
class DetectorLibrary : public std::vector<Identifier> {
public:
    /** \return the only instance of the class */
    static DetectorLibrary* get();

    /** Defines the at operator for access via module and channel number (const)
     * \param [in] mod : the module
     * \param [in] ch : the channel
     * \return a reference to the DetectorLibrary */
    virtual const_reference at(DetectorLibrary::size_type mod,
                               DetectorLibrary::size_type ch) const;
    /** defines the at operator for using an id (const)
     * \param [in] idx : the id to look for
     * \return a reference to the DetectorLibrary */
    virtual const_reference at(DetectorLibrary::size_type idx) const;

    /** Defines the at operator for access via module and channel number
     * \param [in] mod : the module
     * \param [in] ch : the channel
     * \return a reference to the DetectorLibrary */
     virtual reference at(DetectorLibrary::size_type mod,
                         DetectorLibrary::size_type ch);
    /** defines the at operator for using an id (const)
     * \param [in] idx : the id to look for
     * \return a reference to the DetectorLibrary */
     virtual reference at(DetectorLibrary::size_type idx);

    /** Add an additional Identifier to the library
     * \param [in] x : the Identifier to push back */
    virtual void push_back(const Identifier &x);
    /** Default destructor */
    virtual ~DetectorLibrary();

    /** Get the set of locations for the provided Identifier
     * \param [in] id : The identifier to get the locations for
     * \return the set of locations for a given type, subtype */
    const std::set<int> &GetLocations(const Identifier &id) const;
    /** Get the set of locations for the given type and subtype
     * \param [in] type : the type to look for
     * \param [in] subtype : the subtype to look for
     * \return the set of locations for a given type, subtype */
    const std::set<int> &GetLocations(const std::string &type,
                                      const std::string &subtype) const;
    /** Get the next undefined location of a given Identifier
     * \param [in] id : The Identifier you want the the next location for
     * \return the id for the locaton */
     int GetNextLocation(const Identifier &id) const;
    /** Get the next undefined location of a given type and subtype
     * \param [in] type : the detector type
     * \param [in] subtype : the detector subtype
     * \return the id for the locaton */
    int GetNextLocation(const std::string &type,
			const std::string &subtype) const;
    /** Get the index for a given module and channel
     * \param [in] mod : the module number
     * \param [in] chan : the channel number
     * \return the index for a given module, channel */
    size_type GetIndex(int mod, int chan) const;
    /** Calculate the module number from the index
     * \param [in] index : the index to convert to module number
     * \return the module number */
    int ModuleFromIndex(int index) const;
    /** Calculate channel number from the index
     * \param [in] index : the index to convert
     * \return the channel calculated from the index */
    int ChannelFromIndex(int index) const;

    /** \return the number of physical modules */
    unsigned int GetPhysicalModules() const {return numPhysicalModules;}
    /** \return the number of modules */
    unsigned int GetModules() const {return numModules;}
    /** Check that the detector is in the list and kill if it's not
     * \param [in] mod : the module number to check
     * \param [in] chan : the channel number to check
     * \return true if it has a value at the given mod,chan */
     bool HasValue(int mod, int chan) const;
    /** Check if the Library has a value at a given index
     * \param [in] index : the index to check for
     * \return true if it has a value at the given mod,chan */
    bool HasValue(int index) const;
    /** Check that the detector is in the list and kill if it's not
     * \param [in] index : the index to look for
     * \param [in] value : the value to check for */
     void Set(int index, const Identifier& value);
    /** Check that the detector is in the list and kill if it's not
     * \param [in] mod : the module number to check
     * \param [in] ch : the channel number to check
     * \param [in] value : the value to check for */
    void Set(int mod, int ch, const Identifier& value);

    /** Print out the map */
    void PrintMap(void) const;
    /** Print out the used detectors
     * \param [in] rawev : the raw event to print from */
    void PrintUsedDetectors(RawEvent& rawev) const;

    /**   Retrieves a vector containing all detector types for which an analysis
     * routine has been defined making it possible to declare this detector type
     * in the map.txt file.  The currently known detector types are in detectorString
     * \return Get the detectors known to the analysis */
    const std::set<std::string>& GetKnownDetectors(void);
    /** \return the used detectors */
    const std::set<std::string>& GetUsedDetectors(void) const;

    typedef std::string mapkey_t; //!< typedef for a mapkey

private:
    DetectorLibrary();//!< Default Constructor
    DetectorLibrary (const DetectorLibrary&); //!< Define the constructor with itself
    DetectorLibrary& operator= (DetectorLibrary const&); //!< Define copy constructor
    static DetectorLibrary* instance; //!< the static instance of the class

    /** Load the XML configuration file */
    void LoadXml();

    /** Make a unique map key for the given type,subtype
     * \param [in] type : the type to make a key out of
     * \param [in] subtype : the subtype to make a key out of
     * \return the constructed map key */
    mapkey_t MakeKey(const std::string &type, const std::string &subtype) const;

    std::map< mapkey_t, std::set<int> > locations; ///< collection of all used locations for a given type and subtype
    static std::set<int> emptyLocations; ///< dummy locations to return when map key does not exist

    unsigned int numModules;//!< number of modules
    unsigned int numPhysicalModules; //!< number of physical modules

    std::set<std::string> usedTypes;//!< used types
    std::set<std::string> usedSubtypes; //!< used subtypes
    std::set<std::string> knownDetectors; //!< known detectors in the analysis
};
#endif // __DETECTORLIBRARY_HPP_
