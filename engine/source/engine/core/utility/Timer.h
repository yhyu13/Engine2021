#pragma once
#include "../exception/EngineException.h"
#include <chrono>

namespace longmarch
{
	/**
	 * @brief Timer that set and return time point in sec
	 *
	 * @author Hang Yu (yohan680919@gmail.com)
	 */
	class Timer
	{
	public:
		Timer()
		{
			ENGINE_EXCEPT_IF(std::chrono::steady_clock::is_steady == false, L"C++11 std::chrono::steady_clock::is_steady() returns FALSE.");
			m_last = std::chrono::steady_clock::now();
		}
		//! Init timer with period in secs
		explicit Timer(double period)
			:
			m_period(period)
		{
			ENGINE_EXCEPT_IF(std::chrono::steady_clock::is_steady == false, L"C++11 std::chrono::steady_clock::is_steady() returns FALSE.");
			m_last = std::chrono::steady_clock::now();
		}
		//! Reset timer to now
		inline void Reset() noexcept
		{
			m_last = std::chrono::steady_clock::now();
		}
		//! Return time in seconds between now and the last reset
		inline double Mark() noexcept
		{
			return std::chrono::duration<double>(std::chrono::steady_clock::now() - m_last).count();
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
		double m_period = { 0.0 };
	};

	/**
	 * @brief Timer that shall be increamend manually, return time in any units interpreted by the user
	 *
	 * @author Hang Yu (yohan680919@gmail.com)
	 */
	class Timer2
	{
	public:
		Timer2() = default;

		explicit Timer2(double period)
			:
			m_period(period)
		{}
		//! Increment timer by dt
		inline void Increment(double dt) noexcept
		{
			m_now += dt;
		}
		//! Reset timer to now
		inline void Reset() noexcept
		{
			m_now = 0.0;
		}
		//! Return time in seconds between now and the last reset
		inline double Mark() noexcept
		{
			return m_now;
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
		double m_now{ 0.0 };
		double m_period = { 0.0 };
	};
}