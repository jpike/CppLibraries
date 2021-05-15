#pragma once

namespace MATH
{
    /// Utilities for working with numbers.
    /// Designed with templates to allow working with multiple different data types,
    /// but methods may not work as expected for all data types.
    class Number
    {
    public:
        template <typename NumberType>
        static bool IsEven(const NumberType number);

        template <typename NumberType>
        static NumberType DecrementAndWrapWithinRange(const NumberType value, const NumberType min_value, const NumberType max_value);
        template <typename NumberType>
        static NumberType IncrementAndWrapWithinRange(const NumberType value, const NumberType min_value, const NumberType max_value);

        template <typename NumericType>
        static NumericType Clamp(
            const NumericType number_to_clamp,
            const NumericType min_value,
            const NumericType max_value);
    };

    /// Determines if a number is even.
    /// @param[in]  number - The number to check.
    /// @return True if the number is even; false otherwise.
    template <typename NumberType>
    bool Number::IsEven(const NumberType number)
    {
        NumberType remainder = (number % 2);
        bool is_even = (0 == remainder);
        return is_even;
    }

    /// Decrements a number to keep it within the specified range,
    /// wrapping to the max value if the min value would be exceeded.
    /// @param[in]  value - The value to decrement but keep within the range.
    /// @param[in]  min_value - The minimum valid value within the range to keep the number in.
    /// @param[in]  max_value - The maximum valid value within the range to keep the number in.
    /// @return The provided number but decremented yet kept within the specified range.
    template <typename NumberType>
    NumberType Number::DecrementAndWrapWithinRange(const NumberType value, const NumberType min_value, const NumberType max_value)
    {
        // CHECK IF THE VALUE IS GREATER THAN THE MAX OF THE RANGE.
        bool value_greater_than_max = (value > max_value);
        if (value_greater_than_max)
        {
            // RETURN THE MAX VALUE.
            // If the number is greater than the max of the range, it needs to be clamped
            // to the max value to keep it within range.
            return max_value;
        }

        // CHECK IF DECREMENTING THE NUMBER WOULD PUT IT OUTSIDE OF THE RANGE.
        bool decrementing_would_put_outside_of_range = (value <= min_value);
        if (decrementing_would_put_outside_of_range)
        {
            // WRAP THE NUMBER TO THE MAX VALUE.
            return max_value;
        }
        else
        {
            // RETURN THE DECREMENTED VALUE.
            NumberType decremented_value = value - 1;
            return decremented_value;
        }
    }

    /// Increments a number to keep it within the specified range,
    /// wrapping to the min value if the max value would be exceeded.
    /// @param[in]  value - The value to increment but keep within the range.
    /// @param[in]  min_value - The minimum valid value within the range to keep the number in.
    /// @param[in]  max_value - The maximum valid value within the range to keep the number in.
    /// @return The provided number but incremented yet kept within the specified range.
    template <typename NumberType>
    NumberType Number::IncrementAndWrapWithinRange(const NumberType value, const NumberType min_value, const NumberType max_value)
    {
        // CHECK IF THE VALUE IS LESS THAN THE MIN OF THE RANGE.
        bool value_less_than_max = (value < min_value);
        if (value_less_than_max)
        {
            // RETURN THE MIN VALUE.
            // If the number is less than the min of the range, it needs to be clamped
            // to the min value to keep it within range.
            return min_value;
        }

        // CHECK IF INCREMENTING THE NUMBER WOULD PUT IT OUTSIDE OF THE RANGE.
        bool incrementing_would_put_outside_of_range = (value >= max_value);
        if (incrementing_would_put_outside_of_range)
        {
            // WRAP THE NUMBER TO THE MIN VALUE.
            return min_value;
        }
        else
        {
            // RETURN THE INCREMENTED VALUE.
            NumberType incremented_value = value + 1;
            return incremented_value;
        }
    }

    /// Clamps a numeric value to a range.
    /// \tparam NumericType - The type of the value being clamped.
    /// \param[in]  number_to_clamp - The number to clamp.
    /// \param[in]  min_value - The minimum (inclusive) value of the range to clamp to.
    /// \param[in]  max_value - The maximum (inclusive) value of the range to clamp to.
    /// \return The provided number clamped to the specified range.
    template <typename NumericType>
    NumericType Number::Clamp(
        const NumericType number_to_clamp,
        const NumericType min_value,
        const NumericType max_value)
    {
        // CLAMP THE NUMBER TO THE THE MIN VALUE IF IT'S TOO LOW.
        bool number_too_low = (number_to_clamp < min_value);
        if (number_too_low)
        {
            return min_value;
        }

        // CLAMP THE NUMBER TO THE MAX VALUE IF IT'S TOO HIGH.
        bool number_too_high = (number_to_clamp > max_value);
        if (number_too_high)
        {
            return max_value;
        }

        // RETURN THE NUMBER SINCE IT'S ALREADY CLAMPED WITHIN THE RANGE.
        return number_to_clamp;
    }
}
