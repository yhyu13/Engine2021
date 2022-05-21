#pragma once
#include "MathUtil.h"

namespace longmarch
{
    template<class T, int Size = sizeof(T)>
    class LongMarch_EncryptValue
    {
    public:
        LongMarch_EncryptValue() = default;
        explicit LongMarch_EncryptValue(const T& value)
        {
            setValue(value);
        }
        explicit LongMarch_EncryptValue(const T& value, int key, bool is_value_encrypted)
        {
            m_Value = value;
            m_EncryptKey = key;
            if (!is_value_encrypted)
            {
                encrypt();
            }
        }

        T getValue()
        {
            return decrypt();
        }

        void setValue(const T& value)
        {
            m_Value = value;
            while (m_Value == value)
            {
                genRandKey();
                encrypt();
            }
        }

    private:
        void genRandKey() noexcept
        {
            m_EncryptKey = replacement::rand();
        }

        void encrypt()
        {
            if (m_EncryptKey == 0)
            {
                throw std::runtime_error("bad encryption key");
                return;
            }
            if constexpr (Size == 4)
            {
                *(reinterpret_cast<int*>(&m_Value)) ^= m_EncryptKey;
            }
            else if constexpr (Size == 2)
            {
                *(reinterpret_cast<short*>(&m_Value)) ^= m_EncryptKey;
            }
            else if constexpr (Size == 8)
            {
                *(reinterpret_cast<long long*>(&m_Value)) ^= m_EncryptKey;
            }
            else if constexpr (Size == 1)
            {
                *(reinterpret_cast<char*>(&m_Value)) ^= m_EncryptKey;
            }
            else
            {
                static_assert(Size == 1, "unhandled encrypt value type");
            }
        }

        T decrypt()
        {
            if (m_EncryptKey == 0)
            {
                throw std::runtime_error("bad encryption key");
                return T();
            }
            T ret = m_Value;
            if constexpr (Size == 4)
            {
                *(reinterpret_cast<int*>(&ret)) ^= m_EncryptKey;
            }
            else if constexpr (Size == 2)
            {
                *(reinterpret_cast<short*>(&ret)) ^= m_EncryptKey;
            }
            else if constexpr (Size == 8)
            {
                *(reinterpret_cast<long long*>(&ret)) ^= m_EncryptKey;
            }
            else if constexpr (Size == 1)
            {
                *(reinterpret_cast<char*>(&ret)) ^= m_EncryptKey;
            }
            else
            {
                static_assert(Size == 1, "unhandled encrypt value type");
            }
            return ret;
        }

    private:
        T m_Value;
        int m_EncryptKey;
    };
}
