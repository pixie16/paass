/** \file McpProcessor.h
 * 
 * Class for handling position-sensitive mcp signals
 */

#ifndef __MCPPROCESSOR_H_
#define __MCPPROCESSOR_H_

#include "EventProcessor.h"

class McpProcessor : public EventProcessor
{
 public:
  McpProcessor(); // no virtual c'tors
  virtual void DeclarePlots(void);
  virtual bool Process(RawEvent &rEvent);
  
#ifdef useroot
  virtual bool AddBranch(TTree *tree);
  virtual void FillBranch(void);
#endif
  
 private:
  static const size_t nPos = 4;

  struct McpData {
    double raw[nPos];
    double xpos;
    double ypos;
    int mult;
    
    void Clear(void);
  } data;
};

#endif // __MCPPROCESSOR_H_
