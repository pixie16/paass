/** \file PlotsRegister.hpp
 *
 * Plot registration class
 * @authors D. Miller, K. Miernik, S. V. Paulauskas
 */

#ifndef __PLOTSREGISTER_HPP_
#define __PLOTSREGISTER_HPP_

#include <utility>
#include <vector>
#include <string>

//! Holds ranges and offsets of all plots. Singleton class.
class PlotsRegister {
public:
    /** \return pointer to only instance of PlotsRegister*/
    static PlotsRegister *get();

    /** Check if the range provided is legit
    * \param [in] offset : the offset that needs checked
    * \param [in] range : the range that needs checked
    * \return true if the range is legit */
    bool CheckRange(int offset, int range) const;

    /** Add an offset, range, and name to known list
    * \param [in] offset : the offset to add
    * \param [in] range : the associated range to add
    * \param [in] name_ : the name to add
    * \return true if everything is good */
    bool Add(int offset, int range, std::string name_);

    /** Default destructor */
    ~PlotsRegister();

private:
    /** Default constructor */
    PlotsRegister() {};

    PlotsRegister(const PlotsRegister &);//!< Overload of the constructor
    PlotsRegister &operator=(PlotsRegister const &);//!< the copy constructor
    static PlotsRegister *instance;//!< static instance of the class

    std::vector<std::pair<int, int>> reg; //!< Vector of min, max of histogram numbers
};

#endif // __PLOTSREGISTER_HPP_
