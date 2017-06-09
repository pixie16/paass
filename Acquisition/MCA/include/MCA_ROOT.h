///@authors K. Smith, C. R. Thornsberry
#ifndef MCA_ROOT_H
#define MCA_ROOT_H

#include "MCA.h"
#include <stdio.h>
#include <unistd.h>

class TFile;

class TH1F;

class MCA_ROOT : public MCA {
private:
    TFile *_file;
    std::map<int, TH1F *> _histograms;

    ///A class to handle redirecting stderr
    /**The class redirects stderr to a text file and also saves output in case the user
     * would like to print it. Upon destruction stderr is restored to stdout.
     */
    class cerr_redirect {
    private:
        char buf[BUFSIZ];

    public:
        cerr_redirect(const char *logFile) {
            for (int i = 0; i < BUFSIZ; i++)
                buf[i] = '\0';
            freopen(logFile, "a", stderr);
            setbuf(stderr, buf);
        };

        /**stderr is pointed to a duplicate of stdout, buffering is then set to no buffering.
         */
        ~cerr_redirect() {
            dup2(fileno(stdout), fileno(stderr));
            setvbuf(stderr, NULL, _IONBF, BUFSIZ);
        };

        void Print() {
            fprintf(stdout, "%s", buf);
            fflush(stdout);
        }

    };

public:
    ///Default constructor
    MCA_ROOT(PixieInterface *pif, const char *basename);

    ///Defaul destructor
    ~MCA_ROOT();

    bool StoreData(int mod, int ch);

    void Flush();

    bool OpenFile(const char *basename);

    TH1F *GetHistogram(int mod, int ch);

    void Reset();

};

#endif
