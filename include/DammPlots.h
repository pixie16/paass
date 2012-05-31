#ifndef DAMMPLOTS_H
#define DAMMPLOTS_H

#include <string>
#include <iostream>
#include <map>
#include <sstream>
#include "PlotsRegister.h"

using namespace std;

/* Forward declaration */
class Plots;

/** Damm plot */
class DammPlot {
    public:
        /** Returns id */
        int getId() const {
            return id_;
        }

        void plot(double val1, double val2 = -1, double val3 = -1, const char* name="h");

    private:
        /** Plot id (sould be unique) */
        const int id_;

        /** Plot number of dimensions */
        const unsigned short dim_;

        /** Only class Plots will have access to constructors,
         * user should not be allowed to create his own histograms 
         * outside of Plots class. */
        friend class Plots;

        /** Constructors based on previously used DeclareHistogram functions. */
        DammPlot(int dammId, int xSize, const char* title,
                 int halfWordsPerChan, int xHistLength,
                 int xLow, int xHigh);

        DammPlot(int dammId, int xSize, const char* title,
                 int halfWordsPerChan = 2) ;

        DammPlot(int dammId, int xSize, const char* title,
                 int halfWordsPerChan,
                 int contraction);

        DammPlot(int dammId, int xSize, int ySize,
                 const char *title, int halfWordsPerChan,
                 int xHistLength, int xLow, int xHigh,
                 int yHistLength, int yLow, 
                 int yHigh);

        DammPlot(int dammId, int xSize, int ySize,
                 const char* title,
                 int halfWordsPerChan = 1);

        DammPlot(int dammId, int xSize, int ySize,
                 const char* title, int halfWordsPerChan,
                 int xContraction, int yContraction);

};


/** Holds pointers to all Histograms.*/
// Formerly singleton class
class Plots {
    public:
        Plots (int offset, int range, PlotsRegister* reg) {
            reg->add(offset, range);

            offset_ = offset;
            range_ = range;
        }

        bool checkRange (int id) {
            if (id < range_ && id >= 0)
                return true;
            else 
                return false;
        }

        /** Checks if id is taken */
        bool exists (int id) {
            if (idList_.count(id) == 0)
                return false;
            else
                return true;
        }

        /** Check if mnemonic mne is taken */
        bool exists (const string& mne) {
            // Empty mnemonic is always allowed
            if (mne.size() == 0)
                return false;

            if (mneList_.count(mne) == 0)
                return false;
            else
                return true;
        }

        /** Here go C++ wrappers for histograms calls */
        bool DeclareHistogram1D(int dammId, int xSize, const char* title,
                    int halfWordsPerChan, int xHistLength, int xLow, int xHigh, string mne = "") {
            if ( !exists(dammId) && !exists(mne) && checkRange(dammId) ) {
                idList_.insert( pair<int, DammPlot*>(dammId, new DammPlot(dammId + offset_, xSize, title,
                                                     halfWordsPerChan, xHistLength, xLow, xHigh)) );

                // Mnemonic is optional and added only if longer then 0
                if (mne.size() > 0)
                    mneList_.insert( pair<string, int>(mne, dammId) );
                return true;
            } else {
                if (checkRange(dammId))
                    cerr << "Histogram " << dammId << ", " << mne << " already exists." << endl; 
                else
                    cerr << "Id : " << dammId << " is outside of allowed range (" << range_ << ")." << endl;
                exit(1);
                return false;
            }
        }

        bool DeclareHistogram1D(int dammId, int xSize, const char* title, string mne = "") {
            if ( !exists(dammId) && !exists(mne) && checkRange(dammId) ) {
                idList_.insert( pair<int, DammPlot*>(dammId, new DammPlot(dammId + offset_, xSize, title)) );
                // Mnemonic is optional and added only if longer then 0
                if (mne.size() > 0)
                    mneList_.insert( pair<string, int>(mne, dammId) );
                return true;
            } else {
                if (checkRange(dammId) )
                    cerr << "Histogram " << dammId << ", " << mne << " already exists." << endl; 
                else
                    cerr << "Id : " << dammId << " is outside of allowed range (" << range_ << ")." << endl;
                exit(1);
                return false;
            }
        }
        
        bool DeclareHistogram1D(int dammId, int xSize, const char* title,
                    int halfWordsPerChan, int contraction, string mne = "") {
            if ( !exists(dammId) && !exists(mne) && checkRange(dammId) ) {
                idList_.insert( pair<int, DammPlot*>(dammId, new DammPlot(dammId + offset_, xSize, title, halfWordsPerChan, contraction)) );
                // Mnemonic is optional and added only if longer then 0
                if (mne.size() > 0)
                    mneList_.insert( pair<string, int>(mne, dammId) );
                return true;
            } else {
                if (checkRange(dammId) )
                    cerr << "Histogram " << dammId << ", " << mne << " already exists." << endl; 
                else
                    cerr << "Id : " << dammId << " is outside of allowed range (" << range_ << ")." << endl;
                exit(1);
                return false;
            }
        }
        bool DeclareHistogram2D(int dammId, int xSize, int ySize,
                                const char *title, int halfWordsPerChan,
                                int xHistLength, int xLow, int xHigh,
                                int yHistLength, int yLow, int yHigh, string mne = "") {
            if ( !exists(dammId) && !exists(mne) && checkRange(dammId) ) {
                idList_.insert( pair<int, DammPlot*>(dammId, new DammPlot(dammId + offset_, xSize, ySize,
                                                     title, halfWordsPerChan,
                                                     xHistLength, xLow, xHigh,
                                                     yHistLength, yLow, yHigh)) ); 
                // Mnemonic is optional and added only if longer then 0
                if (mne.size() > 0)
                    mneList_.insert( pair<string, int>(mne, dammId) );
                return true;
            } else {
                if (checkRange(dammId) )
                    cerr << "Histogram " << dammId << ", " << mne << " already exists." << endl; 
                else
                    cerr << "Id : " << dammId << " is outside of allowed range (" << range_ << ")." << endl;
                exit(1);
                return false;
            }
        }
        bool DeclareHistogram2D(int dammId, int xSize, int ySize,
                                const char* title, string mne = "") {
            if ( !exists(dammId) && !exists(mne) && checkRange(dammId) ) {
                idList_.insert( pair<int, DammPlot*>(dammId, new DammPlot(dammId + offset_, xSize, ySize, title)) );
                // Mnemonic is optional and added only if longer then 0
                if (mne.size() > 0)
                    mneList_.insert( pair<string, int>(mne, dammId) );
                return true;
            } else {
                if (checkRange(dammId) )
                    cerr << "Histogram " << dammId << ", " << mne << " already exists." << endl; 
                else
                    cerr << "Id : " << dammId << " is outside of allowed range (" << range_ << ")." << endl;
                exit(1);
                return false;
            }
        }
        bool DeclareHistogram2D(int dammId, int xSize, int ySize,
                    const char* title, int halfWordsPerChan,
                    int xContraction, int yContraction, string mne = "") {
            if ( !exists(dammId) && !exists(mne) && checkRange(dammId) ) {
                idList_.insert( pair<int, DammPlot*>(dammId, new DammPlot(dammId + offset_, xSize, ySize,
                                                                          title, halfWordsPerChan,
                                                                          xContraction, yContraction)) );
                // Mnemonic is optional and added only if longer then 0
                if (mne.size() > 0)
                    mneList_.insert( pair<string, int>(mne, dammId) );
                return true;
            } else {
                if (checkRange(dammId) )
                    cerr << "Histogram " << dammId << ", " << mne << " already exists." << endl; 
                else
                    cerr << "Id : " << dammId << " is outside of allowed range (" << range_ << ")." << endl;
                exit(1);
                return false;
            }
        }

        /** Gives access to Histogram by id.*/
        virtual DammPlot* id(int id) {
            if (exists(id))
                return idList_[id];
            else {
                cerr << "Histogram " << id << " does not exists!" << endl;
                exit(1);
            }
        }

        /** Gives access to Histogram by id.*/
        virtual DammPlot* operator[] (int id) {
            if (exists(id))
                return idList_[id];
            else {
                cerr << "Histogram " << id << " does not exists!" << endl;
                exit(1);
            }
        }

        /** Gives access to Histogram by mnemonic .*/
        virtual DammPlot* mne(string mne) {
            if (exists(mne))
                return idList_[mneList_[mne]];
            else {
                cerr << "Histogram " << mne << " does not exists!" << endl;
                exit(1);
            }
        }

        /** Gives access to Histogram by mnemonic.*/
        virtual DammPlot* operator[] (string mne) {
            if (exists(mne))
                return idList_[mneList_[mne]];
            else {
                cerr << "Histogram " << mne << " does not exists!" << endl;
                exit(1);
            }
        }


        ~Plots () {
            map <int, DammPlot*>::iterator it;
            for (it = idList_.begin(); it != idList_.end(); ++it)
                delete it->second;
        }

    private:
        /** Holds offset for a given set of plots */
        int offset_;
        /** Holds allowed range for a given set of plots*/
        int range_;
        /** Map of int -> Plot */
        map <int, DammPlot*> idList_;
        /** Map of mnemonic -> int (-> Plot) */
        map <string, int> mneList_;
};

#endif
