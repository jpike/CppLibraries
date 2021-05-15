#pragma once

#include <stdexcept>
#include <string>

/// Holds code related to handling errors.
namespace ERROR_HANDLING
{
    /// Throws an invalid argument exception if the provided parameter is null.
    /// @tparam NullableType - The type of the nullable object to check.
    ///     This is declared to include the entire pointer type (rather
    ///     than just the type of the pointed-to value) in order to support
    ///     different types of pointers (regular, shared, unique, etc.).
    /// @param[in]  nullable - The nullable to check.
    /// @param[in]  exception_message - The message to include in the exception,
    ///     if thrown.
    /// @throws std::invalid_argument - Thrown if the nullable is null.
    template <typename NullableType>
    void ThrowInvalidArgumentExceptionIfNull(
        const NullableType& nullable,
        const std::string& exception_message)
    {
        bool is_null = (nullptr == nullable);
        if (is_null)
        {
            throw std::invalid_argument(exception_message);
        }
    }
}
