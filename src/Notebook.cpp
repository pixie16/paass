#include <sstream>
#include <time.h>
#include "pugixml.hpp"

#include "Messenger.hpp"
#include "Notebook.hpp"
#include "Exceptions.hpp"

Notebook* Notebook::instance = NULL;

Notebook* Notebook::get() {
    if (!instance) {
        instance = new Notebook();
    }
    return instance;
}

Notebook::Notebook() {
    pugi::xml_document doc;

    pugi::xml_parse_result result = doc.load_file("Config.xml");
    if (!result) {
        std::stringstream ss;
        ss << "Notebook: error parsing file Config.xml";
        ss << " : " << result.description();
        throw IOException(ss.str());
    }

    pugi::xml_node note = doc.child("Configuration").child("Notebook");

    file_name_ = std::string(note.attribute("file").as_string());
    mode_ = std::string(note.attribute("mode").as_string("a"));

    Messenger m;
    m.detail("Notebook: " + file_name_ + " mode: " + mode_);

    std::ofstream note_file;

    if (mode_ == "r") {
        note_file.open(file_name_.c_str(), std::ios::out);
    } else if (mode_ == "a") {
        note_file.open(file_name_.c_str(), std::ios::out | std::ios::app);
    } else {
        std::stringstream ss;
        ss << "Notebook: unknown mode";
        ss << " : " << mode_;
        throw IOException(ss.str());
    }

    if (!note_file.good()) {
        std::stringstream ss;
        ss << "Notebook: error opening output file";
        ss << " : " << file_name_;
        throw IOException(ss.str());
    }
    note_file << "# Starting notebook on : " << currentDateTime() << std::endl;
    note_file.close();
}

const std::string Notebook::currentDateTime() const {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
    return buf;
}

void Notebook::report(std::string note) {
    std::ofstream note_file;
    note_file.open(file_name_.c_str(), std::ios::out | std::ios::app);
    if (!note_file.good()) {
        std::stringstream ss;
        ss << "Notebook: error opening output file";
        ss << " : " << file_name_;
        throw IOException(ss.str());
    }
    note_file << note << std::endl;
    note_file.close();
}

Notebook::~Notebook() {
    delete instance;
    instance = NULL;
}
