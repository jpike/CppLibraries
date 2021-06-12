#pragma once

#include <memory>
#include <gsl/gsl>

/// Holds code related to computer memory.
namespace MEMORY
{
    /// A type alias to improve readability for non-null raw pointers.
    /// @tparam Type - The type of the underyling item being pointed to.
    template <typename Type>
    using NonNullRawPointer = gsl::strict_not_null<Type*>;

    /// A type alias to improve readability for non-null unique pointers.
    /// @tparam Type - The type of the underlying item in the pointer.
    template <typename Type>
    using NonNullUniquePointer = gsl::strict_not_null<std::unique_ptr<Type>>;

    /// A type alias to improve readability for non-null shared pointers.
    /// @tparam Type - The type of the underlying item in the pointer.
    template <typename Type>
    using NonNullSharedPointer = gsl::strict_not_null<std::shared_ptr<Type>>;
}
