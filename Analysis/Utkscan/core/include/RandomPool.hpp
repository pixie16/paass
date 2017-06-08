/** \file RandomPool.hpp
 * \brief Holds a pre-generated pool of random numbers
 *
 * This generates a pool of random numbers using the Mersenne twister.
 * \author David Miller
 * \date 18 August 2010
 */
#ifndef __RANDOMPOOL_HPP_
#define __RANDOMPOOL_HPP_

#include "MersenneTwister.hpp"

//! A random pool of numbers using Mersenne twister - Singleton Class
class RandomPool {
private:
    RandomPool(); //!<Default constructor
    RandomPool(const RandomPool &);  //!< Overload of the constructor
    RandomPool &operator=(RandomPool const &);//!< the copy constructor
    static RandomPool *instance;//!< static instance of the class

    static const size_t size = 1000000; ///< default size of the pool

    MTRand generator;     ///< random number generator
    size_t counter;       ///< current random number index
    double numbers[size]; ///< the pool of random numbers
public:
    /** \return The only instance to the random pool */
    static RandomPool *get();

    void Generate(void); //!< Generates a random number

    /** \return a random number in the specified range [0, range]
    * \param [in] range : the upper bound for the range to get */
    double Get(double range = 1);
};

#endif // __RANDOMPOOL_HPP_
