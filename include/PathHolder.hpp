#ifndef PATHHOLDER_H
#define PATHHOLDER_H

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "Exceptions.hpp"

class PathHolder {
    public:
        PathHolder () {}

        PathHolder (std::string fileName);

        std::string GetConfigPath () {
            return path_;
        }

        std::string GetFullPath (std::string file) {
            std::stringstream ss;
            ss << path_ << file;
            return ss.str();
        }

        void SetPath(std::string fileName);

    private:
        static std::string path_;
};

#endif
