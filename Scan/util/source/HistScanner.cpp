#include "HistScanner.hpp"

#include <limits>

#include "TDirectory.h"
#include "TH1F.h"
#include "TColor.h"

#include "HistUnpacker.hpp"

HistScanner::HistScanner() :
	RootScanner()
{
	file_ = new TFile("blah.root","RECREATE");
	eventData_ = new std::vector< HistScannerChanData >;
	tree_ = new TTree("data","");
	tree_->Branch("eventData",&eventData_);
}

HistScanner::~HistScanner() {
	tree_->ResetBranchAddresses();
	file_->Write();
	file_->Close();
	delete file_;
	delete tree_;
	delete eventData_;
}

/** Return a pointer to the Unpacker object to use for data unpacking.
  * If no object has been initialized, create a new one.
  * \return Pointer to an Unpacker object.
  */
Unpacker *HistScanner::GetCore(){ 
	if(!core){ core = new HistUnpacker(); }
	return core;
}

bool HistScanner::AddEvent(XiaData* event) {		
	HistScannerChanData data;
	data.mod = event->modNum;
	data.chan = event->chanNum;
	data.filterEn = event->energy; 

	eventData_->push_back(data);
	return true;
}

void HistScanner::IdleTask() {
	ProcessNewHists();

	RootScanner::IdleTask();
}


bool HistScanner::ProcessEvents() {
	//Fill the tree with the current event.
	tree_->Fill();

	if (tree_->GetEntries() % 100 == 0) {
		for (auto padItr=histos_.begin(); padItr != histos_.end(); ++padItr) {
			TVirtualPad *pad = padItr->first;
			HistMap_ *map = &padItr->second;
			for (auto itr = map->begin(); itr != map->end(); ++itr) {
				auto key = itr->first;
				int mod = std::get<0>(key);
				int chan = std::get<1>(key);
				DataType_ type = std::get<2>(key);
				Plot(mod, chan, type, pad);
			}
			ScaleHistograms(pad);
		}
		GetCanvas()->Update();
	}
	//We've processed the data so we clear the vector for the next data.
	eventData_->clear();

	return true;
}

/** ExtraCommands is used to send command strings to classes derived
  * from ScanInterface. If ScanInterface receives an unrecognized
  * command from the user, it will pass it on to the derived class.
  * \param[in]  cmd_ The command to interpret.
  * \param[out] arg_ Vector or arguments to the user command.
  * \return True if the command was recognized and false otherwise.
  */
bool HistScanner::ExtraCommands(const std::string &cmd, std::vector<std::string> &args){
	if (cmd == "plot") {
		if (args.size() != 3 && args.size() != 4) {
			std::cout << "ERROR: Incorrect syntax for plot command.\n";
			std::cout << "Usage: plot <mod> <chan> <type> [pad]\n";
			return true;
		}
		int mod = -1, chan = -1;
		try { mod = std::stoi(args[0]); }
		catch (const std::invalid_argument& ia) { 
			std::cout << "ERROR: Invalid module argument: '" << args[0] << "'\n"; 
		}
		try { chan = std::stoi(args[1]); }
		catch (const std::invalid_argument& ia) { 
			std::cout << "ERROR: Invalid channel argument: '" << args[1] << "'\n"; 
		}
		DataType_ type = DataType_::INVALID;
		if (args[2] == "filter") type = DataType_::FILTEREN;
		else if (args[2] == "peakadc") type = DataType_::PEAKADC;
		else if (args[2] == "tqdc") type = DataType_::TRACEQDC;
		else {
			std::cout << "ERROR: Incorrect type: '" << args[2] << "'.\n";
			std::cout << "Valid choices: filter, peakadc, tqdc\n";
		}
		if (mod < 0 || chan < 0 || type == DataType_::INVALID) return true;

		if (args.size() == 3) {
			newHists_.push_back(std::make_tuple(mod, chan, type, gPad));
		}
		else {
			int padIndex = 0;
			try { padIndex = std::stoi(args[3]); }
			catch (const std::invalid_argument& ia) { 
				std::cout << "ERROR: Invalid pad index: '" << args[3] << "'\n"; 
				return true;
			}
			TVirtualPad *pad = GetCanvas()->GetPad(padIndex);
			if (!pad || pad ==0) {
				std::cout << "ERROR: Invalid pad index: " << padIndex << ".\n";
				return true;
			}
			newHists_.push_back(std::make_tuple(mod, chan, type, pad));
		}
		GetCanvas()->Update();
		return true;
		
	}
	else if (cmd == "clear") {
		tree_->Reset();

		for (auto padItr=histos_.begin(); padItr != histos_.end(); ++padItr) {
			HistMap_ *map = &padItr->second;
			for (auto itr = map->begin(); itr != map->end(); ++itr) {
				TH1F* hist = dynamic_cast<TH1F*> (gDirectory->Get(itr->second.c_str()));
				if (hist) hist->Reset();
			}
		}
		return true;
	}
	else if (cmd == "divide") {
		if (args.size() == 1) {
			int pads = 0;
			try { pads = std::stoi(args[0]); }
			catch (const std::invalid_argument& ia) { 
				std::cout << "ERROR: Invalid pad argument.\n";
			}
			if (pads <= 0) return true;
			GetCanvas()->Clear();
			GetCanvas()->DivideSquare(pads);
			return true;
		}
		else if (args.size() == 2) {
			int padsX = 0, padsY = 0;
			try { padsX = std::stoi(args[0]); }
			catch (const std::invalid_argument& ia) { 
				std::cout << "ERROR: Invalid pad X argument: '" << args[0] << "'\n";
			}
			try { padsY = std::stoi(args[1]); }
			catch (const std::invalid_argument& ia) { 
				std::cout << "ERROR: Invalid pad Y argument: '" << args[1] << "'\n";
			}
			if (padsX <= 0 || padsY <= 0) return true;
			GetCanvas()->Clear();
			GetCanvas()->Divide(padsX, padsY);
			return true;
		}
		else {
			std::cout << "ERROR: Incorrect syntax for divide command.\n";
			std::cout << "Usage: divide <numPads>\n";
			std::cout << "       divide <numXPads> <numYPads>\n";
			return true;
		}
	}

	return false;
}

void HistScanner::ProcessNewHists() {
	while (!newHists_.empty()) {
		NewHistKey_ key = newHists_.back();
		int mod = std::get<0>(key);
		int chan = std::get<1>(key);
		DataType_ type = std::get<2>(key);
		TVirtualPad* pad = std::get<3>(key);

		//Store the new keys in the map with a null hist.
		histos_[pad][std::make_tuple(mod, chan, type)] = "";
		//Make the inital plot.
		Plot(mod, chan, type, pad);

		newHists_.pop_back();
	}
}
void HistScanner::Plot(int mod, int chan, DataType_ type, TVirtualPad *pad /*= gPad*/) {
	static const std::vector< Color_t > colors = {kBlue + 2, kRed, kGreen + 1, kMagenta + 2};

	//Determine the name of the branch to plot.
	std::string branch;
	switch (type) {
		case DataType_::FILTEREN :
			branch = "filterEn"; break;
		case DataType_::PEAKADC :
			branch = "peakAdc"; break;
		case DataType_::TRACEQDC :
			branch = "traceQdc"; break;
		default :
			return;
	}

	//Declare string streams used to populate the draw command.
	std::stringstream histName;
	std::stringstream drawCmd;
	std::stringstream drawWeight;
	std::stringstream drawOpt;

	//Determine the weight or cut, we only want data from the specific mod/chan.
	drawWeight << "mod == " << mod << " && chan == " << chan;

	//Declare a pointer to the histogram.
	TH1F* hist = nullptr;
	Color_t drawColor = kBlack;
	//Determine the key.
	auto key = std::make_tuple(mod, chan, type);
	//Find the entry matching the key.
	auto padItr = histos_.find(pad);
	HistMap_ *map = nullptr; 
	if (padItr != histos_.end()) {
		map = &padItr->second; 
		auto itr = map->find(key);

		//If we found something set the hist pointer.
		if (itr != map->end()) hist = dynamic_cast<TH1F*> (gDirectory->Get(itr->second.c_str()));
		//If this is not the first plot and the map is not empty we set option "SAME".
		if (itr != map->begin() && !map->empty()) {
			drawOpt << "SAME";
		}
		//Determine the line color if the hist is not defined.
		if (!hist) {
			drawColor = colors.at(std::distance(map->begin(), itr));
		}
	}

	if (hist) {
		histName << hist->GetName();
		drawCmd << branch << ">>+" << histName.str();

		tree_->Draw(drawCmd.str().c_str(),drawWeight.str().c_str(),drawOpt.str().c_str(),std::numeric_limits<Long64_t>::max(),hist->GetEntries());
		GetCanvas()->Modified();
	}
	else {
		histName << "hM" << mod << "C" << chan << "_" << branch;
		drawCmd << branch << ">>" << histName.str() << "(1000)";

		TVirtualPad *prevPad = gPad;
		pad->cd();
		if (tree_->Draw(drawCmd.str().c_str(),drawWeight.str().c_str(),drawOpt.str().c_str())) {

			hist = (TH1F*) gDirectory->Get(histName.str().c_str());
			if (hist) {
				std::stringstream title;
				title << "M" << mod << "C" << chan << " " << branch;
				hist->SetTitle(title.str().c_str());
				hist->SetLineColor(drawColor);

//				float xMin = hist->GetXaxis()->GetXmin();
//				float xMax = hist->GetXaxis()->GetXmax();
//				hist->SetBins(xMax - xMin, xMin, xMax);

				//If the map is not empty and the draw option is not SAME, 
				//	we need to redraw all the histograms on the pad.
				if (map && !map->empty() && drawOpt.str() != "SAME") {
					for (auto itr = map->begin(); itr != map->end(); ++itr) {
						if (itr != map->begin()) {
							TH1F* hist = dynamic_cast<TH1F*> (gDirectory->Get(itr->second.c_str()));
							if (hist) {
								hist->SetLineColor(colors.at(std::distance(map->begin(), itr)));
								hist->Draw("SAME");
							}
						}
					}
				}

				ResetZoom(pad);
				pad->Modified();
			}
		
			histos_[pad][key] = histName.str();
		}
		else histos_[pad][key] = "";

		prevPad->cd();
	}
}

void HistScanner::ScaleHistograms(TVirtualPad* pad) {
	auto padItr = histos_.find(pad);
	if (padItr != histos_.end()) {
		HistMap_ map = padItr->second;
		float xMin, xMax, yMin, yMax;
		for (auto itr = map.begin(); itr != map.end(); ++itr) {
			TH1F* hist = dynamic_cast<TH1F*> (gDirectory->Get(itr->second.c_str()));
			if (hist) {
				if (hist->GetXaxis()->GetXmin() < xMin) xMin = hist->GetXaxis()->GetXmin();
				if (hist->GetXaxis()->GetXmax() > xMax) xMax = hist->GetXaxis()->GetXmax();
				if (hist->GetYaxis()->GetXmin() < yMin) yMin = hist->GetYaxis()->GetXmin();
				if (hist->GetYaxis()->GetXmax() > yMax) yMax = hist->GetYaxis()->GetXmax();
				if (hist->GetBinContent(hist->GetMaximumBin()) > yMax) {
					yMax = hist->GetBinContent(hist->GetMaximumBin());
				}
				if (yMin < 0.1) yMin = 0.1;
			}
		}
		for (auto itr = map.begin(); itr != map.end(); ++itr) {
			TH1F* hist = dynamic_cast<TH1F*> (gDirectory->Get(itr->second.c_str()));
			if (hist) {
				hist->GetXaxis()->SetLimits(xMin, xMax);
				hist->GetYaxis()->SetLimits(yMin, yMax);
				if (itr == map.begin()) hist->Draw();
				else hist->Draw("SAME");
				UpdateZoom(hist->GetXaxis(), hist->GetYaxis(), pad);
			}
		}
	}
}

