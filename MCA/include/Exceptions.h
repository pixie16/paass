/*
 * Copyright Krzysztof Miernik 2012
 * k.a.miernik@gmail.com 
 *
 * Distributed under GNU General Public Licence v3
 */

#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <string>
#include <iostream>

/**
* Exception handler.
* Basic class for error handling.
* It consist of only one field (message) created with inlined
* constructor.
*/
class GenError {
public:
    /** Error message is passed in the ctor.*/
    GenError(const std::string msg = "") : message(msg) {}
    /** Shows error message. */
    std::string show() { return message;}
private:
    /** Error message. */
    const std::string message;
};

/**
* Class for IO error handling.
*/
class IOError : public GenError {
public:
    /** Constructor calling base class constructor in initializer list */
    IOError(const std::string msg = 0) : GenError(msg) {}
};

/**
 * Class for errors related to arrays and vectors.
 */
class ArrayError : public GenError {
public:
    /** Constructor calling base class constructor in initializer list */
    ArrayError(const std::string msg = 0) : GenError(msg) {}
};

/**
* General function for requiriment check.
*/
inline void require(bool requirement,
                const std::string& msg = "Requirement failed"){
    if (!requirement) {
        throw GenError(msg);
    }
}


#endif
