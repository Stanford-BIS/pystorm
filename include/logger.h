#ifndef LOGGER_H
#define LOGGER_H
//
// This file declares and defines the Logging system for PyStorm.
//
// The code is subdivided into the following sections.
//    Policies - logger policies
//    Loggers  - the Logger generic class and the concrete classes
//        using the policies.
//    Loggable Interface - the interface that must be provided
//        a classes that will use the operator<< functions directly.
//    Stream operators - Stream operators that can stream messages
//        to a Logger (or nothing if logging is not enabled).

#include <iostream>
#include <boost/current_function.hpp>

//////////////////////////////////
// Policies                     //
//////////////////////////////////

/// 
/// InfoLogPolicy is a functor that is passed as a template
/// parameter to Logger allowing the Logger to structure
/// messages that provide generic information.
/// 
struct InfoLogPolicy {
    static void log(const std::string& msg) {
        std::cout << "INFO: " << msg << std::endl;
    }
};

/// 
/// WarningLogPolicy is a functor that is passed as a template
/// parameter to Logger allowing the Logger to structure
/// messages that provide generic warnings.
/// 
struct WarningLogPolicy {
    static void log(const std::string& msg) {
        std::cout << "WARNING: " << msg << std::endl;
    }
};

/// 
/// ErrorLogPolicy is a functor that is passed as a template
/// parameter to Logger allowing the Logger to structure
/// messages that tells the user an error has occurred.
/// 
struct ErrorLogPolicy {
    static void log(const std::string& msg) {
        std::cout << "ERROR: " << msg << std::endl;
    }
};

//////////////////////////////////
// Loggers                      //
//////////////////////////////////

///
/// Logger is a generic class used to stream information to 
/// standard output.
/// 
/// The class is provided an object implementing the log function
/// calling the log function in it's own log method.
/// The class is meant to be used in conjuction of the LogPolicy 
/// types that follow (e.g. InfoLogPolicy and WarningLogPolicy).
/// In addition, there are two helper functions implementing
/// operators that allow a developer to use a Logger instance
/// in a "stream"-style (e.g. aLogger << "A Message").
/// The operators also have the added advantage that if the
/// variable LOG_ENABLED is not defined, the body of the 
/// operators will be empty and the compiler can optimize
/// the function calls out of the binary. The binary will not 
/// make the function calls.
///
template< typename LogPolicy >
class Logger : public LogPolicy {
public:
    explicit Logger() {
        // This allows us to use BOOST_CURRENT_FUNCTION to print
        // the function the Logger is being called from.
        boost::detail::current_function_helper();
    }
    using LogPolicy::log;

    void log(const std::string& msg) {
        LogPolicy::log(msg);
    }
};

///
/// InfoLogger is a Logger using the InfoLogPolicy
///
typedef Logger<InfoLogPolicy>    InfoLogger;

///
/// WarningLogger is a Logger using the WarningLogPolicy
///
typedef Logger<WarningLogPolicy> WarningLogger;

///
/// ErrorLogger is a Logger using the ErrorLogPolicy
///
typedef Logger<ErrorLogPolicy>   ErrorLogger;

//////////////////////////////////
// Loggable Interface           //
//////////////////////////////////

/// 
/// LoggableIfc is an interface whose functions
/// allow a class to use the operator<< functions directly.
/// For example, an object whose type is a class implementing
/// LoggableIf could use the function operator<< as follows:
///
///   anInfoLogger << objectOfTypeLoggerIfc;
/// 
struct LoggableIfc {
    virtual std::string toString() const = 0;
};

//////////////////////////////////
// Stream operators             //
//////////////////////////////////

//
// When compiling, not defining the variable LOG_ENABLED 
// will declare the following operator<< functions with 
// empty bodies. Empty bodied functions can be optimized out
// during compilation, therefore, adding logging messages will
// not affect performance of released code.
//
#ifdef LOG_ENABLED

///
/// The overloaded function takes a Logger and a string
/// and streams the string using the Logger.
///
template< typename Logger >
void operator<<(Logger& log, const std::string& msg) {
    log.log(msg);
}

///
/// The overloaded function takes a Logger and a Loggable
/// object and streams a message provided the object
/// using the Logger.
///
template< typename Logger >
void operator<<(Logger& log, const LoggableIfc& obj) {
    log.log(obj.toString());
}

#else

///
/// The overloaded function does nothing. Compiler optimizations
/// will remove calls to these functions.
///
template < typename Logger >
void operator<<(Logger& log, const std::string& msg) {
}

///
/// The overloaded function does nothing. Compiler optimizations
/// will remove calls to these functions.
///
template< typename Logger >
void operator<<(Logger& log, const LoggableIfc& object) {
}

#endif // if LOG_ENABLED

#endif // ifdef LOGGER_H
