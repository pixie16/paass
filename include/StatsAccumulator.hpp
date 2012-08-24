/** \file StatsAccumulator.hpp
 * \brief A simple class which accumulates data from a vector
 *  for basic statistical analysis.
 *
 * \author David Miller
 * \date August 2009
 */

#ifndef __STATS_ACCUMULATOR_HPP_
#define __STATS_ACCUMULATOR_HPP_

class StatsAccumulator
{
 private:
    double sum;   //< the sum of the values
    double sumSq; //< the sum of the values squared
    int count;    //< the number of values
        
    StatsAccumulator(double s, double ss, int c); // complete specification
 public:
    StatsAccumulator(); // create an empty accumulator
    StatsAccumulator(const StatsAccumulator &x) :
	sum(x.sum), sumSq(x.sumSq), count(x.count) {};
    // must define function here for template
    // this lets it work on ints, doubles, floats, ...
    template<typename T>
	StatsAccumulator(const T& t) :
	sum(t), sumSq(t*t), count(1) {}; //< one element of type T
    
    // virtual function so more complicated accumulators
    //  could be derived from this class as needed
    virtual StatsAccumulator operator+(const StatsAccumulator &x) const;
    const StatsAccumulator& operator=(const StatsAccumulator &x);
    const StatsAccumulator& operator+=(const StatsAccumulator &x)
	{return (this->operator=(*this + x));};

    template<typename T>
	void Accumulate(const T& t)
	{this->operator+=(t);};
    virtual void Clear(void);     //< clear the stored sums
    double GetMean(void) const;   //< get mean
    double GetStdDev(void) const; //< get std dev using *unbiased* estimator
    virtual ~StatsAccumulator();
};

#endif // __STATS_ACCUMULATOR_HPP_
