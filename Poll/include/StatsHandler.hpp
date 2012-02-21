// Small class to handle the statistics information from pixies

#ifndef __STATSHANDLER_HPP_
#define __STATSHANDLER_HPP_

#include <string>
#include <vector>

#include "pixie16app_defs.h"

template<typename T>
class ChannelArray : public std::vector<T>
{
public:
    ChannelArray() : std::vector<T>(NUMBER_OF_CHANNELS, T()) {};
    void assign(const T &t) {
	std::vector<T>::assign(NUMBER_OF_CHANNELS, t);
    }
};

class StatsHandler
{
public:
    StatsHandler(size_t nCards = 1);
    ~StatsHandler();

    void AddEvent(unsigned int mod, unsigned int ch, size_t size);
    void AddTime(double dtime);

    double GetDataRate(size_t mod);
    double GetEventRate(size_t mod);
private:
    typedef ChannelArray<unsigned int> channel_counter_t;
    typedef ChannelArray<double> channel_rates_t;
    
    /** number of events for each channel this tick */
    std::vector<channel_counter_t> nEventsDelta;
    /** total number of events for each channel */
    std::vector<channel_counter_t> nEventsTotal; 
    /** data in bytes this tick per module*/
    std::vector<size_t> dataDelta;
    /** total data in bytes per module*/
    std::vector<size_t> dataTotal;
    /** calculated event rate in Hz for each channel */
    std::vector<channel_rates_t> calcEventRate;
    /** calculated data rate in bytes per second for each module */
    std::vector<size_t> calcDataRate;

    /** time elapsed in seconds */
    double timeElapsed;
    /** total time in seconds */
    double totalTime;
    /** time between data dumps in seconds */
    double dumpTime;

    /** number of cards in the system */
    unsigned int numCards;

    /** place where data is dumped to */
    static const std::string dumpFile;

    void Dump(void);
};

#endif // __STATSHANDLER_HPP_
