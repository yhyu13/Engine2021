#pragma once

#include "engine/core/EngineCore.h"

namespace longmarch
{
    template <typename ...__TRIVIAL__>
    void FOpenS(FILE*& file, const char* fileName, const char* mode)
    {
        errno_t err;
        if ((err = fopen_s(&file, fileName, mode)) != 0)
        {
            char buf[256];
            strerror_s(buf, sizeof(buf), err);
            ERROR_PRINT(Str("FOpenS : Cannot open file '%s': %s", fileName, buf));
        }
        else
        {
            ASSERT(file != nullptr, Str("FOpenS fails at : path '%s', mode '%s'", fileName, mode));
        }
    }
}
