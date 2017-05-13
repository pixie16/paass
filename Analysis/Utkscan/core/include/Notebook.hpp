/** \file Notebook.hpp
 * \brief An object to take notes
 * \author K. A. Miernik
 * \date April 13, 2013
 */
#ifndef __NOTEBOOK_HPP__
#define __NOTEBOOK_HPP__

#include <string>
#include <fstream>

//! A class to output things to a notebook
class Notebook {
public:
    /** \return only instance of Notebook class. */
    static Notebook *get();

    /** Save string to file
    * \param [in] note : the note to report in the file */
    void report(std::string note);

    /** \return the current date and time */
    const std::string currentDateTime() const;

    /** Default Destructor */
    ~Notebook();

private:
    Notebook(); //!<Default constructor
    Notebook(Notebook const &); //!< Overload of the constructor
    void operator=(Notebook const &);//!< the copy constructor
    static Notebook *instance;//!< static instance of the class

    std::string mode_; //!< the mode for the notebook class
    std::string file_name_;//!< the file name to output into
};

#endif
