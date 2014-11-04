/** \file Exceptions.hpp
 * \brief Class to handle errors occurring during the execution of the code
 *
 * \author K. A. Miernik
 * \date November 22, 2012
 */
#ifndef __EXCEPTION_HPP__
#define __EXCEPTION_HPP__

#include <stdexcept>
#include <string>

/** \brief Exception with customizable message.
 *
 * Use derived classes for specific exceptions.*/
class GeneralException : public std::exception {
public:
    /** Default Constructor */
    GeneralException(){};

    /** Constructor for the general exception that accepts a message to print
    * \param [in] msg : the message to print before killing the execution */
    explicit GeneralException(const std::string& msg)
        : exception(), message_(msg) {}

    /** Destructor that throws the exception for us */
    virtual ~GeneralException() throw() {}

    /** Overloading of the what() method that outputs what the error was
     * \return the error */
    virtual const char* what() const throw() {
        return message_.c_str();
    }

protected:
    const std::string message_; //!< The message that is to be printed
};

/** \brief Warning with customizable message.
 *
 * Use derived classes for specific events. Although looks exactly like
 * GeneralExceptions, the difference is that warnings are meant to be gently
 * caught without aborting the program execution. */
class GeneralWarning : public std::exception {
public:
    /** Default Constructor */
    GeneralWarning(){};

    /** Constructor for the general exception that accepts a message to print
    * \param [in] msg : the message to print before killing the execution */
    explicit GeneralWarning(const std::string& msg)
        : exception(), message_(msg) {}

    /** Destructor that throws the exception for us */
    virtual ~GeneralWarning() throw() {}

    /** Overloading of the what() method that outputs what the error was
     * \return the error */
    virtual const char* what() const throw() {
        return message_.c_str();
    }

protected:
    const std::string message_; //!< The message that is to be printed
};

/** \brief Read/write exception for all file operation related tasks. */
class IOException : public GeneralException {
public:
    /** Constructor taking the error message as the argument
    * \param [in] msg : the error message to output */
    IOException(const std::string& msg) :
        GeneralException(msg) {}
};

/** \brief Exception in the TreeCorrelator, both reading and parsing XML file,
 * as well as creating new places, accessing places etc. */
class TreeCorrelatorException : public GeneralException {
public:
    /** Constructor taking the error message as the argument
    * \param [in] msg : the error message to output */
    TreeCorrelatorException(const std::string& msg) :
        GeneralException(msg) {}
};

/** \brief Error in conversion of string to numerical or boolean value. */
class ConversionException : public GeneralException {
public:
    /** Constructor taking the error message as the argument
    * \param [in] msg : the error message to output */
    ConversionException(const std::string& msg) :
        GeneralException(msg) {}
};

/** \brief Error in creating or accesing histograms. */
class HistogramException : public GeneralException {
public:
    /** Constructor taking the error message as the argument
    * \param [in] msg : the error message to output */
    HistogramException(const std::string& msg) :
        GeneralException(msg) {}
};

/** \brief Use this exception if feature is not implemented yet. */
class NotImplemented : public GeneralException {
public:
    /** Constructor taking the error message as the argument
    * \param [in] msg : the error message to output */
    NotImplemented(const std::string& msg) :
        GeneralException(msg) {}
};

#endif
