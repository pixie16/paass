#include <iostream>
#include <fstream>

#include "Unpacker.hpp"
#include "ScanInterface.hpp"
#include "hribf_buffers.h"

DIR_buffer ldfDir;
HEAD_buffer ldfHead;
PLD_header pldHead;

void help(char *name_){
	std::cout << "  SYNTAX: " << name_ << " <files ...>\n";
}

int main(int argc, char *argv[]){
	if(argc < 2){
		std::cout << " Error: Invalid number of arguments to " << argv[0] << ". Expected 1, received " << argc-1 << ".\n";
		help(argv[0]);
		return 1;
	}

	int file_format = -1;
	std::string dummy, extension;
	for(int i = 1; i < argc; i++){
		std::cout << "File no. " << i << ": " << argv[i] << std::endl;

		extension = get_extension(argv[i], dummy);
		if(extension == "ldf") // List data format file
			file_format = 0;
		else if(extension == "pld") // Pixie list data file format
			file_format = 1;
		else{
			std::cout << " ERROR! Invalid file extension '" << extension << "'.\n\n";
			continue;
		}
		
		std::ifstream file(argv[i], std::ios::binary);
		if(!file.is_open() || !file.good()){
			std::cout << " ERROR! Failed to open input file! Check that the path is correct.\n\n";
			file.close();
			continue;
		}

		// Start reading the file
		// Every poll2 ldf file starts with a DIR buffer followed by a HEAD buffer
		if(file_format == 0){
			ldfDir.Read(&file);
			ldfHead.Read(&file);
			ldfDir.Print();
			ldfHead.Print();
		}
		else if(file_format == 1){
			pldHead.Read(&file);
			pldHead.Print();	
		}
		
		file.close();
		
		std::cout << std::endl;
	}
	
	return 0;
}


