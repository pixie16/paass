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

/** 
 *  \brief A random pool of numbers
 */
class RandomPool {
private:
    RandomPool();
    RandomPool (const RandomPool&);
    RandomPool& operator= (RandomPool const&);
    static RandomPool* instance;

    static const size_t size = 1000000; ///< default size of the pool

    MTRand generator;     ///< random number generator
    size_t counter;       ///< current random number index
    double numbers[size]; ///< the pool of random numbers
public:
    static RandomPool* get();

    void Generate(void);        
    double Get(double range=1); 
};

#endif // __RANDOMPOOL_HPP_
