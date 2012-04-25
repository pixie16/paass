/** \file DetectorLibrary.hpp
 *
 * This is an extension of the vector of the identifiers that was previously
 *   used.
 */

#ifndef __DETECTORLIBRARY_HPP_
#define __DETECTORLIBRARY_HPP_

#include <map>
#include <set>
#include <string>
#include <vector>

// forward declaration, perhaps just move this class info here
class Identifier; 

class DetectorLibrary : public std::vector<Identifier>
{
public:
    DetectorLibrary();
    
    virtual const_reference at(size_type mod, size_type ch) const;
    virtual const_reference at(size_type idx) const;
    virtual reference at(size_type mod, size_type ch);
    virtual reference at(size_type idx);

    virtual void push_back(const Identifier &x);
    virtual ~DetectorLibrary();

    int GetNextLocation(const std::string &type, 
			const std::string &subtype) const;
    size_type GetIndex(int mod, int chan) const; 
    unsigned int GetPhysicalModules() const {return numPhysicalModules;}
    unsigned int GetModules() const {return numModules;}
    bool HasValue(int mod, int chan) const;
    void Set(int index, const Identifier& value);
    void Set(int mod, int ch, const Identifier& value);

    void PrintMap(void) const;
    void PrintUsedDetectors(void) const;

    const std::set<std::string>& GetKnownDetectors(void); 
    const std::set<std::string>& GetUsedDetectors(void) const;
private:
    std::map<std::string, int> highLocation; ///< largest location defined for a given type and subtype
    
    unsigned int numModules;
    unsigned int numPhysicalModules;
    
    std::set<std::string> usedTypes;
    std::set<std::string> usedSubtypes;

    std::set<std::string> knownDetectors;
};

#endif // __DETECTORLIBRARY_HPP_
