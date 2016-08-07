/** \file StatsData.hpp
 *  \brief Stores statistics data from the data stream in its original format
 */
#ifndef __STATS_DATA_HPP__
#define __STATS_DATA_HPP__

//! Class to store statistics data from data stream in original format
class StatsData {
public:
    static const pixie::word_t headerLength = 1; //!< the header length

    /** Default Constructor that clears the data structures  */
    StatsData(void);
    /** Default Destructor */
    ~StatsData(){};

    /** Copy the statistics data from the data stream to a memory block,
     * preserving a copy of the old statistics data so that the incremental
     * change can be determined
     * \param [in] buf : the buffer to get the stats from
     * \param [in] vsn : the vsn of the buffer */
    void DoStatisticsBlock(pixie::word_t *buf, int vsn);

    /** \return the most recent statistics live time for a given id
    * \param [in] id : the id you want the time for */
    double GetCurrTime(unsigned int id) const;

    /** \return the change in the number of fast peaks between the two most
    *  recent statistics blocks for a given id
    * \param [in] id : the id you want the different peaks from */
    double GetDiffPeaks(unsigned int id) const;

    /** \return the elapsed live time between the two most recent statistics
    *  blocks for a given channel.
    * \param [in] id : the id you want the time diff for */
    double GetDiffTime(unsigned int id) const;

    /** \return run time from the statistics block for a given module
    * \param [in] mod : the module you want the time for */
    double GetRealTime(unsigned int mod = 0) const;
private:
    static const size_t statSize = N_DSP_PAR - DSP_IO_BORDER; //!< Size of the start
    static const size_t maxVsn = 14; //!< maximum value of the vsn

    double firstTime; /**< Store the time of the first statistics block */
    pixie::word_t oldData[maxVsn][statSize]; /**< Older statistics data to calculate the change in statistics */
    pixie::word_t data[maxVsn][statSize];    /**< Statistics data from each module */
};

#endif
