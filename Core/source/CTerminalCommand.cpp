#include "CTerminalCommand.hpp"

/// This is the default constructor expecting the name of the command to be 
/// provided.
Command::Command(std::string name) : 
	name_(name)
{

}

/// This method provides options a list of options that can be used for tab
/// completion. If the index is not provided then all valid options are 
/// returned. An invalid index returns an empty vector.
std::vector< std::string > Command::GetOptions(const int& index /* = -1 */) {
	if (index < 0) { //If index is less than zero then we return all options.
		//Construct a vector containing all options.
		std::vector< std::string > allOptions;
		for (auto itr = options_.begin(); itr != options_.end(); ++itr) {
			allOptions.insert(allOptions.end(), itr->begin(), itr->end());
		}
		return allOptions;
	}
	//Index larger than defined options.
	else if ((size_t) index >= options_.size()) { 
		//Return an empty vector as we don't know if there are any valid options.
		return std::vector< std::string >();
	}
	else { //Index was valid, return the list matching the index.
		return options_[index]; 
	}
}

/// Protected method designed to allow derived classes to specify all valid
/// options. Must specify the index in the argument list the option is expected
/// as well as the string of the option.
void Command::AddOption(unsigned int index, std::string option) {
	//Resize the options vector when it is not large enough for the specified index.
	if ((size_t) index > options_.size()) {
		options_.resize(index + 1);
	}

	//Push the option into the vector.
	options_[index].push_back(option);
}
