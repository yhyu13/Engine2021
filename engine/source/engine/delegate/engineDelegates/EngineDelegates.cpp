#include "engine-precompiled-header.h"
#include "EngineDelegates.h"

namespace longmarch
{
    namespace delegates
    {
#define DEFINE_DELEGATE(name, ...) Delegate<__VA_ARGS__> name = Delegate<__VA_ARGS__>()

        DEFINE_DELEGATE(WorkerThreadReportWait, uint32_t, uint32_t, double);
        DEFINE_DELEGATE(WorkerThreadReportExec, uint32_t, uint32_t, double);

#undef DEFINE_DELEGATE
    }
}
