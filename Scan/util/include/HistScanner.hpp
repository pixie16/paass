#ifndef HISTSCANNER_H
#define HISTSCANNER_H

#include <vector>
#include <map>
#include <tuple>

#include "TTree.h"
#include "TFile.h"

#include "RootScanner.hpp"
#include "HistScannerChanData.hpp"

class HistScanner : public RootScanner {
	public:
		/// @brief Default constructor.
		HistScanner();
		/// @brief Default destructor.
		virtual ~HistScanner();
		/// @brief Processes each event by filling the tree and clearing the event.
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
		/// @brief Create a HistScannerChanData from the porivded XiaData.
		/// @param[in] event An XIA event to process.
		bool AddEvent(XiaData* event);

	private:
		Unpacker* GetCore();

		TFile *file_; //<ROOT file containing the tree.
		TTree *tree_; //<Tree containing the current plottable data.

		/// Vector containing all the channel data for an event.
		std::vector< HistScannerChanData > *eventData_; 

		/// The type for key for the hist map.
		typedef std::tuple< int, int, std::string > HistKey_;
		/// The type for the histogram map.
		typedef std::map< HistKey_, std::string > HistMap_;

		///A vector of new histograms, containing a HistKey_ and a TVirtualPad*.
		std::vector< std::pair< HistKey_, TVirtualPad* > > newHists_;
		///A map of plotted histograms.
		std::map< TVirtualPad*, HistMap_ > histos_;
		///A map whose value is the number of times a histogram key was requested
		/// for plotting.
		std::map< HistKey_, int > histCount_;
		///The number of TTree entries that had been scanned to create the 
		/// corresponding histogram.
		std::map< TH1*, long > treeEntries_;

		///@brief Initialize and plot newly requested plots.
		void ProcessNewHists();
		///@brief Plot the histogram corresponding to the provided key.
		///@param[in] key The key of the histogram to plot.
		///@param[in] par The corresponding pad that the histogram should be 
		/// plotted on
		void Plot(HistKey_ key, TVirtualPad *pad = gPad);

};

#endif //HISTSCANNER_H
