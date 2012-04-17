/** \file MapFile.hpp
 *
 * An independent way to read in the map file with an updated format
 *   from the InitMap() days
 */

#ifndef __MAPFILE_HPP_
#define __MAPFILE_HPP_

#include <string>
#include <vector>
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
    void TokenToVector(std::string token, std::vector<int> &list,
		       const std::vector<int> &completeList) const;

    std::vector<int> completeChannelList; ///< holds a list of all the channels in a module
    std::vector<int> completeModuleList;  ///< holds a list of all the modules in the acquisition
public:
    static const std::string defaultFile; //< default map file

    MapFile(void);
    MapFile(const std::string &filename);
};

#endif // __MAPFILE_HPP_
