///@authors K. Smith, K. Miernik, C. R. Thornsberry
#include <string>
#include <stdio.h>

#include "MCA_ROOT.h"
#include "MCA_DAMM.h"

//printf("Usage: %s <outputType> [duration] [basename]\n",argv[0]);
//printf("\touputType\tCan be either damm or root.\n");
int main(int argc, char *argv[]) {
    int totalTime = 0;
    const char *basename = "MCA";
    bool useRoot = false;

    if (argc >= 2) {
        std::string type = argv[1];
        if (type == "root") useRoot = true;
        else if (type != "damm")totalTime = atoi(argv[1]);
        if (argc >= 3) {
            if (totalTime == 0) totalTime = atoi(argv[2]);
            else basename = argv[2];
            if (argc >= 4)
                basename = argv[3];
        }
    }
    if (totalTime == 0) totalTime = 10;

    PixieInterface pif("pixie.cfg");
    pif.GetSlots();

    pif.Init();

    //cxx, end any ongoing runs
    pif.EndRun();
    pif.Boot(PixieInterface::DownloadParameters |
             PixieInterface::ProgramFPGA |
             PixieInterface::SetDAC, true);

    pif.RemovePresetRunLength(0);

#if defined(USE_ROOT) && defined(USE_DAMM)
    MCA *mca;
    if (useRoot) mca = new MCA_ROOT(&pif, basename);
    else mca = new MCA_DAMM(&pif, basename);
#elif defined(USE_ROOT)
    MCA* mca = new MCA_ROOT(&pif,basename);
#elif defined(USE_DAMM)
    MCA* mca = new MCA_DAMM(&pif,basename);
#endif
    if (mca->IsOpen())
        mca->Run(totalTime);
    delete mca;

    return EXIT_SUCCESS;
}
