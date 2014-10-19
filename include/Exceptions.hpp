#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <stdexcept>
#include <string>

/** Exception with customizable message. Use derived classes for specific
 * exceptions.*/
class GeneralException : public std::exception {
public:
    explicit GeneralException(const std::string& msg) 
        : exception(), message_(msg) {}

    virtual ~GeneralException() throw() {}

    virtual const char* what() const throw()
    {
        return message_.c_str();
    }

protected:
    const std::string message_;
};

/** Warning with customizable message. Use derived classes for specific
 * events. Although looks excatly like GeneralExceptions, the difference is
 * that warnings are meant to be gently caught without aborting the program
 * execution.*/
class GeneralWarning : public std::exception {
public:
    explicit GeneralWarning(const std::string& msg) 
        : exception(), message_(msg) {}

    virtual ~GeneralWarning() throw() {}

    virtual const char* what() const throw()
    {
        return message_.c_str();
    }

protected:
    const std::string message_;
};

/** Read/write exception for all file operation related tasks. */
class IOException : public GeneralException {
    public:
        IOException(const std::string& msg) : 
            GeneralException(msg) {}
};

/** Exception in the TreeCorrelator, both reading and parsing XML file, 
 * as well as creating new places, accessing places etc. */
class TreeCorrelatorException : public GeneralException {
    public:
        TreeCorrelatorException(const std::string& msg) : 
            GeneralException(msg) {}
};

/** Error in conversion of string to numerical or boolean value. */
class ConversionException : public GeneralException {
    public:
        ConversionException(const std::string& msg) : 
            GeneralException(msg) {}
};

/** Error in creating or accesing histograms. */
class HistogramException : public GeneralException {
    public:
        HistogramException(const std::string& msg) : 
            GeneralException(msg) {}
};

/** Use this exception is feature is not implemented yet. */
class NotImplemented : public GeneralException {
    public:
        NotImplemented(const std::string& msg) : 
            GeneralException(msg) {}
};

#endif
