/** \file PlotsRegister.hpp
 *
 * Plot registration class
 */

#ifndef __PLOTSREGISTER_HPP_
#define __PLOTSREGISTER_HPP_

#include <vector>
#include <utility>


/** Holds ranges and offsets of all plots. Singleton class. */
class PlotsRegister {
    public:
        /** Returns pointer to only instance of PlotsRegister*/
        static PlotsRegister* get();
        
        bool CheckRange (int offset, int range) const;
        bool Add(int offset, int range);

    private:
        PlotsRegister() {};
        PlotsRegister (const PlotsRegister&);
        PlotsRegister& operator= (PlotsRegister const&);
        static PlotsRegister* instance;

        // Vector of min, max of histogram numbers 
        std::vector< std::pair<int, int> > reg;
};

#endif // __PLOTSREGISTER_HPP_
