#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <stdexcept>
#include <string>

/** Exception with customizable message. */
class GeneralException : public std::exception {
public:
    explicit GeneralException(const std::string& msg) 
        : exception(), message_(msg) {}

    virtual ~GeneralException() throw() {}

    virtual const char* what() const throw()
    {
        return message_.c_str();
    }

private:
    const std::string message_;
};

#endif
