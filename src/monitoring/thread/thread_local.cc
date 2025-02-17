



#include "thread_local.h"

namespace latte
{

    void ThreadLocalPtr::InitSingletons() { ThreadLocalPtr::Instance(); }


} // namespace latte
