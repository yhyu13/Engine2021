#pragma once
#include "../EngineCore.h"
#include <queue>
#include <atomic>

namespace longmarch
{
	extern std::atomic_flag stbi_hdr_write_lock;
	extern std::atomic_flag stbi_png_write_lock;

	struct atomic_flag_guard {
		NONCOPYABLE(atomic_flag_guard);
		atomic_flag_guard() = delete;
		explicit atomic_flag_guard(std::atomic_flag& flag) noexcept;
		~atomic_flag_guard() noexcept;
	private:
		std::atomic_flag* m_lock;
	};

	struct atomic_bool_guard {
		NONCOPYABLE(atomic_bool_guard);
		atomic_bool_guard() = delete;
		explicit atomic_bool_guard(std::atomic_bool& flag) noexcept;
		~atomic_bool_guard() noexcept;
	private:
		std::atomic_bool* m_lock;
	};

	/*
		Base atomic class for class static method
	*/
	struct BaseAtomicClassStatic
	{
	public:
		virtual ~BaseAtomicClassStatic() = default;
#define LOCK_GUARD_S() atomic_flag_guard __lock(sc_flag)
		static void LockS() noexcept;
		static void UnlockS() noexcept;
	protected:
		inline static std::atomic_flag sc_flag; // c++ 20 default initialization to false
	};

	/*
		Base atomic class for NON-INSTANTIABLE classes
	*/
	struct BaseAtomicClassNI
	{
	public:
		NONINSTANTIABLE(BaseAtomicClassNI);
#define LOCK_GUARD_NI() atomic_flag_guard __lock(ni_flag)
		static void LockNI() noexcept;
		static void UnlockNI() noexcept;
	protected:
		inline static std::atomic_flag ni_flag; // c++ 20 default initialization to false
	};

	/*
		Base atomic class for NON-COPYABLE classes
	*/
	struct BaseAtomicClassNC
	{
	public:
		NONCOPYABLE(BaseAtomicClassNC);

#define LOCK_GUARD_NC() atomic_flag_guard __lock(nc_flag)

		BaseAtomicClassNC() noexcept
		{
			//nc_flag.clear();
		};
		virtual ~BaseAtomicClassNC() noexcept = default;

		void LockNC() const noexcept;
		void UnlockNC() const noexcept;

	protected:
		mutable std::atomic_flag nc_flag;
	};

	/*
		Base atomic class for COPYABLE classes
	*/
	struct BaseAtomicClass2
	{
	public:

#define LOCK_GUARD2() atomic_flag_guard __lock(m_flag)

		BaseAtomicClass2() noexcept {//m_flag.clear();
		}
		virtual ~BaseAtomicClass2() noexcept = default;
		BaseAtomicClass2(const BaseAtomicClass2& other) noexcept {//m_flag.clear();
		}
		BaseAtomicClass2(BaseAtomicClass2&& other) noexcept {//m_flag.clear();
		}
		BaseAtomicClass2& operator=(const BaseAtomicClass2& rhs) noexcept { return *this; }
		BaseAtomicClass2& operator=(BaseAtomicClass2&& rhs) noexcept { return *this; }

		void Lock2() const noexcept;
		void Unlock2() const noexcept;

	protected:
		mutable std::atomic_flag m_flag;
	};

	template<typename T>
	class AtomicQueue final : public BaseAtomicClassNC
	{
	public:
		NONCOPYABLE(AtomicQueue);
		AtomicQueue() = default;

		void push(T&& value) noexcept
		{
			LOCK_GUARD_NC();
			m_queque.emplace_back(value);
		}

		void push(const T& value) noexcept
		{
			LOCK_GUARD_NC();
			m_queque.emplace_back(value);
		}

		T& front() noexcept
		{
			LOCK_GUARD_NC();
			return m_queque.front();
		}

		const T& front() const noexcept
		{
			LOCK_GUARD_NC();
			return m_queque.front();
		}

		void pop() noexcept
		{
			LOCK_GUARD_NC();
			m_queque.pop_front();
		}

		bool empty() const noexcept
		{
			LOCK_GUARD_NC();
			return m_queque.empty();
		}

		bool contains(const T& value) const noexcept
		{
			LOCK_GUARD_NC();
			return std::find(m_queque.begin(), m_queque.end(), value) != m_queque.end();
		}

		size_t size() noexcept
		{
			LOCK_GUARD_NC();
			return m_queque.size();
		}

		auto begin() noexcept
		{
			LOCK_GUARD_NC();
			return m_queque.begin();
		}

		auto end() noexcept
		{
			LOCK_GUARD_NC();
			return m_queque.end();
		}

		auto begin() const noexcept
		{
			LOCK_GUARD_NC();
			return m_queque.begin();
		}

		auto end() const noexcept
		{
			LOCK_GUARD_NC();
			return m_queque.end();
		}

		auto cbegin() const noexcept
		{
			LOCK_GUARD_NC();
			return m_queque.cbegin();
		}

		auto cend() const noexcept
		{
			LOCK_GUARD_NC();
			return m_queque.cend();
		}

	private:
		std::deque<T> m_queque;
	};
}
