/// @author C. R. Thornsberry
#include <iostream>
#include <fstream>
#include <string.h>

#include "Unpacker.hpp"
#include "ScanInterface.hpp"
#include "hribf_buffers.h"

DIR_buffer ldfDir;
HEAD_buffer ldfHead;
PLD_header pldHead;

void help(char *name_) {
    std::cout << "  SYNTAX: " << name_ << " [options] <files ...>\n";
    std::cout << "   Available options:\n";
    std::cout
            << "    --columns | Output file information in tab-delimited columns.\n";
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cout << " Error: Invalid number of arguments to " << argv[0]
                  << ". Expected 1, received " << argc - 1 << ".\n";
        help(argv[0]);
        return 1;
    }

    bool col_output = false;
    int file_format = -1;
    int file_count = 1;
    std::string dummy, extension;
    for (int i = 1; i < argc; i++) {
        // Check for command line options.
        if (strcmp(argv[i], "--columns") == 0) {
            col_output = true;
            continue;
        }

        if (!col_output)
            std::cout << "File no. " << file_count++ << ": " << argv[i]
                      << std::endl;
        else
            std::cout << file_count++ << "\t" << argv[i] << "\t";

        extension = get_extension(argv[i], dummy);
        if (extension == "ldf") // List data format file
            file_format = 0;
        else if (extension == "pld") // Pixie list data file format
            file_format = 1;
        else {
            if (!col_output)
                std::cout << " ERROR! Invalid file extension '" << extension
                          << "'.\n\n";
            else
                std::cout << "FAILED\n";
            continue;
        }

        std::ifstream file(argv[i], std::ios::binary);
        if (!file.is_open() || !file.good()) {
            if (!col_output)
                std::cout
                        << " ERROR! Failed to open input file! Check that the path is correct.\n\n";
            else
                std::cout << "FAILED\n";
            file.close();
            continue;
        }

        // Start reading the file
        // Every poll2 ldf file starts with a DIR buffer followed by a HEAD buffer
        if (file_format == 0) {
            ldfDir.Read(&file);
            ldfHead.Read(&file);
            if (!col_output) {
                ldfDir.Print();
                ldfHead.Print();
            } else {
                ldfDir.PrintDelimited();
                ldfHead.PrintDelimited();
            }
            std::cout << std::endl;
        } else if (file_format == 1) {
            pldHead.Read(&file);
            if (!col_output)
                pldHead.Print();
            else
                pldHead.PrintDelimited();
            std::cout << std::endl;
        }

        file.close();
    }

    return 0;
}


