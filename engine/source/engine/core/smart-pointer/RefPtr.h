#pragma once

#include "engine/core/allocator/TemplateMemoryManager.h"

namespace longmarch
{
    // A base class for reference counting which supports non-copyable inherited classes.
    struct MS_ALIGN8 BaseRefCountClassNC
    {
    public:
        BaseRefCountClassNC() = default;
        NONCOPYABLE(BaseRefCountClassNC);
        
        inline void IncrementRef() const noexcept
        {
            m_refCounter.fetch_add(1, std::memory_order_relaxed);
        }

        inline bool DecrementRef() const noexcept
        {
            return m_refCounter.fetch_sub(1, std::memory_order_relaxed) > 1;
        }

    protected:
        // yuhang : protected non-virtual destructor that prevents delete by base pointer
        ~BaseRefCountClassNC() noexcept = default;

    private:
        mutable std::atomic_uint_fast32_t m_refCounter{0};
    };

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
     *  false sharing.
     *
     *
     *  @detail On construction, the RefPtr increment the reference counting.
     *          On destruction, the RefPtr tries to decrement the reference counting.
     *
     *  @attention Only those T* created by RefPtr<T>::Allocator::New can be managed by RefPtr<T>.
     * 
     */
    template <class T, typename Allocator_T = TemplateMemoryManager<T>>
    class RefPtr
    {
    public:
        using Allocator = Allocator_T;

        // Big Five----------------------------------------------------------------------------------------------------
        RefPtr()
            : m_ptr(nullptr)
        {
        }

        RefPtr(T* ref)
            : m_ptr(ref)
        {
            m_ptr->IncrementRef();
        }

        ~RefPtr()
        {
            if (m_ptr && !m_ptr->DecrementRef())
            {
                Allocator::Delete(m_ptr);
                m_ptr = nullptr;
            }
        }

        RefPtr(const RefPtr& other)
            : m_ptr(other.m_ptr)
        {
            if (m_ptr)
            {
                m_ptr->IncrementRef();
            }
        }

        RefPtr(RefPtr&& other) noexcept
            : m_ptr(other.m_ptr)
        {
            other.m_ptr = nullptr;
        }

        RefPtr& operator=(const RefPtr& other)
        {
            if (this == &other)
                return *this;
            this->~RefPtr();
            m_ptr = other.m_ptr;
            if (m_ptr)
            {
                m_ptr->IncrementRef();
            }
            return *this;
        }

        RefPtr& operator=(RefPtr&& other) noexcept
        {
            if (this == &other)
                return *this;
            this->~RefPtr();
            m_ptr = other.m_ptr;
            other.m_ptr = nullptr;
            return *this;
        }

        // Polymorphism Casting---------------------------------------------------------------------------------------------

        template <class U>
        RefPtr(const RefPtr<U>& other) noexcept
        {
            static_assert(std::is_same_v<T, U>, "RefPtr Polymorphism Casting not allowed!");
        }

        template <class U>
        RefPtr(RefPtr<U>&& other) noexcept
        {
            static_assert(std::is_same_v<T, U>, "RefPtr Polymorphism Casting not allowed!");
        }

        template <class U>
        RefPtr& operator=(const RefPtr<U>& other) noexcept
        {
            static_assert(std::is_same_v<T, U>, "RefPtr Polymorphism Casting not allowed!");
        }

        template <class U>
        RefPtr& operator=(RefPtr<U>&& other) noexcept
        {
            static_assert(std::is_same_v<T, U>, "RefPtr Polymorphism Casting not allowed!");
        }

        // Operator overloading---------------------------------------------------------------------------------------------

        T* operator->() noexcept
        {
            ASSERT(valid(), "RefPtr nullptr error!");
            return m_ptr;
        }

        T& operator*() noexcept
        {
            ASSERT(valid(), "RefPtr nullptr error!");
            return *m_ptr;
        }

        const T* operator->() const noexcept
        {
            ASSERT(valid(), "RefPtr nullptr error!");
            return m_ptr;
        }

        const T& operator*() const noexcept
        {
            ASSERT(valid(), "RefPtr nullptr error!");
            return *m_ptr;
        }

        // Methods---------------------------------------------------------------------------------------------

        [[nodiscard]] T* get(bool validate = true) noexcept
        {
            if (validate)
            {
                ASSERT(valid(), "RefPtr nullptr error!");
            }
            return m_ptr;
        }

        [[nodiscard]] const T* get(bool validate = true) const noexcept
        {
            if (validate)
            {
                ASSERT(valid(), "RefPtr nullptr error!");
            }
            return m_ptr;
        }

        void reset() noexcept
        {
            this->~RefPtr();
        }

        [[nodiscard]] bool valid() const noexcept
        {
            return m_ptr != nullptr;
        }


    private:
        T* m_ptr{nullptr};
    };
}

/*
    Custum hash function for RefPtr<T>
*/
namespace std
{
    template <class T>
    struct hash<RefPtr<T>>
    {
        std::size_t operator()(const RefPtr<T>& ptr) const noexcept
        {
            return hash<T*>()(ptr.get(false));
        }
    };
}
