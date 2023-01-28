#pragma once

#include "../Delegate.h"

namespace longmarch
{
    namespace delegates
    {
#define DECLARE_DELEGATE(name, ...) extern Delegate<__VA_ARGS__> name

        DECLARE_DELEGATE(WorkerThreadReportWait, uint32_t, uint32_t, double);
        DECLARE_DELEGATE(WorkerThreadReportExec, uint32_t, uint32_t, double);
        
#undef DECLARE_DELEGATE
    }
}
