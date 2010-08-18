/** \file RandomPool.h
 *  \brief Holds a pre-generated pool of random numbers
 *
 *  This generates a pool of random numbers using the Mersenne twister.
 *  DTM - 08-18-2010
 */

#ifndef __RANDOMPOOL_H_
#define __RANDOMPOOL_H_

#include "MersenneTwister.h"

/** 
 *  \brief A random pool of numbers
 */
class RandomPool {
 private:
  static const size_t size = 1000000; ///< default size of the pool

  MTRand generator;     ///< random number generator
  size_t counter;       ///< current random number index
  double numbers[size]; ///< the pool of random numbers
 public:
  RandomPool(void);
  void Generate(void);        
  double Get(double range=1); 
};

#endif // __RANDOMPOOL_H_
