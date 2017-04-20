#include <iostream>
#include "gtest/gtest.h"
#include "logger.h"

class LoggableObject : public LoggableIfc {
public:
    LoggableObject(std::string msg) : m_message(msg) {
    }
    std::string toString() const {
        std::string retValue = m_message;
        return retValue;
    }

protected:
    std::string m_message;
};

TEST(LoggerTest, HandlesInfoLogWithStringMessage){
    InfoLogger logger;
    std::stringstream buffer;
    std::string correctString = "INFO: A Message\n";

    // Save cout's current buffer
    std::streambuf *sbuf = std::cout.rdbuf();

    // Redirect cout to buffer
    std::cout.rdbuf(buffer.rdbuf());

    // Logger will print to buffer
    logger << "A Message";

    // Compare buffer's string to the correct string
    ASSERT_EQ(buffer.str(),correctString);

    // Reset cout to point to it's old buffer
    std::cout.rdbuf(sbuf);
}

TEST(LoggerTest, HandlesWarningLogWithStringMessage){
    WarningLogger logger;
    std::stringstream buffer;
    std::string correctString = "WARNING: A Message\n";

    std::streambuf *sbuf = std::cout.rdbuf();
    std::cout.rdbuf(buffer.rdbuf());
    logger << "A Message";
    ASSERT_EQ(buffer.str(),correctString);
    std::cout.rdbuf(sbuf);
}

TEST(LoggerTest, HandlesErrorLogWithStringMessage){
    ErrorLogger logger;
    std::stringstream buffer;
    std::string correctString = "ERROR: A Message\n";

    std::streambuf *sbuf = std::cout.rdbuf();
    std::cout.rdbuf(buffer.rdbuf());
    logger << "A Message";
    ASSERT_EQ(buffer.str(),correctString);
    std::cout.rdbuf(sbuf);
}

TEST(LoggerTest, HandlesInfoLogWithLoggableObject){
    std::string msg = "A Message from a LoggableObject";
    InfoLogger logger;
    LoggableObject obj(msg);
    std::stringstream buffer;
    std::string correctString = "INFO: A Message from a LoggableObject\n";

    std::streambuf *sbuf = std::cout.rdbuf();
    std::cout.rdbuf(buffer.rdbuf());
    logger << obj;
    ASSERT_EQ(buffer.str(),correctString);
    std::cout.rdbuf(sbuf);
}

TEST(LoggerTest, HandlesWarningLogWithLoggableObject){
    std::string msg = "A Message from a LoggableObject";
    WarningLogger logger;
    LoggableObject obj(msg);
    std::stringstream buffer;
    std::string correctString = "WARNING: A Message from a LoggableObject\n";

    std::streambuf *sbuf = std::cout.rdbuf();
    std::cout.rdbuf(buffer.rdbuf());
    logger << obj;
    ASSERT_EQ(buffer.str(),correctString);
    std::cout.rdbuf(sbuf);
}

TEST(LoggerTest, HandlesErrorLogWithLoggableObject){
    std::string msg = "A Message from a LoggableObject";
    ErrorLogger logger;
    LoggableObject obj(msg);
    std::stringstream buffer;
    std::string correctString = "ERROR: A Message from a LoggableObject\n";

    std::streambuf *sbuf = std::cout.rdbuf();
    std::cout.rdbuf(buffer.rdbuf());
    logger << obj;
    ASSERT_EQ(buffer.str(),correctString);
    std::cout.rdbuf(sbuf);
}
