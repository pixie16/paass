/** \file McpProcessor.hpp
 * \brief Class for handling position-sensitive mcp signals
 * @authors D. Miller
 */
#ifndef __MCPPROCESSOR_HPP_
#define __MCPPROCESSOR_HPP_

#include "EventProcessor.hpp"

//! Class that handles MCP detectors
class McpProcessor : public EventProcessor {
public:
    /** Default Constructor */
    McpProcessor();

    /** Declare the plots for the analysis */
    void DeclarePlots(void);

    /** Process the event
     * \param [in] rEvent : the event to process
     * \return true if the process was successful */
    bool PreProcess(RawEvent &rEvent);
private:
    static const size_t nPos = 4; //!< number of positions

    /** \brief Data structure to hold MCP data */
    struct McpData {
        double raw[nPos];//!< the raw information
        double xpos;//!< the x position
        double ypos;//!< the y position
        int mult;//!<  the multiplicity

        void Clear(void);//!< clears the data map
    } data; //!< instance of the McpData structure
};

#endif // __MCPPROCESSOR_HPP_
