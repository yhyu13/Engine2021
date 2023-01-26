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
            Reset();
        }

        //! Init timer with period in secs
        explicit Timer(double period)
            :
            m_period(period)
        {
            Reset();
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
        inline double MarkSec(bool reset = false) noexcept
        {
            const auto ret = Mark<std::ratio<1>, double>();
            if (reset)
            {
                Reset();
            }
            return ret;
        }

        inline double MarkMilli(bool reset = false) noexcept
        {
            const auto ret = Mark<std::milli, double>();
            if (reset)
            {
                Reset();
            }
            return ret;
        }

        inline double MarkMicro(bool reset = false) noexcept
        {
            const auto ret = Mark<std::micro, double>();
            if (reset)
            {
                Reset();
            }
            return ret;
        }

        inline double MarkNano(bool reset = false) noexcept
        {
            const auto ret = Mark<std::nano, double>();
            if (reset)
            {
                Reset();
            }
            return ret;
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
        }

        //! Templated method that return time between now and the last reset, default in sec.
        template <typename ratio = std::ratio<1>, typename ret_type = double>
        inline ret_type Mark() noexcept 
        {
            if (!m_init)
            {
                Reset();
                m_init = true;
            }
            return std::chrono::duration<ret_type, ratio>(std::chrono::steady_clock::now() - m_last).count();
        }

        //! Return time in seconds between now and the last reset
        inline double MarkSec(bool reset = false) noexcept
        {
            const auto ret = Mark<std::ratio<1>, double>();
            if (reset)
            {
                Reset();
            }
            return ret;
        }

        inline double MarkMilli(bool reset = false) noexcept
        {
            const auto ret = Mark<std::milli, double>();
            if (reset)
            {
                Reset();
            }
            return ret;
        }

        inline double MarkMicro(bool reset = false) noexcept
        {
            const auto ret = Mark<std::micro, double>();
            if (reset)
            {
                Reset();
            }
            return ret;
        }

        inline double MarkNano(bool reset = false) noexcept
        {
            const auto ret = Mark<std::nano, double>();
            if (reset)
            {
                Reset();
            }
            return ret;
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
