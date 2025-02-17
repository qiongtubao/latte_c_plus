

#ifndef __LATTE_C_PLUS_ALIGNED_STORAGE_H
#define __LATTE_C_PLUS_ALIGNED_STORAGE_H

#include <string>
namespace latte
{
    template <typename T, std::size_t Align = alignof(T)>
            struct aligned_storage {
        struct type {
            alignas(Align) unsigned char data[sizeof(T)];
        };
    };
    
} // namespace latte


#endif