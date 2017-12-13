#pragma once

#include "gtest/gtest.h"
#include <iostream>
#include <sstream>

namespace Common {

/** Base helper to redirect std::cerr to a string while the object is alive. */
class ErrorRedirect
{
protected:
    ErrorRedirect();
    ~ErrorRedirect();

    /** @return the collected error output */
    const std::ostringstream& getErrors() const;

private:
    std::streambuf* originalCerrStreamBuffer_;
    std::ostringstream interceptedCerr_;
};

/**
 * Test that some error output was produced.
 * Objects can be stacked to further intercept output.
 */
class ExpectErrorLog : private ErrorRedirect
{
public:
    ExpectErrorLog();
    ~ExpectErrorLog();
};

/**
 * Test that no error output was produced
 * Objects can be stacked to further intercept output.
 */
class ExpectNoErrorLogs : private ErrorRedirect
{
public:
    ExpectNoErrorLogs();
    ~ExpectNoErrorLogs();
};

// ErrorRedirect implementation
inline ErrorRedirect::ErrorRedirect() : originalCerrStreamBuffer_{std::cerr.rdbuf()}, interceptedCerr_{}
{
    std::cerr.rdbuf(interceptedCerr_.rdbuf());
}

inline ErrorRedirect::~ErrorRedirect()
{
    std::cerr.rdbuf(originalCerrStreamBuffer_);
}

inline const std::ostringstream& ErrorRedirect::getErrors() const
{
    return interceptedCerr_;
}

// ExpectErrorLog implementation
inline ExpectErrorLog::ExpectErrorLog() : ErrorRedirect() {}

inline ExpectErrorLog::~ExpectErrorLog()
{
    EXPECT_FALSE(getErrors().str().empty());
}

// ExpectNoErrorLogs implementation
inline ExpectNoErrorLogs::ExpectNoErrorLogs() : ErrorRedirect() {}

inline ExpectNoErrorLogs::~ExpectNoErrorLogs()
{
    if (!getErrors().str().empty())
    {
        std::cout << "Unexpected errors:" << std::endl << getErrors().str() << std::endl;
    }
    EXPECT_TRUE(getErrors().str().empty());
}

} // namespace Common
