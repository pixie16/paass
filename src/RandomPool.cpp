/** \file RandomPool.cpp
 *  \brief Implementation of the random pool of numbers
 *
 *  David Miller, August 2010
 */

#include "RandomPool.hpp"

RandomPool randoms; ///< an ugly global to hold some random numbers

/*! Simple constructor which initializes the generator
 */
RandomPool::RandomPool(void) : generator()
{
  Generate();
}

/*! Generate some random numbers using the Mersenne twister */
void RandomPool::Generate(void)
{
  for (size_t i=0; i < size; i++)
    numbers[i] = generator.rand(1);
  counter = 0;
}

/*! Get a random number in the range [0,range) */
double RandomPool::Get(double range)
{
    double d = numbers[counter++];
    
    if (counter == size)
      counter = 0;

    return d * range;
}
