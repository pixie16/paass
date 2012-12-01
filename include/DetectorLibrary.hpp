/** \file DetectorLibrary.hpp
 * \brief An extension of the vector of the identifiers previously used.
 */

#ifndef __DETECTORLIBRARY_HPP_
#define __DETECTORLIBRARY_HPP_

#include <map>
#include <set>
#include <string>
#include <vector>

#include "ChanIdentifier.hpp"
#include "RawEvent.hpp"

class DetectorLibrary : public std::vector<Identifier>
{
public:
    static DetectorLibrary* get();
    
    virtual const_reference at(size_type mod, size_type ch) const;
    virtual const_reference at(size_type idx) const;
    virtual reference at(size_type mod, size_type ch);
    virtual reference at(size_type idx);

    virtual void push_back(const Identifier &x);
    virtual ~DetectorLibrary();

    const std::set<int> &GetLocations(const Identifier &id) const;
    const std::set<int> &GetLocations(const std::string &type, const std::string &subtype) const;
    int GetNextLocation(const Identifier &id) const;
    int GetNextLocation(const std::string &type, 
			const std::string &subtype) const;
    size_type GetIndex(int mod, int chan) const;
    int ModuleFromIndex(int index) const;
    int ChannelFromIndex(int index) const;

    unsigned int GetPhysicalModules() const {return numPhysicalModules;}
    unsigned int GetModules() const {return numModules;}
    bool HasValue(int mod, int chan) const;
    bool HasValue(int index) const;
    void Set(int index, const Identifier& value);
    void Set(int mod, int ch, const Identifier& value);

    void PrintMap(void) const;
    void PrintUsedDetectors(RawEvent& rawev) const;

    const std::set<std::string>& GetKnownDetectors(void); 
    const std::set<std::string>& GetUsedDetectors(void) const;

    typedef std::string mapkey_t;
private:
    DetectorLibrary();
    DetectorLibrary (const DetectorLibrary&);
    DetectorLibrary& operator= (DetectorLibrary const&);
    static DetectorLibrary* instance;

    mapkey_t MakeKey( const std::string &type, const std::string &subtype ) const;

    std::map< mapkey_t, std::set<int> > locations; ///< collection of all used locations for a given type and subtype
    static std::set<int> emptyLocations; ///< dummy locations to return when map key does not exist

    unsigned int numModules;
    unsigned int numPhysicalModules;
    
    std::set<std::string> usedTypes;
    std::set<std::string> usedSubtypes;

    std::set<std::string> knownDetectors;
};

#endif // __DETECTORLIBRARY_HPP_
