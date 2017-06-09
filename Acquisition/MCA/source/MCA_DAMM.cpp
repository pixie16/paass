///@authors K. Smith, C. R. Thornsberry
#include "MCA_DAMM.h"

#include <iostream>

#include "HisDrr.h"
#include "Display.h"

MCA_DAMM::MCA_DAMM(PixieInterface *pif, const char *basename) :
        MCA(pif) {
    OpenFile(basename);
}

MCA_DAMM::~MCA_DAMM() {
    delete _histogram;
}

bool MCA_DAMM::OpenFile(const char *basename) {
    std::string _basename = basename;
    string input = INSTALL_PREFIX;
    input.append("/share/mca_input.txt");
    string drr = _basename + ".drr";
    string his = _basename + ".his";

    std::string message =
            std::string("Creating new empty DAMM histogram ") + basename +
            std::string(".his");
    Display::LeaderPrint(message);
    _histogram = new HisDrr(drr, his, input);
    cout << Display::OkayStr() << endl;

    return (_isOpen = true);
}

bool MCA_DAMM::StoreData(int mod, int ch) {
    PixieInterface::word_t histo[ADC_SIZE];

    _pif->ReadHistogram(histo, ADC_SIZE, mod, ch);

    int id = (mod + 1) * 100 + ch;

    vector<unsigned int> data;
    data.resize(HIS_SIZE, 0);

    for (size_t i = 0; i < ADC_SIZE; i++) {
        unsigned v = static_cast<unsigned>(i / (ADC_SIZE / HIS_SIZE));
        data[v] += histo[i];
    }
    _histogram->setValue(id, data);

    return true;
}
