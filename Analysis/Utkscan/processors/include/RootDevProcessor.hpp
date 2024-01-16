/**@file RootDevProcessor.hpp
*@brief  Basic ROOT output. Fills a generic struc in the same tree layout as the other processors. It has NO damm output
*@authors T.T. King 
*@date 03/30/2019
*/
#ifndef PAASS_RootDevProcessor_H
#define PAASS_RootDevProcessor_H

#include <string>
#include <vector>

#include "EventProcessor.hpp"
#include "PaassRootStruct.hpp"
#include "RawEvent.hpp"

class RootDevProcessor : public EventProcessor {
   public:
    /**Constructor */
    RootDevProcessor(std::string);

    /** Deconstructor */
    ~RootDevProcessor() = default;

    /** Preprocess the event
    * \param [in] event : the event to preprocess
    * \return true if successful
    */
    virtual bool PreProcess(RawEvent &event);

    /** Process the event
     * \param [in] event : the event to process
     * \return true if successful
     */
    virtual bool Process(RawEvent &event);

   private:
    processor_struct::ROOTDEV RDstruct;  
    std::vector<std::string> typestoread;
    std::string Rev;
};

#endif  //PAASS_RootDevProcessor_H
