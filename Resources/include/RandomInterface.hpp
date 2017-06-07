/**@file RandomInterface.hpp
 * @brief Provides a singleton interface for random numbers
 * @authors David Miller and S.V. Paulauskas
 * @date May 2017
 */
#ifndef __RANDOMINTERFACE_HPP_
#define __RANDOMINTERFACE_HPP_

#include <random>

/// An  of numbers using Mersenne twister - Singleton Class
class RandomInterface {
public:
    /** \return The only instance to the random pool */
    static RandomInterface *get();

    /** \return a random number in the specified range [0, range]
    * \param [in] range : the upper bound for the range to get */
    double Generate(const double &range = 1);
private:
    RandomInterface(); //!<Default constructor
    RandomInterface(const RandomInterface &);  //!< Overload of the constructor
    RandomInterface &operator=(RandomInterface const &);//!< the copy constructor
    static RandomInterface *instance;//!< static instance of the class

    std::mt19937_64 engine_;
    std::uniform_real_distribution<double> distribution_;
    unsigned seed_;
};

#endif // __RANDOMINTERFACE_HPP_
