#include "HistScanner.hpp"

#include <limits>
#include <stdexcept>

#include "TDirectory.h"
#include "TH1F.h"
#include "TColor.h"
#include "TTreeFormula.h"

#include "TError.h"

#include "XiaData.hpp"

#include "HistUnpacker.hpp"

///Initialize the scanner as well as create a ROOT file and tree to store 
/// processed data.
HistScanner::HistScanner() :
	RootScanner()
{
	//Only output ROOT errors if they are fatal.
	gErrorIgnoreLevel = kFatal;

	file_ = new TFile("histScanner.root","RECREATE");
	eventData_ = new HistScannerChanData();
	tree_ = new TTree("data","");
	tree_->Branch("eventData",&eventData_);
}

///Write the tree to to file, close the file and destroy the objects.
HistScanner::~HistScanner() {
	tree_->ResetBranchAddresses();
	file_->Write();
	file_->Close();
	delete file_;
	delete eventData_;
}

/// Return a pointer to the Unpacker object to use for data unpacking.
/// If no object has been initialized, create a new one.
/// @return Pointer to an Unpacker object.
Unpacker *HistScanner::GetCore(){ 
	if(!core){ core = new HistUnpacker(); }
	return core;
}

///Fills a HistScannerChanData with values from XiaData and derived ChannelData.
/// The HistScannerChanData is then pushed back onto the eventData_ vector to 
/// be process by ProcessEvents.
bool HistScanner::AddEvent(XiaData* event) {		
	eventData_->Set(event);
	return true;
}

void HistScanner::IdleTask() {
	//Process any new histograms the user defined.
	ProcessNewHists();

	//Update the zoom on each pad, in case the user modified the histograms
	for (auto padItr=histos_.begin(); padItr != histos_.end(); ++padItr) {
		TVirtualPad *pad = padItr->first;
		UpdateZoom(pad);
	}

	//Run the RootScanner IdleTask.
	RootScanner::IdleTask();
}

///Processes a built event by filling the tree. After a certain number of 
/// events the histograms are replotted. This routine clears the event 
/// after filling the tree.
bool HistScanner::ProcessEvents() {
	//Fill the tree with the current event.
	tree_->Fill();

	if (tree_->GetEntries() % 1000 == 0) {
		for (auto padItr=histos_.begin(); padItr != histos_.end(); ++padItr) {
			TVirtualPad *pad = padItr->first;
			HistMap_ *map = &padItr->second;
			for (auto itr = map->begin(); itr != map->end(); ++itr) {
				Plot(itr->first, pad);
			}
			UpdateZoom(pad);
		}
		GetCanvas()->Update();
	}
	//We've processed the data so we clear the class for the next data.
	eventData_->Clear();

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
	if (cmd == "plot") { return PlotCommand(args); }
	else if (cmd == "zero") { return ZeroCommand(args); }
	else if (cmd == "divide") { return DivideCommand(args); }
	else if (cmd == "help") { return HelpCommand(args); }
	return false;
}

void HistScanner::CmdHelp(const std::string &prefix) {
	HelpCommand(std::vector< std:: string >());
}

bool HistScanner::HelpCommand(const std::vector<std::string> &args) {
	if (args.size() == 1) {
		if (args[0] == "plot") {	
			std::cout << "Usage: plot <mod> <chan> <expr> [pad]\n";
			std::cout << "       plot <expr> [pad]\n";
			std::cout << " Plots a new histogram for module, chan and expr specified.\n";
			std::cout << " If no pad is specified the plot is added to the currently selected pad.\n";
			return true;
		}
		else if (args[0] == "zero") {
			std::cout << "Usage: zero\n";
			std::cout << " Zeros all histograms and stored data.\n";
			return true;
		}
		else if (args[0] == "divide") {
			std::cout << "Usage: divide <numPads>\n";
			std::cout << "       divide <numXPads> <numYPads>\n";
			std::cout << " Divides the canvas in the selected number of pads.\n";
			return true;
		}
	}
	std::cout << "Specific Commands:	\n";
	std::cout << " plot   - Creates a plot.\n";
	std::cout << " zero   - Zeros all plots and associated data.\n";
	std::cout << " divide - Divides the canvas into multiple pads.\n";
	return true;
}

bool HistScanner::PlotCommand(const std::vector<std::string> &args) {
	if (args.size() == 0 || args.size() > 4) {
		std::cout << "ERROR: Incorrect syntax for plot command.\n";
		std::cout << "Usage: plot <mod> <chan> <expr> [pad]\n";
		std::cout << "       plot <expr> [pad]\n";
		return true;
	}

	int mod = -1, chan = -1;
	if (args.size() > 2) {
		try { mod = std::stoi(args[0]); }
		catch (const std::invalid_argument& ia) { 
			std::cout << "ERROR: Invalid module argument: '" << args[0] << "'\n"; 
		}
		try { chan = std::stoi(args[1]); }
		catch (const std::invalid_argument& ia) { 
			std::cout << "ERROR: Invalid channel argument: '" << args[1] << "'\n"; 
		}
		//Stop if the choice made so far are invalid.
		if (mod < 0 || chan < 0) return true;
	}

	std::string expr;
	if (args.size() > 2) expr = args[2];
	else expr = args[0];

	//Test the expression is valid.
	size_t startPos = 0, stopPos = 0;
	//Loop over each argument separated by colons.
	while(stopPos < expr.length()) {
		stopPos = expr.find_first_of(":", startPos);
		if (stopPos == std::string::npos) stopPos = expr.length();

		//The plotter can only handle 1D right now.
		if (stopPos != expr.length()) {
			std::cout << "ERROR: Only 1D plots are currently supported.\n";
			
			//Stop the plot command
			return true;
		}

		//Get the subexpression to test
		std::string split = expr.substr(startPos, stopPos - startPos);

		//Test it with a TTreeFormula.
		TTreeFormula formula("test", split.c_str(),tree_);
		if(formula.GetNdim() == 0) {
			std::cout << "ERROR: Incorrect expr: '" << expr << "', ";
			std::cout << "(" << split << ").\n";
			std::cout << "Valid choices: ";

			TList *list = (TList*) tree_->GetListOfLeaves();
			for( TObject *obj = list->First(); obj; obj = list->After(obj)) {
				std::cout << obj->GetName();
				if (obj != list->Last()) std::cout << ", ";
			}
			std::cout << "\n";
			
			//Stop the plot command
			return true;
		}
		
		//Iterate the start position forward one unit.
		startPos = stopPos + 1;
	}

	//Determine the weight for the provided expression
	// If the module and channel number are provided we determine the weight
	// from the multiplicity for that pair.
	std::stringstream weight;
	if (mod > -1 && chan > -1) {
		weight << "mult[" << mod << "][" << chan << "]>0";
	}

	//Determine pad
	TVirtualPad *pad = gPad;

	//Get the string argument provided for the pad
	std::string padStr = "";
	if (args.size() == 2) padStr = args[1];
	else if (args.size() == 4) padStr = args[3]; 

	//If a string argument was provided we try to find the pad.
	if (padStr != "") {	
		int padIndex = 0;
		try { padIndex = std::stoi(padStr); }
		catch (const std::invalid_argument& ia) { 
			std::cout << "ERROR: Invalid pad index: '" << padStr << "'\n"; 
			return true;
		}
		pad = GetCanvas()->GetPad(padIndex);
		if (!pad || pad ==0) {
			std::cout << "ERROR: Invalid pad index: " << padIndex << ".\n";
			return true;
		}
	}

	//Add to the new histogram vector.
	newHists_.push_back(std::make_pair(std::make_tuple(expr, weight.str()), pad));

	return true;

}
bool HistScanner::ZeroCommand(const std::vector<std::string> &args) {
	tree_->Reset();

	for (auto padItr=histos_.begin(); padItr != histos_.end(); ++padItr) {
		HistMap_ *map = &padItr->second;
		for (auto itr = map->begin(); itr != map->end(); ++itr) {
			TH1F* hist = dynamic_cast<TH1F*> (gDirectory->Get(itr->second.c_str()));
			if (hist) hist->Reset();
		}
		ResetZoom(padItr->first);
		padItr->first->Modified();
	}

	GetCanvas()->Update();

	return true;
}
bool HistScanner::DivideCommand(const std::vector<std::string> &args) {
	if (args.size() == 1) {
		int pads = 0;
		try { pads = std::stoi(args[0]); }
		catch (const std::invalid_argument& ia) { 
			std::cout << "ERROR: Invalid pad argument.\n";
		}
		if (pads <= 0) return true;
		//We need to delete all the histos as their associated pads are to be deleted.
		histos_.clear();
		//Clear the canvas
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
		//We need to delete all the histos as their associated pads are to be deleted.
		histos_.clear();
		//Clear the canvas
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

///Defines the newly requested histogram's names and pushes them into the 
/// histogram map to be plotted by Plot(). Plot is then called to provide an 
/// immediate histogram.
void HistScanner::ProcessNewHists() {
	while (!newHists_.empty()) {
		auto key = newHists_.back().first;
		auto pad = newHists_.back().second;

		std::string expr = std::get<0>(key);
		std::string weight = std::get<1>(key);

		//Define the new histograms name.
		std::stringstream histName;
		histName << "h_" << expr << "_" << histCount_[key]++;

		//Push the histogram into the map.
		histos_[pad][key] = histName.str();

		//Make the initial plot.
		Plot(key, pad);
	
		//Update the zoom on the pad for the new histogram.
		UpdateZoom(pad);

		newHists_.pop_back();
	}
}


///The main plotting method. This method determines arguments to be passed to
/// TTree::Draw() to handle plotting. If a histogram has previously been created
/// the Draw command only adds new entries after that last draw to reduce time
/// required to loop over the tree.
void HistScanner::Plot(HistKey_ key, TVirtualPad *pad /*= gPad*/) {
	static const std::vector< Color_t > colors = {kBlue + 2, kRed, kGreen + 1, kMagenta + 2};

	std::string expr = std::get<0>(key);
	std::string weight = std::get<1>(key);

	//Declare string streams used to populate the draw command.
	std::stringstream drawCmd;
	std::stringstream drawWeight;
	std::stringstream drawOpt;
	std::string histName;
	Color_t drawColor = kBlack;

	//Determine the weight or cut, we only want data from the specific mod/chan.
	drawWeight << weight;//"(" << weight << ")";

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

	TVirtualPad *prevPad = gPad;
	pad->cd();

	if (hist) {
		drawCmd << expr << ">>+" << histName;

		tree_->Draw(drawCmd.str().c_str(),drawWeight.str().c_str(),drawOpt.str().c_str(),std::numeric_limits<Long64_t>::max(),treeEntries_[hist]);
		treeEntries_[hist] = tree_->GetEntries();
		pad->Modified();
	}
	else {
		drawColor = colors.at(std::distance(histMap->begin(), histItr));
		drawCmd << expr << ">>" << histName << "(1000)";

		if (tree_->Draw(drawCmd.str().c_str(),drawWeight.str().c_str(),drawOpt.str().c_str())) {
			hist = (TH1F*) gDirectory->Get(histName.c_str());
			if (hist) {
				//Determine the x maximum from the histogram
				float xMax = hist->GetXaxis()->GetXmax();

				hist->SetBins(xMax, 1, xMax);
				hist->Reset();

				drawCmd.str("");
				drawCmd << expr << ">>" << histName;

				tree_->Draw(drawCmd.str().c_str(),drawWeight.str().c_str(),drawOpt.str().c_str());
				treeEntries_[hist] = tree_->GetEntries();

				hist->SetLineColor(drawColor);

				hist->Draw(drawOpt.str().c_str());

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
			}
		}
		else {
			//The Draw command didn't add any entries so we delete the hist and try again later.
			hist = (TH1F*) gDirectory->Get(histName.c_str());
			delete hist;
		}

		prevPad->cd();
	}
}

