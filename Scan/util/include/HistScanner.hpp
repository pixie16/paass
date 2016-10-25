#ifndef HISTSCANNER_H
#define HISTSCANNER_H

#include <vector>
#include <map>
#include <tuple>
#include <chrono>
#include <mutex>

#include "TTree.h"
#include "TFile.h"

#include "RootScanner.hpp"
#include "HistScannerChanData.hpp"

//forward declaration of XiaData.
class XiaData;

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

		/** Receive various status notifications from the scan.
		 * \param[in] code_ The notification code passed from ScanInterface methods.
		 * \return Nothing.
		 */
		virtual void Notify(const std::string &code_="");



	protected:
		/// @brief Create a HistScannerChanData from the porivded XiaData.
		/// @param[in] event An XIA event to process.
		bool AddEvent(XiaData* event);

	private:
		Unpacker* GetCore();

		TFile *file_; //<ROOT file containing the tree.
		TTree *tree_; //<Tree containing the current plottable data.

		/// Vector containing all the channel data for an event.
		HistScannerChanData *eventData_; 

		/// The type for key for the hist map.
		typedef std::tuple< std::string, std::string > HistKey_;
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

		std::mutex histMutex_;

		///@brief Initialize and plot newly requested plots.
		void ProcessNewHists();
		///@brief Plot the histogram corresponding to the provided key.
		///@param[in] key The key of the histogram to plot.
		///@param[in] par The corresponding pad that the histogram should be 
		/// plotted on
		void Plot(HistKey_ key, TVirtualPad *pad = gPad);

		void PlotCommand(const std::vector< std::string > &args);
		void RefreshCommand(const std::vector< std::string > &args);
		void ZeroCommand(const std::vector< std::string > &args);
		void ClearCommand(const std::vector< std::string > &args);
		void DivideCommand(const std::vector< std::string > &args);
		void HelpCommand(const std::vector< std::string > &args);

		void CmdHelp(const std::string &prefix);

		float refreshDelaySec_;
		bool refreshRequested_;
		std::chrono::system_clock::time_point lastRefresh_;

};

#endif //HISTSCANNER_H
