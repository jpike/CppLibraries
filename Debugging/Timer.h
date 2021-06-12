#pragma once

#include <chrono>
#include <iostream>
#include <string>

namespace DEBUGGING
{
    template<typename ClockType = std::chrono::high_resolution_clock>
    class Timer
    {
    public:
        explicit Timer(const std::string& name) :
            Name(name),
            StartTime(ClockType::now())
        {}

        ~Timer()
        {
            std::cout << GetElapsedTimeText() << std::endl;
        }

        std::string GetElapsedTimeText() const
        {
            auto current_time = ClockType::now();
            auto elapsed_time = current_time - StartTime;
            std::string elapsed_time_text = (
                Name + ": " +
                std::to_string(elapsed_time.count()) + "\t" +
                std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_time).count()) + "ms\t" +
                std::to_string(std::chrono::duration_cast<std::chrono::seconds>(elapsed_time).count()) + "s");
            return elapsed_time_text;
        }

        std::string Name = {};
        std::chrono::time_point<ClockType> StartTime = {};
    };

    using HighResolutionTimer = Timer<std::chrono::high_resolution_clock>;
    using SystemClockTimer = Timer<std::chrono::system_clock>;
}
