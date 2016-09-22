#ifndef CTERMINALCOMMAND_HPP
#define CTERMINALCOMMAND_HPP

#include <string>
#include <vector>

/// @brief An abstract class providing information about a command.
/// This class contains methods and information needed to create, process, and 
/// execute a command. It is expected that a derived class set all possible 
/// options upon construction and overload the Execute() method. When the user
/// constructs the command and arguments tab completion options are provided via
/// GetOptions() and execution occurs via Execute().
class Command {
	public:
		/// @brief Default constructor.
		/// @param[in] Name of the command.
		Command(std::string name);

		/// @brief Default destructor.
		virtual ~Command() { };

		/// @brief This method executes the command with the supplied arguments.
		/// @param[in] arguments The arguments for the command.
		/// @return Nothing.
		virtual void Execute(const std::vector< std::string > &arguments) = 0;

		/// @brief Returns the name of the command.
		/// @return Name of the command.
		const std::string& GetName() {return name_;}

		/// @brief This method provides possible options for provided index.
		/// @param[in] index The index for which options should be returned.
		/// @return A vector of strings containing all options for index.
		std::vector< std::string > GetOptions(const int& index = -1);

	protected:
		/// @brief Method adds an option to list of valid options.
		/// @param[in] index The index for which the option is valid.
		/// @param[in] option The option value.
		/// @return Nothing.
		void AddOption(unsigned int index, std::string option);

	private:
		const std::string name_; //< Name of the command.
		std::vector< std::vector< std::string> > options_; //< Vector of valid options. First index is argument index.
};

#endif //CTERMINALCOMMAND_HPP
