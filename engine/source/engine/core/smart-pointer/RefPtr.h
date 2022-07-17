﻿#pragma once

#include "engine/core/allocator/MemoryManager.h"

namespace longmarch
{
#define ADD_REFERENCE_COUTNER() \
    public: \
    inline void IncrementRef() noexcept  \
    {\
        ++m_refCounter;\
    }\
    inline bool DecrementRef() noexcept\
    {\
        return --m_refCounter > 0;\
    }\
    private: \
    std::atomic_uint_fast32_t m_refCounter { 0 };

    // Concept for reference countable classes
    template <typename T>
    concept ReferenceCountable = requires(T&& t)
    {
        { t.IncrementRef() };
    }
    && requires(T&& t)
    {
        { t.DecrementRef() } -> std::convertible_to<bool>;
    };

    /**
     *  @brief A reference pointer requires template class instance method (1) Increment reference (2) Decrement reference
     *  The template class should maintain a mutable atomic reference counter, and the template class should align itself to avoid
     *  false sharing
     *
     *  The polymorphism casting uses explicit casting from U to T (i.e. (T*)(U* ptr)).
     *
     *  @detail On construction, the RefPtr increment the reference counting
     *          On destruction, the RefPtr tries to decrement the reference counting
     * 
     */
    template <class T, void Delete(T*)>
        requires ReferenceCountable<T>
    class RefPtr
    {
    public:
        // Big Five----------------------------------------------------------------------------------------------------

        RefPtr(T* ref) noexcept
            : m_ptr(ref)
        {
            m_ptr->IncrementRef();
        }

        ~RefPtr() noexcept
        {
            if (m_ptr && !m_ptr->DecrementRef())
            {
                Delete(m_ptr);
                m_ptr = nullptr;
            }
        }

        RefPtr(const RefPtr& other) noexcept
            : m_ptr(other.m_ptr)
        {
            m_ptr->IncrementRef();
        }

        RefPtr(RefPtr&& other) noexcept
            : m_ptr(other.m_ptr)
        {
            other.m_ptr = nullptr;
        }

        RefPtr& operator=(const RefPtr& other) noexcept
        {
            if (this == &other)
                return *this;
            m_ptr = other.m_ptr;
            m_ptr->IncrementRef();
            return *this;
        }

        RefPtr& operator=(RefPtr&& other) noexcept
        {
            if (this == &other)
                return *this;
            m_ptr = other.m_ptr;
            other.m_ptr = nullptr;
            return *this;
        }

        // Polymorphism Casting---------------------------------------------------------------------------------------------

        template <class U, void Delete2(T*)>
        RefPtr(const RefPtr<U, Delete2>& other) noexcept
        {
            static_assert(std::is_same_v<T,U> && Delete==Delete2, "RefPtr Polymorphism Casting not allowed!");
            m_ptr = (T*)other.m_ptr;
            m_ptr->IncrementRef();
        }

        template <class U, void Delete2(T*)>
        RefPtr(RefPtr<U, Delete2>&& other) noexcept
        {
            static_assert(std::is_same_v<T,U> && Delete==Delete2, "RefPtr Polymorphism Casting not allowed!");
            m_ptr = (T*)other.m_ptr;
            other.m_ptr = nullptr;
        }

        template <class U, void Delete2(T*)>
        RefPtr& operator=(const RefPtr<U, Delete2>& other) noexcept
        {
            static_assert(std::is_same_v<T,U> && Delete==Delete2, "RefPtr Polymorphism Casting not allowed!");
            if (this == &other)
                return *this;
            m_ptr = (T*)other.m_ptr;
            m_ptr->IncrementRef();
            return *this;
        }

        template <class U, void Delete2(T*)>
        RefPtr& operator=(RefPtr<U, Delete2>&& other) noexcept
        {
            static_assert(std::is_same_v<T,U> && Delete==Delete2, "RefPtr Polymorphism Casting not allowed!");
            if (this == &other)
                return *this;
            m_ptr = (T*)other.m_ptr;
            other.m_ptr = nullptr;
            return *this;
        }

        // Operator overloading---------------------------------------------------------------------------------------------

        T* operator->() noexcept
        {
            return m_ptr;
        }

        T& operator*() noexcept
        {
            return *m_ptr;
        }

        // Getter-----------------------------------------------------------------------------------------------------------

        [[nodiscard]] T* get() noexcept
        {
            return m_ptr;
        }

    private:
        T* m_ptr{nullptr};
    };
}
