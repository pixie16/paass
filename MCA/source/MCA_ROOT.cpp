#include "MCA_ROOT.h"

#include <TFile.h>
#include <TH1D.h>

#include "Display.h"

MCA_ROOT::MCA_ROOT(PixieInterface *pif, const char *basename) :
	MCA(pif)
{
	OpenFile(basename);
}
MCA_ROOT::~MCA_ROOT() {
	if (IsOpen()) {
		_file->Write(0,TObject::kOverwrite);
		_file->Close();
	}
	delete _file;
}

bool MCA_ROOT::OpenFile(const char* basename) {

	std::string message = std::string("Creating new empty ROOT histogram ") + basename + std::string(".root");
	Display::LeaderPrint(message);
	//We redirect stderr, to catch ROOT error messages
	cerr_redirect *redirect = new cerr_redirect("Pixie16msg.txt");
	//Try and open the file
	_file = new TFile(Form("%s.root",basename),"RECREATE");
	_isOpen = _file->IsOpen() && _file->IsWritable();
	if (Display::StatusPrint(!_isOpen)) {
		//Print out the ROOT error message
		redirect->Print();
		return _isOpen;
	}
	redirect->Print();
	delete redirect;

	//Loop over the number of cards and channels to build the histograms.
	for (int card=0;card < _pif->GetNumberCards();card++) {
		for (int ch=0;ch < _pif->GetNumberChannels();ch++) {
			int id = (card + 1) * 100 + ch;
			_histograms[id] = new TH1D(Form("h%d%02d",card,ch),Form("Mod %d Ch %d",card,ch),ADC_SIZE,0,ADC_SIZE);
		}
	}

}

bool MCA_ROOT::StoreData(int mod, int ch) {
	PixieInterface::word_t histo[ADC_SIZE];

	_pif->ReadHistogram(histo, ADC_SIZE, mod, ch);

	int id = (mod + 1) * 100 + ch;
	auto loc = _histograms.find(id);
	TH1D* histogram;
	if (loc == _histograms.end())	{
		return false;
	}
	histogram = loc->second;

	for (size_t i = 0; i < ADC_SIZE; i++) {
		histogram->SetBinContent(i+1,histo[i]);
	}


  return true;

}
void MCA_ROOT::Flush() {
	_file->Write(0,TObject::kOverwrite);
}
