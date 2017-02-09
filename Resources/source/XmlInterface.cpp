///@file XmlInterface.cpp
///@brief Singleton class that handles opening and closing XML files using
/// pugixml.
///@author S. V. Paulauskas
///@date February 09, 2017
#include <sstream>
#include <stdexcept>

#include "XmlInterface.hpp"

XmlInterface *XmlInterface::instance_ = NULL;

XmlInterface::XmlInterface(const std::string &file) {
    pugi::xml_parse_result result = xmlDocument_.load_file(file.c_str());

    if (!result) {
        std::stringstream ss;
        ss << "XmlInterface::XmlInterface : We were unable to open a "
                "configuration file named \"" << file << "\".";
        throw std::invalid_argument(ss.str());
    }
}

/** Instance is created upon first call */
XmlInterface *XmlInterface::get() {
    if (!instance_)
        instance_ = new XmlInterface("Config.xml");
    return (instance_);
}

/** Instance is created upon first call */
XmlInterface *XmlInterface::get(const std::string &file) {
    if (!instance_)
        instance_ = new XmlInterface(file);
    return (instance_);
}

XmlInterface::~XmlInterface() {
    delete (instance_);
    instance_ = NULL;
}