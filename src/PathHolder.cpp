#include "PathHolder.hpp"

std::string PathHolder::path_ = ".";

PathHolder::PathHolder (std::string fileName) {
    SetPath(fileName);
}

void PathHolder::SetPath (std::string fileName) {
    std::ifstream config_file(fileName.c_str());
    std::string line;
    if (config_file.good()) {
        while (getline(config_file, line)) {
            if (*line.begin() != '#' || line.size() > 0) {
                path_ = line;
                if (*(path_.rbegin()) != '/')
                    path_ += "/";
            }
        }
    } else {
        std::stringstream ss;
        ss << "Could not open file " << fileName;
        throw IOException(ss.str());
    }
    config_file.close();
    std::cout << "Configuration path: " << path_ << std::endl;
}
