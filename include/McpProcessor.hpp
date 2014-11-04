/** \file McpProcessor.hpp
 * \brief Class for handling position-sensitive mcp signals
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
    virtual void DeclarePlots(void);
    /** Process the event
     * \param [in] rEvent : the event to process
     * \return true if the process was successful */
    virtual bool Process(RawEvent &rEvent);

#ifdef useroot
    /** Add the branch to the tree
    * \param [in] tree : the tree to add the branch to
    * \return true if you could do it *\
    virtual bool AddBranch(TTree *tree);
    /** Fill the branch */
    virtual void FillBranch(void);
#endif

private:
    static const size_t nPos = 4; //!< number of positions

    /** Data structure to hold MCP data */
    struct McpData {
        double raw[nPos];//!< the raw information
        double xpos;//!< the x position
        double ypos;//!< the y position
        int mult;//!<  the multiplicity

        void Clear(void);//!< clears the data map
    } data; //!< instance of the McpData structure
};

#endif // __MCPPROCESSOR_HPP_
