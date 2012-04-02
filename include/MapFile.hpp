/** \file MapFile.hpp
 *
 * An independent way to read in the map file with an updated format
 *   from the InitMap() days
 */

#ifndef __MAPFILE_HPP_
#define __MAPFILE_HPP_

#include <string>
/** 
 * A nice simple class which reads in the map file when declared
 *   and updates the external modchan vector accordingly
 */
class MapFile
{
public:
    static const std::string defaultFile; //< default map file

    MapFile(void);
    MapFile(const std::string &filename);
};

#endif // __MAPFILE_HPP_
