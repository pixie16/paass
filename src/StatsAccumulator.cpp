/** \file StatsAccumulator.cpp
 *
 * Implementation for stats accumulator
 * D. Miller, August 2009
 */ 

#include "StatsAccumulator.h"

#include <cmath>

using std::sqrt;

/** Basic constructor */
StatsAccumulator::StatsAccumulator()
{
    Clear();
}

/** Explicit constructor */
StatsAccumulator::StatsAccumulator(double s, double ss, int c) :
    sum(s), sumSq(ss), count(c)
{
}

/** the basic accumulation procedure
 * let optimization determine whether to inline this
 */
StatsAccumulator StatsAccumulator::operator+
    (const StatsAccumulator& x) const
{
    return StatsAccumulator(sum + x.sum, sumSq + x.sumSq, count + x.count);
}

/** assignment definition */
const StatsAccumulator& StatsAccumulator::operator=(const StatsAccumulator &x)
{
    sum = x.sum;
    sumSq = x.sumSq;
    count = x.count;

    return *this;
}

void StatsAccumulator::Clear()
{
    sum = sumSq = 0.;
    count = 0;
}

double StatsAccumulator::GetMean() const
{
    if (count == 0) 
	return NAN;
    return sum / count;
}

double StatsAccumulator::GetStdDev() const
{
    if (count <= 1)
	return NAN;
    return sqrt( (count * sumSq - sum * sum) / (count * (count - 1)) );
}

/** trivial deconstructor */
StatsAccumulator::~StatsAccumulator()
{
    // do nothing
}
