#pragma once

#include <algorithm>

namespace CONTAINERS
{
    /// Holds generic code related to containers (collections).
    /// Designed to provide easier-to-use interfaces for common container types.
    class Container
    {
    public:
        template <typename ContainerType, typename ValueType>
        static bool Contains(const ContainerType& container, const ValueType& value);
    };

    /// Checks if a container contains a specified value.
    /// @tparam ContainerType - The type of the container to check.
    /// @tparam ValueType - The type of value to held in the container.
    /// @param[in]  container - The container to search through.
    /// @param[in]  value - The value to search for.
    /// @return True if the value is in the container; false if not.
    template <typename ContainerType, typename ValueType>
    bool Container::Contains(const ContainerType& container, const ValueType& value)
    {
        const auto found_value = std::find(container.cbegin(), container.cend(), value);
        bool value_in_container = (container.cend() != found_value);
        return value_in_container;
    }
}

