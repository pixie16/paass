/** \file Notebook.hpp
 *
 * An object to take notes
 */

#ifndef NOTEBOOKHPP
#define NOTEBOOKHPP

#include <string>
#include <fstream>

class Notebook {
public:
    /** Returns only instance of Notebook class.*/
    static Notebook* get();

    /** Save string to file */
    void report(std::string note);

    /** Get current date and time */
    const std::string currentDateTime() const;

    ~Notebook();

private:
    /** Make constructor, copy-constructor and operator =
        * private to complete singleton implementation.*/
    Notebook();
    /* Do not implement*/
    Notebook(Notebook const&);
    void operator=(Notebook const&);
    static Notebook* instance;

    std::string mode_;
    std::string file_name_;
};

#endif 
