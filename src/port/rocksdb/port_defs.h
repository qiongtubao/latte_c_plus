

#ifndef __LATTE_C_PLUS_PORT_DEFS_H
#define __LATTE_C_PLUS_PORT_DEFS_H

namespace latte
{
    namespace port
    {
        class CondVar;   
        enum class CpuPriority {
            kIdle = 0,
            kLow = 1,
            kNormal = 2,
            kHigh = 3,
        };
    } // namespace port

    
    
} // namespace latte
#endif