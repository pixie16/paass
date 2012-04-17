/** \file DetectorLibrary.hpp
 *
 * This is an extension of the vector of the identifiers that was previously
 *   used.
 */

#ifndef __DETECTORLIBRARY_HPP_
#define __DETECTORLIBRARY_HPP_

#include <map>
#include <string>
#include <vector>

// forward declaration, perhaps just move this class info here
class Identifier; 

class DetectorLibrary : public std::vector<Identifier>
{
public:
  DetectorLibrary(void);
  virtual void push_back(const Identifier &x);
  virtual ~DetectorLibrary();

  int GetNextLocation(const std::string &type, 
		      const std::string &subtype) const;
  int GetNumber(int mod, int chan);
private:
  std::map<std::string, int> highLocation; ///< largest location defined for a given type and subtype
  int numModules;
};

#endif // __DETECTORLIBRARY_HPP_
