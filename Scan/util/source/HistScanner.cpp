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
	//Do not delete as it causes segfault.
	ChannelEvent *chEvent = new ChannelEvent(event);
	HistScannerChanData data;
	data.mod = event->modNum;
	data.chan = event->chanNum;
	data.filterEn = event->energy; 
	chEvent->CorrectBaseline();
	data.peakAdc = chEvent->maximum;
	data.traceQdc = chEvent->IntegratePulse(chEvent->max_index - 10, chEvent->max_index + 15);

	eventData_->push_back(data);
	return true;
}

void HistScanner::IdleTask() {
	ProcessNewHists();
	for (auto padItr=histos_.begin(); padItr != histos_.end(); ++padItr) {
		TVirtualPad *pad = padItr->first;
		//Rescale all histograms on the pad.
		ScaleHistograms(pad);
	}
	GetCanvas()->Update();

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
				Plot(itr->first, pad);
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
		std::string type;
		if (args[2] == "filter") type = "filterEn";
		else if (args[2] == "peakadc") type = "peakAdc";
		else if (args[2] == "tqdc") type = "traceQdc";
		else {
			std::cout << "ERROR: Incorrect type: '" << args[2] << "'.\n";
			std::cout << "Valid choices: filter, peakadc, tqdc\n";
		}
		if (mod < 0 || chan < 0 || type == "") return true;

		//Determine pad
		TVirtualPad *pad = gPad;
		if (args.size() == 4) {
			int padIndex = 0;
			try { padIndex = std::stoi(args[3]); }
			catch (const std::invalid_argument& ia) { 
				std::cout << "ERROR: Invalid pad index: '" << args[3] << "'\n"; 
				return true;
			}
			pad = GetCanvas()->GetPad(padIndex);
			if (!pad || pad ==0) {
				std::cout << "ERROR: Invalid pad index: " << padIndex << ".\n";
				return true;
			}
		}

		//Add to the new histogram vector.
		newHists_.push_back(std::make_pair(std::make_tuple(mod, chan, type), pad));

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
		auto key = newHists_.back().first;
		auto pad = newHists_.back().second;

		int mod = std::get<0>(key);
		int chan = std::get<1>(key);
		std::string type = std::get<2>(key);

		std::stringstream histName;
		histName << "hM" << mod << "C" << chan << "_" << type << "_" << histCount_[key]++;

		histos_[pad][key] = histName.str();

		//Make the initial plot.
		Plot(key, pad);

		newHists_.pop_back();
	}
	GetCanvas()->Update();
}
void HistScanner::Plot(HistKey_ key, TVirtualPad *pad /*= gPad*/) {
	static const std::vector< Color_t > colors = {kBlue + 2, kRed, kGreen + 1, kMagenta + 2};

	int mod = std::get<0>(key);
	int chan = std::get<1>(key);
	std::string type = std::get<2>(key);

	//Declare string streams used to populate the draw command.
	std::stringstream drawCmd;
	std::stringstream drawWeight;
	std::stringstream drawOpt;
	std::string histName;
	Color_t drawColor = kBlack;

	//Determine the weight or cut, we only want data from the specific mod/chan.
	drawWeight << "mod == " << mod << " && chan == " << chan;

	//Find the entry matching the key.
	auto padItr = histos_.find(pad);
	if (padItr == histos_.end()) {
		std::cout << "ERROR: Unable to locate pad in map!\n";
		return;
	}

	HistMap_ *histMap = &(padItr->second); 
	auto histItr = histMap->find(key);

	if (histItr == histMap->end()) {
		std::cout << "ERROR: Unable to locate key in map!\n";
		return;
	}

	histName = histItr->second;

	TH1F* hist = dynamic_cast<TH1F*> (gDirectory->Get(histName.c_str()));

	//If this is not the first plot and the map is not empty we set option "SAME".
	if (histItr != histMap->begin()) {
		drawOpt << "SAME";
	}

	if (hist) {
		drawCmd << type << ">>+" << histName;

		tree_->Draw(drawCmd.str().c_str(),drawWeight.str().c_str(),"GOFF",std::numeric_limits<Long64_t>::max(),hist->GetEntries());
		pad->Modified();
	}
	else {
		drawColor = colors.at(std::distance(histMap->begin(), histItr));
		drawCmd << type << ">>" << histName << "(1000)";

		TVirtualPad *prevPad = gPad;
		pad->cd();
		//std::cout << "Draw(\"" << drawCmd.str() << "\", \"" << drawWeight.str() << "\", \"" << drawOpt.str() << "\")";
		if (tree_->Draw(drawCmd.str().c_str(),drawWeight.str().c_str(),"GOFF")) {
			hist = (TH1F*) gDirectory->Get(histName.c_str());
			if (hist) {
				//Determine the x and y maximum from the histogram
				float xMax = hist->GetXaxis()->GetXmax();
				float yMax = hist->GetBinContent(hist->GetMaximumBin());

				//Check if a pad maximum has been declared.
				auto padMaxItr = padMaximums_.find(pad);
				//If none found we seet it to the maximums for this hist.
				if (padMaxItr == padMaximums_.end()) {
					padMaximums_[pad] = std::make_pair(xMax,yMax);
				}
				//If found we compare to maximusm for this hist and set them accordingly.
				else {
					float *padXMax = &(padMaxItr->second.first);
					float *padYMax = &(padMaxItr->second.second);
					if (*padXMax > xMax) xMax = *padXMax;
					else *padXMax = xMax;
					if (*padYMax > yMax) yMax = *padYMax;
					else *padYMax = yMax;
				}

				hist->SetBins(xMax, 0, xMax);
				hist->Reset();

				drawCmd.str("");
				drawCmd << type << ">>" << histName;

				tree_->Draw(drawCmd.str().c_str(),drawWeight.str().c_str(),"GOFF",std::numeric_limits<Long64_t>::max(),hist->GetEntries());

				hist->GetYaxis()->SetLimits(0,yMax);

				std::stringstream title;
				title << "M" << mod << "C" << chan << " " << type;
				hist->SetTitle(title.str().c_str());
				hist->SetLineColor(drawColor);

				hist->Draw(drawOpt.str().c_str());

				//				ScaleHistograms(pad);

				if (histItr == histMap->begin() && histMap->size() > 1) {
					for (auto itr = histMap->begin(); itr != histMap->end(); ++itr) {
						TH1F* hist = dynamic_cast<TH1F*> (gDirectory->Get(itr->second.c_str()));
						if (hist) {
							hist->SetLineColor(colors.at(std::distance(histMap->begin(), itr)));
							if (itr == histMap->begin()) hist->Draw();
							else	hist->Draw("SAME");
						}
					}
				}


				pad->Modified();
				GetCanvas()->Update();

				ResetZoom(pad);
				//pad->Modified();
				//GetCanvas()->Update();

				//If the map is not empty and the draw option is not SAME, 
				//	we need to redraw all the histograms on the pad.
				/*
					if (drawOpt.str() != "SAME" && map->size() > 1) {
					std::cout << "Need to replot as the order of plots has changed.\n";
					for (auto itr = map->begin(); itr != map->end(); ++itr) {
					if (itr != map->begin()) {
					TH1F* hist = dynamic_cast<TH1F*> (gDirectory->Get(itr->second.c_str()));
					if (hist) {
					hist->SetLineColor(colors.at(std::distance(map->begin(), itr)));
					std::cout << "Plotting " << hist->GetLineColor() << "\n";
					hist->Draw("SAME");
					}
					}
					}
					}
					*/
				//pad->Modified();
				//GetCanvas()->Update();
			}

		}

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
					std::cout << "New ymax " << yMax << "->" << hist->GetBinContent(hist->GetMaximumBin()) << "\n";
					yMax = hist->GetBinContent(hist->GetMaximumBin());
				}
			}
		}
		for (auto itr = map.begin(); itr != map.end(); ++itr) {
			TH1F* hist = dynamic_cast<TH1F*> (gDirectory->Get(itr->second.c_str()));
			if (hist) {
				std::cout << "Set limits " << xMin << "-" << xMax << " " << yMin << "-" << yMax << "\n";
				hist->SetMaximum(yMax);
				hist->GetXaxis()->SetLimits(xMin, xMax);
				hist->GetYaxis()->SetLimits(yMin, yMax);
				//if (itr == map.begin()) hist->Draw();
				//else hist->Draw("SAME");
				std::cout << "Get limits " << hist->GetXaxis()->GetXmin() << "-" << hist->GetXaxis()->GetXmax() << " " << hist->GetYaxis()->GetXmin() << "-" << hist->GetYaxis()->GetXmax() << "\n";
				UpdateZoom(hist->GetXaxis(), hist->GetYaxis(), pad);
				std::cout << "U Get limits " << hist->GetXaxis()->GetXmin() << "-" << hist->GetXaxis()->GetXmax() << " " << hist->GetYaxis()->GetXmin() << "-" << hist->GetYaxis()->GetXmax() << "\n";
			}
		}
		pad->Modified();
		GetCanvas()->Update();
	}
}

