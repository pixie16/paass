/* 
 * \file Trace.h
 *
 * A simple class to store the traces.
 * Used instead of a typedef so additional functionality can be added later.
 */

#ifndef __TRACE_H_
#define __TRACE_H_

#include <vector>

class Trace : public std::vector<int>
{
 public:
    // an automatic conversion
    Trace(const std::vector<int> &x) : std::vector<int>(x) {};
};

#endif // __TRACE_H_
