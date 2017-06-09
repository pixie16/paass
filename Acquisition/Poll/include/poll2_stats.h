///@authors C. R. Thornsberry, K. Smith
// Small class to handle the statistics information from pixies

#ifndef POLL2_STATS_H
#define POLL2_STATS_H

#define NUM_CHAN_PER_MOD 16

class Client;

class StatsHandler {
public:
    StatsHandler(size_t nCards = 1);

    ~StatsHandler();

    void
    AddEvent(unsigned int mod, unsigned int ch, size_t size, int delta_ = 1);

    bool AddTime(double dtime);

    ///Set the amount of time between scalers dumps in seconds.
    void SetDumpInterval(double interval) { dumpTime = interval; };

    double GetDataRate(size_t mod);

    double GetTotalDataRate();

    double GetEventRate(size_t mod);

    ///Return the total run time.
    double GetTotalTime();

    ///Set the ICR and OCR from the XIA module.
    void
    SetXiaRates(int mod, std::vector <std::pair<double, double>> *xiaRates);

    bool CanSend() { return is_able_to_send; }

    ///Clear the stats.
    void Clear();

    void ClearRates();

    void ClearTotals();

    void Dump();

private:
    Client *client; // UDP client for network access

    /** number of events for each channel this tick */
    unsigned int **nEventsDelta;

    /** total number of events for each channel */
    unsigned int **nEventsTotal;

    /** data in bytes this tick per module*/
    size_t *dataDelta;

    /** total data in bytes per module*/
    size_t *dataTotal;

    /** calculated event rate in Hz for each channel */
    double **calcEventRate;

    double **inputCountRate; ///<The XIA Module input count rate.
    double **outputCountRate; ///<The XIA Module output count rate.

    /** calculated data rate in bytes per second for each module */
    size_t *calcDataRate;

    /** time elapsed in seconds */
    double timeElapsed;

    /** total time in seconds */
    double totalTime;

    /** time between data dumps in seconds */
    double dumpTime;

    /** number of cards in the system */
    unsigned int numCards;

    bool is_able_to_send; /// Is StatsHandler able to send on the network?

};

#endif
