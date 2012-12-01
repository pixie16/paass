/** \file MapFile.hpp
 * \brief An independent way to read in the map file with an updated format
 *   from the InitMap() days
 */

#ifndef __MAPFILE_HPP_
#define __MAPFILE_HPP_

#include <set>
#include <string>
#include <vector>
#include <map>
#include "TreeCorrelator.hpp"

/** 
 * A nice simple class which reads in the map file when declared
 *   and updates the external modchan vector accordingly
 */
class MapFile
{
private:
    void TokenizeString(const std::string &in, std::vector<std::string> &out) const;
    void ProcessTokenList(const std::vector<std::string> &list) const;
    bool HasWildcard(const std::string &str) const;
    void TokenToVector(std::string token, std::vector<int> &list, int number) const;
		      
    bool isRead;
public:
    MapFile(const std::string &filename = "map2.txt");
    static const std::string defaultFile; ///< default map file

    operator bool() {return isRead;} ///< is the map file valid
};

#endif // __MAPFILE_HPP_
