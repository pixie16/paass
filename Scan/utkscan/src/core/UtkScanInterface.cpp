
#include <unistd.h>
#include <getopt.h>

#include "DetectorDriver.hpp"
#include "HisFile.hpp"
#include "UtkScanInterface.hpp"
#include "UtkUnpacker.hpp"

// Define a pointer to an OutputHisFile for later use.
OutputHisFile *output_his = NULL;

/// Default constructor.
UtkScanInterface::UtkScanInterface() : ScanInterface() {
    init_ = false;
}

/// Destructor.
UtkScanInterface::~UtkScanInterface(){
    if(init_){
	delete(output_his);
    }
}

/** ExtraCommands is used to send command strings to classes derived
 * from ScanInterface. If ScanInterface receives an unrecognized
 * command from the user, it will pass it on to the derived class.
 * \param[in]  cmd_ The command to interpret.
 * \param[out] arg_ Vector or arguments to the user command.
 * \return True if the command was recognized and false otherwise. */
bool UtkScanInterface::ExtraCommands(const std::string &cmd_, 
				     std::vector<std::string> &args_) {
    if(cmd_ == "mycmd") {
	if(args_.size() >= 1) {
	    // Handle the command.
	} else{
	    std::cout << msgHeader << "Invalid number of parameters to 'mycmd'\n";
	    std::cout << msgHeader << " -SYNTAX- mycmd <param>\n";
	}
    } else 
	return(false); // Unrecognized command.
    
    return(true);
}

/** CmdHelp is used to allow a derived class to print a help statement about
 * its own commands. This method is called whenever the user enters 'help'
 * or 'h' into the interactive terminal (if available).
 * \param[in]  prefix_ String to append at the start of any output.
 * \return Nothing. */
void UtkScanInterface::CmdHelp(){
    std::cout << "   mycmd <param> - Do something useful.\n";
}

/** SyntaxStr is used to print a linux style usage message to the screen.
 * \param[in]  name_ The name of the program.
 * \return Nothing. */
void UtkScanInterface::SyntaxStr(char *name_){ 
    std::cout << " usage: " << std::string(name_) << " [input] [options]\n";
}

/** Initialize the map file, the config file, the processor handler, 
 * and add all of the required processors.
 * \param[in]  prefix_ String to append to the beginning of system output.
 * \return True upon successfully initializing and false otherwise. */
bool UtkScanInterface::Initialize(std::string prefix_) {
    if(init_)
	return(false);

    Globals::get(GetConfigFile());

    try{
	// Read in the name of the his file.
	output_his = new OutputHisFile(GetOutputFile().c_str());
	
	output_his->SetDebugMode(false);
	
	/** The DetectorDriver constructor will load processors
	 *  from the xml configuration file upon first call.
	 *  The DeclarePlots function will instantiate the DetectorLibrary
	 *  class which will read in the "map" of channels.
	 *  Subsequently the raw histograms, the diagnostic histograms
	 *  and the processors and analyzers plots are declared.
	 *
	 *  Note that in the PixieStd the Init function of DetectorDriver
	 *  is called upon first buffer. This include reading in the
	 *  calibration and walk correction factors.
	 */
	DetectorDriver::get()->DeclarePlots();
	output_his->Finalize();
    } catch(std::exception &e){
	// Any exceptions will be intercepted here
	std::cout << prefix_ << "Exception caught at Initialize:" << std::endl;
	std::cout << prefix_ << e.what() << std::endl;
	exit(EXIT_FAILURE);
    }
    return(init_ = true);
}

/** Peform any last minute initialization before processing data. 
 * /return Nothing. */
void UtkScanInterface::FinalInitialization(){
    // Do some last minute initialization before the run starts.
}

/** Receive various status notifications from the scan.
 * \param[in] code_ The notification code passed from ScanInterface methods.
 * \return Nothing. */
void UtkScanInterface::Notify(const std::string &code_/*=""*/) {
    if(code_ == "START_SCAN"){
    } else if(code_ == "STOP_SCAN"){
    } else if(code_ == "SCAN_COMPLETE") { 
	std::cout << msgHeader << "Scan complete.\n"; 
    } else if(code_ == "LOAD_FILE"){ 
	std::cout << msgHeader << "File loaded.\n"; 
    } else if(code_ == "REWIND_FILE"){  
    } else{ 
	std::cout << msgHeader << "Unknown notification code '" << code_ << "'!\n"; 
    }
}

/** Return a pointer to the Unpacker object to use for data unpacking.
 * If no object has been initialized, create a new one.
 * \return Pointer to an Unpacker object. */
Unpacker *UtkScanInterface::GetCore(){ 
    if(!core)
	core = (Unpacker*)(new UtkUnpacker());
    return(core);
}

