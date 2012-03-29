/** 
 * Handle QDCs for the position sensitive detector
 */

#ifndef __QDCPROCESSOR_HH_
#define __QDCPROCESSOR_HH_

#include "EventProcessor.h"
class QdcProcessor : public EventProcessor
{
public:
    QdcProcessor(); // no virtual c'tors
    virtual bool Process(RawEvent &event);
    virtual void DeclarePlots(void) const;
};
    
#endif // __QDCPROCESSOR_HH_
