#pragma once

#include <stdexcept>
#include "ErrorHandling/NullChecking.h"

/// A namespace for testing the code in the corresponding file.
namespace NULL_CHECKING_TESTS
{
    TEST_CASE("An exception is thrown for null.", "[ThrowInvalidArgumentExceptionIfNull]")
    {
        // CALL THE FUNCTION WITH NULL.
        bool exception_thrown = false;
        try
        {
            ERROR_HANDLING::ThrowInvalidArgumentExceptionIfNull(nullptr, "Null");
        }
        catch (const std::invalid_argument&)
        {
            exception_thrown = true;
        }
        
        // VALIDATE AN EXCEPTION WAS THROWN.
        REQUIRE(exception_thrown);
    }

    TEST_CASE("No exception is thrown for non-null.", "[ThrowInvalidArgumentExceptionIfNull]")
    {
        // CALL THE FUNCTION WITH A NON-NULL VALUE.
        bool exception_thrown = false;
        try
        {
            int non_null = 1;
            ERROR_HANDLING::ThrowInvalidArgumentExceptionIfNull(&non_null, "Non-null");
        }
        catch (const std::invalid_argument&)
        {
            exception_thrown = true;
        }
        
        // VALIDATE NO EXCEPTION WAS THROWN.
        REQUIRE(!exception_thrown);
    }
}
