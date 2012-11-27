#ifndef __STATS_DATA_HPP
#define __STATS_DATA_HPP

class StatsData {
private:
    static const size_t statSize = N_DSP_PAR - DSP_IO_BORDER;
    static const size_t maxVsn = 14;

    double firstTime; /**< Store the time of the first statistics block */
    pixie::word_t oldData[maxVsn][statSize]; /**< Older statistics data to calculate the change in statistics */
    pixie::word_t data[maxVsn][statSize];    /**< Statistics data from each module */
public:
    static const pixie::word_t headerLength = 1;

    StatsData(void);
    void DoStatisticsBlock(pixie::word_t *buf, int vsn);

    double GetCurrTime(unsigned int id) const;
    double GetDiffPeaks(unsigned int id) const;
    double GetDiffTime(unsigned int id) const;
    double GetRealTime(unsigned int mod = 0) const;
};

#endif
