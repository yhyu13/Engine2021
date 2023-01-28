#pragma once
#include "../core/utility/TypeHelper.h"
#include <functional>
#include <vector>
#include <set>

namespace longmarch
{
    template<typename... Args>
    class Delegate : public BaseAtomicClassNC
    {
    private:
        typedef void (*GlobalDelegateFunction) (Args... args);

        // Ignore binding instance member method to delegates as we need to explicitly handle pointer life time
        // template<class T>
        // typedef void (T::* InstanceHandlerFunction) (Args... args);

    public:
        NONCOPYABLE(Delegate);
        Delegate() = default;

        bool IsBoundAny() const
        {
            return !m_delegates.empty();
        }
    
        bool BindGlobal(GlobalDelegateFunction function)
        {
            LOCK_GUARD_NC();
            size_t mask = 0;
            LongMarch_HashCombine(mask, reinterpret_cast<uintptr_t>(&function));
            if (!LongMarch_SetEmplace(m_delegatesHashSet, mask))
            {
                return false;
            }
            m_delegates.push_back(function);
            return true;
        }

        // template<class T>
        // void BindInstance(T* instance, InstanceHandlerFunction<T> function)
        // {
        //     LOCK_GUARD_NC();
        //     size_t mask = 0;
        //     LongMarch_HashCombine(mask, reinterpret_cast<uintptr_t>(&function));
        //     LongMarch_HashCombine(mask, reinterpret_cast<uintptr_t>(instance));
        //     if (!LongMarch_SetEmplace(m_delegatesHashSet, mask))
        //     {
        //         return;
        //     }
        //     m_delegates.push_back(function);
        // }

        void InvokeAll(Args... args)
        {
            LOCK_GUARD_NC();
            for(auto& func : m_delegates)
            {
                func(args...);
            }
        }

    private:
        std::vector<std::function<void(Args... args)>> m_delegates;
        std::set<size_t> m_delegatesHashSet;
    };
}
