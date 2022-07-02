#pragma once

#include "engine/core/allocator/MemoryManager.h"

namespace longmarch
{
    struct LongMarch_ReferenceCounter
    {
        std::atomic_uint_fast32_t m_refCounter { 0 };
    };
    
    template <typename T>
    concept ReferenceCountable = requires(T&& t)
    {
        { t.IncrementRef() };
    }
    && requires(T&& t)
    {
        { t.DecrementRef() } -> std::convertible_to<bool>;
    };

    /*
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
    template <class T>
        requires ReferenceCountable<T>
    class RefPtr
    {
    public:
        // Big Five----------------------------------------------------------------------------------------------------

        RefPtr(const T& ref) noexcept
        {
            m_ptr = &ref;
            m_ptr->IncrementRef();
        }

        ~RefPtr() noexcept
        {
            if (m_ptr && !m_ptr->DecrementRef())
            {
                MemoryManager::Delete(m_ptr);
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

        template <class U>
        RefPtr(const RefPtr<U>& other) noexcept
        {
            m_ptr = (T*)other.m_ptr;
            m_ptr->IncrementRef();
        }

        template <class U>
        RefPtr(RefPtr<U>&& other) noexcept
        {
            m_ptr = (T*)other.m_ptr;
            other.m_ptr = nullptr;
        }

        template <class U>
        RefPtr& operator=(const RefPtr<U>& other) noexcept
        {
            if (this == &other)
                return *this;
            m_ptr = (T*)other.m_ptr;
            m_ptr->IncrementRef();
            return *this;
        }

        template <class U>
        RefPtr& operator=(RefPtr<U>&& other) noexcept
        {
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

        [[nodiscard]] T* Get() noexcept
        {
            return m_ptr;
        }

        [[nodiscard]] T& Ref() noexcept
        {
            return *m_ptr;
        }

    private:
        T* m_ptr{nullptr};
    };
}
