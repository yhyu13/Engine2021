#pragma once

#include <chrono>

namespace longmarch
{
    /**
     * @brief Timer that set and return time point in sec/millisec, etc.
     *
     * @author Hang Yu (yohan680919@gmail.com)
     */
    class Timer
    {
    public:
        Timer()
        {
            m_last = std::chrono::steady_clock::now();
        }

        //! Init timer with period in secs
        explicit Timer(double period)
            :
            m_period(period)
        {
            m_last = std::chrono::steady_clock::now();
        }

        //! Reset timer to now
        inline void Reset() noexcept
        {
            m_last = std::chrono::steady_clock::now();
        }

        //! Templated method that return time between now and the last reset, default in sec.
        template <typename ratio = std::ratio<1>, typename ret_type = double>
        inline ret_type Mark() noexcept
        {
            return std::chrono::duration<ret_type, ratio>(std::chrono::steady_clock::now() - m_last).count();
        }

        //! Return time in seconds between now and the last reset
        inline double MarkSec() noexcept
        {
            return Mark<std::ratio<1>, double>();
        }

        inline double MarkMilli() noexcept
        {
            return Mark<std::milli, double>();
        }

        inline double MarkMicro() noexcept
        {
            return Mark<std::micro, double>();
        }

        inline double MarkNano() noexcept
        {
            return Mark<std::nano, double>();
        }

        //! Set period. Use Check to check if period is met.
        inline void SetPeriod(double period) noexcept
        {
            m_period = period;
        }

        //! Check if period is met. Use SetPeriod to set the period. If period smaller than 0, always return false. If period is equal to 0, always return true
        inline bool Check(bool reset) noexcept
        {
            if (m_period < 0.0)
            {
                return false;
            }
            else if (m_period == 0.0)
            {
                return true;
            }
            else if (Mark() < m_period)
            {
                return false;
            }
            else if (reset)
            {
                Reset();
            }
            return true;
        }

    private:
        std::chrono::steady_clock::time_point m_last;
        double m_period{0.0};
    };

    /**
     * @brief Timer that set and return time point in sec/millisec, etc. LazyInitializedTimer will now initialized until user called Reset() or Mark()
     *        This class has the advantage of trivial construction (save couple hundred clocks) for counting down only when you need it. 
     *
     * @author Hang Yu (yohan680919@gmail.com)
     */
    class LazyInitializedTimer
    {
    public:
        LazyInitializedTimer() = default;

        //! Init timer with period in secs
        explicit LazyInitializedTimer(double period)
            :
            m_period(period)
        {
        }

        //! Reset timer to now
        inline void Reset() noexcept
        {
            m_last = std::chrono::steady_clock::now();
            m_init = true;
        }

        //! Templated method that return time between now and the last reset, default in sec.
        template <typename ratio = std::ratio<1>, typename ret_type = double>
        inline ret_type Mark() noexcept 
        {
            if (!m_init)
            {
                Reset();
            }
            return std::chrono::duration<ret_type, ratio>(std::chrono::steady_clock::now() - m_last).count();
        }

        //! Return time in seconds between now and the last reset
        inline double MarkSec() noexcept
        {
            return Mark<std::ratio<1>, double>();
        }

        inline double MarkMilli() noexcept
        {
            return Mark<std::milli, double>();
        }

        inline double MarkMicro() noexcept
        {
            return Mark<std::micro, double>();
        }

        inline double MarkNano() noexcept
        {
            return Mark<std::nano, double>();
        }

        //! Set period. Use Check to check if period is met.
        inline void SetPeriod(double period) noexcept
        {
            m_period = period;
        }

        //! Check if period is met. Use SetPeriod to set the period. If period smaller than 0, always return false. If period is equal to 0, always return true
        inline bool Check(bool reset) noexcept
        {
            if (m_period < 0.0)
            {
                return false;
            }
            else if (m_period == 0.0)
            {
                return true;
            }
            else if (Mark() < m_period)
            {
                return false;
            }
            else if (reset)
            {
                Reset();
            }
            return true;
        }

    private:
        std::chrono::steady_clock::time_point m_last;
        double m_period{0.0};
        bool m_init{false};
    };
}
