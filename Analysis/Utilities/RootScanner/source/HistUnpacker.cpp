/// @authors K. Smith, S. V. Paulauskas

#include <iostream>
#include <vector>

#include <TTreeFormula.h>
#include <TH2.h>
#include <TError.h>

#include "HistUnpacker.hpp"
#include "RootInterface.hpp"
#include "ScanInterface.hpp"

using namespace std;

HistUnpacker::HistUnpacker() :
        Unpacker(), refreshDelaySec_(2), refreshRequested_(false) {
    //Only output ROOT errors if they are fatal.
    gErrorIgnoreLevel = kFatal;

    file_ = new TFile("histScanner.root", "RECREATE");
    eventData_ = new HistScannerChanData();
    tree_ = new TTree("data", "");
    tree_->Branch("eventData", &eventData_);
}

///Write the tree to to file, close the file and destroy the objects.
HistUnpacker::~HistUnpacker() {
    tree_->ResetBranchAddresses();
    file_->Write();
    file_->Close();
    delete file_;
    delete eventData_;
}

void HistUnpacker::ProcessRawEvent() {
    for (auto evt = rawEvent.begin(); evt < rawEvent.end(); ++evt) {
        if (!running)
            break;
        AddEvent(*evt);
    }
    ProcessEvents();
}

///Fills a HistScannerChanData with values from XiaData and derived ChannelData.
/// The HistScannerChanData is then pushed back onto the eventData_ vector to
/// be process by ProcessEvents.
bool HistUnpacker::AddEvent(XiaData *event) {
    eventData_->Set(event);
    return true;
}

void HistUnpacker::IdleTask() {
    //Process any new histograms the user defined.
    ProcessNewHists();

    //Update the zoom on each pad, in case the user modified the histograms
    for (auto padItr = histos_.begin(); padItr != histos_.end(); ++padItr) {
        TVirtualPad *pad = padItr->first;
        RootInterface::get()->UpdateZoom(pad);
    }

    RootInterface::get()->GetCanvas()->Update();
    RootInterface::get()->IdleTask();
}

///Processes a built event by filling the tree. After a certain number of
/// events the histograms are replotted. This routine clears the event
/// after filling the tree.
bool HistUnpacker::ProcessEvents() {
    //Get the lock for tree access.
    std::unique_lock<std::mutex> treeLock(treeMutex_);
    //Fill the tree with the current event.
    tree_->Fill();

    //We've processed the data so we clear the class for the next data.
    eventData_->Clear();

    //Check that we can make changes to histograms.
    // If not we return to handle more data while other thread is busy.
    std::unique_lock<std::mutex> lock(histMutex_, std::try_to_lock);
    if (!lock.owns_lock())
        return true;

    static std::chrono::duration<float> timeElapsedSec;

    //Only refresh if the delay is greater than 0 or manual refresh requested.
    if (refreshDelaySec_ > 0 || refreshRequested_) {
        timeElapsedSec = std::chrono::duration_cast<std::chrono::duration<float> >(
                std::chrono::system_clock::now() - lastRefresh_);

        if (timeElapsedSec.count() > refreshDelaySec_) {
            refreshRequested_ = false;

            for (auto padItr = histos_.begin(); padItr != histos_.end(); ++padItr) {
                TVirtualPad *pad = padItr->first;
                HistMap_ *map = &padItr->second;
                for (auto itr = map->begin(); itr != map->end(); ++itr)
                    Plot(itr->first, pad);
                RootInterface::get()->UpdateZoom(pad);
            }
            RootInterface::get()->GetCanvas()->Update();
            lastRefresh_ = std::chrono::system_clock::now();
        } else
            IdleTask();
    }
    return true;
}

void HistUnpacker::PlotCommand(const vector<string> &args) {
    if (args.size() == 0 || args.size() > 4) {
        cout << "ERROR: Incorrect syntax for plot command.\n";
        cout << "Usage: plot <mod> <chan> <expr> [pad]\n";
        cout << "       plot <expr> [pad]\n";
        return;
    }

    int mod = -1, chan = -1;
    stringstream arrayIndex;
    if (args.size() > 2) {
        try { mod = stoi(args[0]); }
        catch (const invalid_argument &ia) {
            cout << "ERROR: Invalid module argument: '" << args[0]
                 << "'\n";
        }
        try { chan = stoi(args[1]); }
        catch (const invalid_argument &ia) {
            cout << "ERROR: Invalid channel argument: '" << args[1]
                 << "'\n";
        }
        //Stop if the choice made so far are invalid.
        if (mod < 0 || chan < 0) return;

        arrayIndex << "[" << mod << "][" << chan << "]";
    }

    string expr;
    if (args.size() > 2) expr = args[2];
    else expr = args[0];

    //Test the expression is valid.
    size_t startPos = 0, stopPos = 0;
    //The revised expression permitting modification for specification of mod / chan.
    stringstream revisedExpr;


    //Loop over each argument separated by colons.
    while (stopPos < expr.length()) {
        stopPos = expr.find_first_of(":", startPos);
        if (stopPos == string::npos) stopPos = expr.length();

        //The plotter can only handle 1D and 2D right now.
        if (stopPos != expr.length() &&
            expr.find_first_of(":", stopPos + 1) != string::npos) {
            cout << "ERROR: Only 1D and 2D plots are currently supported.\n";
            //Stop the plot command
            return;
        }

        //Get the subexpression to test
        string split = expr.substr(startPos, stopPos - startPos);
        //Append array indices if module and channel number provided.
        split.append(arrayIndex.str());

        //Test it with a TTreeFormula.
        TTreeFormula formula("test", split.c_str(), tree_);
        if (formula.GetNdim() == 0) {
            cout << "ERROR: Incorrect expr: '" << expr << "', (" << split << ").\n";
            cout << "Check if mod / chan assignment makes sense with expr.\n";
            cout << "Valid branches: ";

            //Print list of valid choices.
            TList *list = (TList *) tree_->GetListOfLeaves();
            for (TObject *obj = list->First(); obj; obj = list->After(obj)) {
                cout << obj->GetName();
                if (obj != list->Last()) cout << ", ";
            }
            cout << "\n";

            //Stop the plot command
            return;
        }

        //Add semicolon between expressions
        if (startPos != 0) revisedExpr << ":";
        //Append the split argument.
        revisedExpr << split;

        //Iterate the start position forward one unit.
        startPos = stopPos + 1;
    }
    expr = revisedExpr.str();

    //Determine the weight for the provided expression
    // If the module and channel number are provided we determine the weight
    // from the multiplicity for that pair.
    stringstream weight;
    if (mod > -1 && chan > -1)
        weight << "mult[" << mod << "][" << chan << "]>0";

    //Determine pad
    TVirtualPad *pad = gPad;

    //Get the string argument provided for the pad
    string padStr = "";
    if (args.size() == 2) padStr = args[1];
    else if (args.size() == 4) padStr = args[3];

    //If a string argument was provided we try to find the pad.
    if (padStr != "") {
        int padIndex = 0;
        try { padIndex = stoi(padStr); }
        catch (const invalid_argument &ia) {
            cout << "ERROR: Invalid pad index: '" << padStr << "'\n";
            return;
        }
        pad = RootInterface::get()->GetCanvas()->GetPad(padIndex);
        if (!pad || pad == 0) {
            cout << "ERROR: Invalid pad index: " << padIndex << ".\n";
            return;
        }
    }

    //Add to the new histogram vector.
    newHists_.push_back(make_pair(make_tuple(expr, weight.str()), pad));
}

void HistUnpacker::ClearCommand(const vector<string> &args) {
    if (args.size() > 1) {
        cout << "ERROR: Incorrect syntax for clear command.\n";
        cout << "Usage: clear [padNum] \n";
        return;
    }

    //Get lock for histograms.
    unique_lock<mutex> lock(histMutex_);

    if (args.empty()) {
        RootInterface::get()->GetCanvas()->Clear();
        for (auto padItr = histos_.begin(); padItr != histos_.end(); ++padItr) {
            HistMap_ *map = &padItr->second;
            for (auto itr = map->begin(); itr != map->end(); ++itr) {
                delete gDirectory->Get(itr->second.c_str());
            }
            if (!map->empty()) map->clear();
            RootInterface::get()->ResetZoom(padItr->first);
        }
    } else {
        //Get the string argument provided for the pad
        string padStr = args[0];

        int padIndex = 0;
        try { padIndex = stoi(padStr); }
        catch (const invalid_argument &ia) {
            cout << "ERROR: Invalid pad index: '" << padStr << "'\n";
            return;
        }

        TVirtualPad *pad = RootInterface::get()->GetCanvas()->GetPad(padIndex);
        if (!pad || pad == 0) {
            cout << "ERROR: Invalid pad index: " << padIndex << ".\n";
            return;
        }

        auto padItr = histos_.find(pad);
        if (padItr != histos_.end()) {
            HistMap_ *map = &padItr->second;
            for (auto itr = map->begin(); itr != map->end(); ++itr)
                delete gDirectory->Get(itr->second.c_str());
            if (!map->empty())
                map->clear();

            RootInterface::get()->ResetZoom(pad);
            pad->Modified();
        }
    }
    RootInterface::get()->GetCanvas()->Update();
}

void HistUnpacker::RefreshCommand(const vector<string> &args) {
    if (args.size() > 1) {
        cout << "ERROR: Incorrect syntax for refresh command.\n";
        cout << "Usage: refresh [delayTimeSec] \n";
        return;
    }
    if (args.empty()) {
        refreshRequested_ = true;
        if (refreshDelaySec_)
            cout << "Refreshing. Time between auto refreshes is "
                 << refreshDelaySec_ << " s.\n";
    } else if (args.size() == 1) {
        float refreshDelaySec = 0;
        try {
            refreshDelaySec = stof(args[0]);
        } catch (const invalid_argument &ia) {
            cout << "ERROR: Invalid refresh delay value: '"
                 << refreshDelaySec << "'\n";
            return;
        }

        if (refreshDelaySec < 0) {
            refreshDelaySec_ = 0;
            cout << "Auto refresh disabled.\n";
        } else {
            refreshDelaySec_ = refreshDelaySec;
            cout << "Auto refresh delay set to " << refreshDelaySec_
                 << " s.\n";
        }
    }
}

void HistUnpacker::ZeroCommand(const vector<string> &args) {
    //Get lock for histograms.
    unique_lock<mutex> treeLock(histMutex_);

    //Get lock for tree.
    unique_lock<mutex> histLock(treeMutex_);

    tree_->Reset();

    for (auto padItr = histos_.begin(); padItr != histos_.end(); ++padItr) {
        HistMap_ *map = &padItr->second;
        for (auto itr = map->begin(); itr != map->end(); ++itr) {
            TH1 *hist = (TH1 *) (gDirectory->Get(itr->second.c_str()));
            if (hist) hist->Reset();
        }
        RootInterface::get()->ResetZoom(padItr->first);
        padItr->first->Modified();
    }

    RootInterface::get()->GetCanvas()->Update();

    return;
}

void HistUnpacker::DivideCommand(const vector<string> &args) {
    if (args.size() == 1) {
        int pads = 0;
        try {
            pads = stoi(args[0]);
        } catch (const invalid_argument &ia) {
            cout << "ERROR: Invalid pad argument.\n";
        }
        if (pads <= 0)
            return;
        //We need to delete all the histos as their associated pads are to be deleted.
        ClearCommand(vector<string>());
        RootInterface::get()->GetCanvas()->DivideSquare(pads);
        return;
    } else if (args.size() == 2) {
        int padsX = 0, padsY = 0;

        try {
            padsX = stoi(args[0]);
        } catch (const invalid_argument &ia) {
            cout << "ERROR: Invalid pad X argument: '" << args[0] << "'\n";
        }

        try {
            padsY = stoi(args[1]);
        } catch (const invalid_argument &ia) {
            cout << "ERROR: Invalid pad Y argument: '" << args[1] << "'\n";
        }

        if (padsX <= 0 || padsY <= 0)
            return;

        //We need to delete all the histos as their associated pads are to be deleted.
        histos_.clear();
        //Clear the canvas
        RootInterface::get()->GetCanvas()->Clear();
        RootInterface::get()->GetCanvas()->Divide(padsX, padsY);
        return;
    } else {
        cout << "ERROR: Incorrect syntax for divide command.\n";
        cout << "Usage: divide <numPads>\n";
        cout << "       divide <numXPads> <numYPads>\n";
        return;
    }
}

///Defines the newly requested histogram's names and pushes them into the 
/// histogram map to be plotted by Plot(). Plot is then called to provide an 
/// immediate histogram.
void HistUnpacker::ProcessNewHists() {
    while (!newHists_.empty()) {
        auto key = newHists_.back().first;
        auto pad = newHists_.back().second;

        auto expr = get<0>(key);
        auto weight = get<1>(key);

        //Define the new histograms name.
        stringstream histName;
        histName << "h_" << expr << "_" << histCount_[key]++;

        //Push the histogram into the map.
        histos_[pad][key] = histName.str();

        //Make the initial plot.
        Plot(key, pad);

        //Update the zoom on the pad for the new histogram.
        RootInterface::get()->UpdateZoom(pad);
        RootInterface::get()->GetCanvas()->Update();

        newHists_.pop_back();
    }
}


///The main plotting method. This method determines arguments to be passed to
/// TTree::Draw() to handle plotting. If a histogram has previously been created
/// the Draw command only adds new entries after that last draw to reduce time
/// required to loop over the tree.
void HistUnpacker::Plot(HistKey_ key, TVirtualPad *pad /*= gPad*/) {
    static const vector<Color_t> colors = {kBlue + 2, kRed, kGreen + 1,
                                           kMagenta + 2};
    string expr = get<0>(key);
    string weight = get<1>(key);

    //Declare string streams used to populate the draw command.
    stringstream drawCmd;
    stringstream drawWeight;
    stringstream drawOpt;
    string histName;
    Color_t drawColor = kBlack;

    //Determine the weight or cut, we only want data from the specific mod/chan.
    drawWeight << weight;//"(" << weight << ")";

    //Find the entry matching the key.
    auto padItr = histos_.find(pad);
    if (padItr == histos_.end()) {
        cout << "ERROR: Unable to locate pad in map!\n";
        return;
    }

    HistMap_ *histMap = &(padItr->second);
    auto histItr = histMap->find(key);

    if (histItr == histMap->end()) {
        cout << "ERROR: Unable to locate key in map!\n";
        return;
    }

    histName = histItr->second;

    //Get pointer to histogram.
    // Cast to TH1 as all higher dimension histogram inherit from TH1.
    TH1 *hist = (TH1 *) (gDirectory->Get(histName.c_str()));

    //If this is not the first plot and the map is not empty we set option "SAME".
    if (histItr != histMap->begin())
        drawOpt << "SAME";

    TVirtualPad *prevPad = gPad;
    pad->cd();

    //Histogram was found so we append to it.
    if (hist) {
        drawCmd << expr << ">>+" << histName;

        if (dynamic_cast<TH2 *>(hist))
            drawOpt << "COLZ";

        tree_->Draw(drawCmd.str().c_str(), drawWeight.str().c_str(), drawOpt.str().c_str(), numeric_limits<Long64_t>::max(),
                    treeEntries_[hist]);

        //Update the number of entries scanned.
        treeEntries_[hist] = tree_->GetEntries();

        pad->Modified();
    } else { //No histogram was found so we need to create one.
        //Get color for this plot.
        unsigned int colorIndex = distance(histMap->begin(), histItr);
        if (colorIndex < colors.size()) drawColor = colors.at(colorIndex);

        //Make an initial draw command.
        drawCmd << expr << ">>" << histName << "(1000)";

        RootInterface::get()->ResetZoom(pad);
        //Attempt to draw and see if we get any counts.
        if (tree_->Draw(drawCmd.str().c_str(), drawWeight.str().c_str(),
                        drawOpt.str().c_str())) {
            //Get a pointer to the created object.
            hist = (TH1 *) gDirectory->Get(histName.c_str());
            if (hist) {
                //Determine the x maximum from the histogram
                double xMax = hist->GetXaxis()->GetXmax();

                //Check if this is a 2D histogram
                if (dynamic_cast<TH2 *>(hist)) {

                    double yMax = hist->GetYaxis()->GetXmax();

                    int xBins = 256, yBins = 256;
                    if (xMax < xBins) xBins = xMax;
                    if (yMax < yBins) yBins = yMax;

                    hist->SetBins(xBins, 0, xMax, yBins, 0, yMax);

                    drawOpt << "COLZ";
                } else { //If a 1D histogram
                    hist->SetBins(xMax, 0, xMax);
                }

                hist->Reset();

                drawCmd.str("");
                drawCmd << expr << ">>" << histName;

                tree_->Draw(drawCmd.str().c_str(), drawWeight.str().c_str(),
                            drawOpt.str().c_str());
                treeEntries_[hist] = tree_->GetEntries();

                hist->SetLineColor(drawColor);

                if (histItr == histMap->begin() && histMap->size() > 1) {
                    for (auto itr = histMap->begin();
                         itr != histMap->end(); ++itr) {
                        TH1 *hist = (TH1 *) (gDirectory->Get(
                                itr->second.c_str()));
                        if (hist) {
                            hist->SetLineColor(colors.at(
                                    distance(histMap->begin(), itr)));
                            if (itr == histMap->begin()) hist->Draw();
                            else hist->Draw("SAME");
                        }
                    }
                }
                RootInterface::get()->GetCanvas()->Update();
            }
        } else {
            //The Draw command didn't add any entries so we delete the hist and try again later.
            hist = (TH1 *) gDirectory->Get(histName.c_str());
            delete hist;
        }
        prevPad->cd();
    }
}
