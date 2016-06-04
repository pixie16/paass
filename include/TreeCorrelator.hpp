/** \file TreeCorrelator.hpp
 * \brief A class to handle complex correlations between various processors
 *
 * This guy was originally named "NewCorrelator.hpp" it was renamed.
 * \author K. A. Miernik
 * \date August 19, 2012
 */
#ifndef __TREECORRELATOR_HPP__
#define __TREECORRELATOR_HPP__

#include <iostream>
#include <string>
#include <sstream>
#include <map>

#include "pugixml.hpp"
#include "Places.hpp"
#include "PlaceBuilder.hpp"
#include "Exceptions.hpp"

/** \brief XML document walker and parser for TreeCorrelator xml config file*/
class Walker {
public:
    /** Parse specific place
    * \param [in] node : the xml node to parse
    * \param [in] parent : the string of the parent node
    * \param [in] verbose : verbosity
    */
    void parsePlace(pugi::xml_node node, std::string parent,
                    bool verbose);

    /** Walks recursively through the tree
    * \param [in] node : the xml node to parse
    * \param [in] parent : the string of the parent node
    * \param [in] verbose : verbosity
    */
    void traverseTree(pugi::xml_node node, std::string parent,
                      bool verbose);
};

/** \brief Singleton class holding map of all places.*/
class TreeCorrelator {
public:
    /** \return Instance of TreeCorrelator class.*/
    static TreeCorrelator* get();

    /** \return pointer to place or throw exception if it doesn't exist.
    * \param [in] name : the name of the place */
    Place* place(std::string name);

    /** Create place, alter or add existing place to the tree.
    * \param [in] params : the map of the parameters
    * \param [in] verbose : verbosity */
    void createPlace(std::map<std::string, std::string>& params,
                     bool verbose);

    /** Add child to place parent with coincidence coin.
    * \param [in] parent : the name of the parent node
    * \param [in] child : the name of the child node
    * \param [in] coin : coincidence
    * \param [in] verbose : verbosity */
    void addChild(std::string parent, std::string child, bool coin,
                  bool verbose);

    /**
     * This function initializes the correlator tree. Should be called
     * after all basic places from map2.txt were initialized.
    */
    void buildTree();

    /** Default Destructor */
    ~TreeCorrelator();

    /** This map holds all Places. */
    std::map<std::string, Place*> places_;
private:
    /** Make constructor, copy-constructor and operator =
     * private to complete singleton implementation.*/
    TreeCorrelator() {};
    /* Do not implement*/
    TreeCorrelator(TreeCorrelator const&);//!< Overload of the constructor
    void operator=(TreeCorrelator const&); //!< the copy constructor
    static TreeCorrelator* instance; //!< A static instance of the tree correlator

    static PlaceBuilder builder; //!< Instance of the PlaceBuilder

    /** Splits name string into the vector of string. Assumes that if
     * the last token (delimiter being "_") is in format "X-Y,Z" where
     * X, Y are integers, the X and Y are range of base names to be retured
     * E.g. abc_1-2,4,5-6 will return ["abc_1", "abc_2", "abc_4", "abc_5",
     * "abc_6"]. If no range token or comma is found, the name itself is
     * returned as a only element of the vector
     * \param [in] name : the name string to split
     * \return a vector of the split names */
    std::vector<std::string> split_names(std::string name);
};

#endif
