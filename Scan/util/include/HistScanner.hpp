#ifndef HISTSCANNER_H
#define HISTSCANNER_H

#include <vector>
#include <map>

#include "TTree.h"
#include "TFile.h"

#include "RootScanner.hpp"
#include "HistScannerChanData.hpp"

class HistScanner : public RootScanner {
	public:
		HistScanner();
		/// Destructor.
		virtual ~HistScanner();
		bool ProcessEvents();
		/** ExtraCommands is used to send command strings to classes derived
		 * from ScanInterface. If ScanInterface receives an unrecognized
		 * command from the user, it will pass it on to the derived class.
		 * \param[in]  cmd_ The command to interpret.
		 * \param[out] arg_ Vector or arguments to the user command.
		 * \return True if the command was recognized and false otherwise.
		 */
		virtual bool ExtraCommands(const std::string &cmd_, std::vector<std::string> &args_);
		void IdleTask();


	protected:
		bool AddEvent(XiaData* event);

	private:
		TFile *file_;
		TTree *tree_; //<Tree containing the current plottable data.
		Unpacker* GetCore();

		std::vector< HistScannerChanData > *eventData_; //< Vector containing all the channel data for an event.

		enum class DataType_ {FILTEREN, PEAKADC, TRACEQDC, INVALID}; //< Data types.
		typedef std::map< std::tuple< int, int, DataType_ >, std::string > HistMap_;
		std::map < TVirtualPad*, HistMap_ > histos_;

		void Plot(int mod, int chan, DataType_ type, TVirtualPad *pad = gPad);
	
		void ScaleHistograms(TVirtualPad* pad);

};

#endif //HISTSCANNER_H
