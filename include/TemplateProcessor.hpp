/** \file TemplateProcessor.hpp
 * \brief A Template class to be used to build others.
 * \author S. V. Paulauskas
 * \date October 26, 2014
 */
#ifndef __TEMPLATEPROCESSOR_HPP_
#define __TEMPLATEPROCESSOR_HPP_

#include "EventProcessor.hpp"

class TemplateProcessor : public EventProcessor {
public:
    TemplateProcessor();
    ~TemplateProcessor(){};
    virtual void DeclarePlots(void);
    virtual bool PreProcesso(RawEvent &event);
    virtual bool Process(RawEvent &event);
private:
    virtual bool RetrieveData(RawEvent &event);
    virtual void AnalyzeData(RawEvent& rawev);
};
#endif // __TEMPLATEPROCESSOR_HPP_
