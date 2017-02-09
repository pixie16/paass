///@file XmlInterface.hpp
///@brief Singleton class that handles opening and closing XML files using
/// pugixml.
///@author S. V. Paulauskas
///@date February 09, 2017
#ifndef PIXIESUITE_XMLINTERFACE_HPP
#define PIXIESUITE_XMLINTERFACE_HPP
#include <string>

#include "pugixml.hpp"

///A class that handles interfacing with an XML file. At the moment I'm going
/// to make this a singleton since it will be easier implementation. I also
/// cannot think of a program at the moment that will need more than one XML
/// file.
class XmlInterface {
public:
    ///@return only instance of XmlInterface class
    static XmlInterface *get();

    ///@return only instance of XmlInterface class
    static XmlInterface *get(const std::string &file);

    ///@return A constant pointer to the document that was opened up
    const pugi::xml_document* GetDocument() const { return &xmlDocument_; }

    ///Default destructor that deletes the instance when its called.
    ~XmlInterface();
private:
    ///Constructor taking a file name as the argument
    ///@param[in] file : The file name that we want to open.
    XmlInterface(const std::string &file);

    XmlInterface(XmlInterface const &);//!< Overload of the constructor
    void operator=(XmlInterface const &); //!< copy constructor
    static XmlInterface *instance_; //!< Create the static instance of the class

    pugi::xml_document xmlDocument_;
};

#endif //PIXIESUITE_XMLINTERFACE_HPP
