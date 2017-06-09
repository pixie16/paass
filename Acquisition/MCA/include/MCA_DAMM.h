///@author K. Miernik, K. Smith
#ifndef MCA_DAMM_H
#define MCA_DAMM_H

#include "MCA.h"

class HisDrr;

class MCA_DAMM : public MCA {
private:
    HisDrr *_histogram;
public:
    MCA_DAMM(PixieInterface *pif, const char *basename);

    ~MCA_DAMM();

    bool OpenFile(const char *basename);

    bool StoreData(int mod, int ch);

    void Flush() {};
};

#endif
