/** \file RandomPool.cpp
 *  \brief Implementation of the random pool of numbers
 *
 *  \author David Miller
 *  \date August 2010
 */

#include "RandomPool.hpp"

RandomPool* RandomPool::instance = NULL;

RandomPool* RandomPool::get() {
    if (!instance) {
        instance = new RandomPool();
    }
    return instance;
}

RandomPool::RandomPool() : generator() {
  Generate();
}

void RandomPool::Generate(void) {
    for (size_t i = 0; i < size; i++)
        numbers[i] = generator.rand(1);
    counter = 0;
}

double RandomPool::Get(double range) {
    double d = numbers[counter++];

    if (counter == size)
      counter = 0;

    return d * range;
}
