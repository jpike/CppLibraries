#pragma once

#include <array>
#include <cstddef>

namespace CONTAINERS
{
    /// A type alias for more easily declaring nested arrays.
    /// \tparam ElementType - The type of elements in the array.
    /// \tparam OuterArraySize - The number of "inner arrays" within the outer array.
    /// \tparam InnerArraySize - The number of elements within each inner array.
    template <typename ElementType, std::size_t OuterArraySize, std::size_t InnerArraySize>
    using NestedArray = std::array<std::array<ElementType, InnerArraySize>, OuterArraySize>;

    /// A type alias for a nested array where indices are determined by enums.
    /// Enum types are required to have a `COUNT` item that indicates the number of items for that enum type.
    /// \tparam ElementType - The type of elements in the array.
    /// \tparam OuterEnumType - The enum type used for the first indices into the array.
    /// \tparam InnerEnumType - The enum type used for the second indices into the array.
    template <typename ElementType, typename OuterEnumType, typename InnerEnumType>
    using NestedEnumArray = NestedArray<ElementType, OuterEnumType::COUNT, InnerEnumType::COUNT>;
}
