


#ifndef __LATTE_C_PLUS_MEMORY_ALLOCATOR_H
#define __LATTE_C_PLUS_MEMORY_ALLOCATOR_H


#include "customizable/customizable.h"
#include "io/config_options.h"
#include <string>
#include "status/status.h"

namespace latte
{
    // namespace rocksdb
    // {
        // MemoryAllocator is an interface that a client can implement to supply custom
        // memory allocation and deallocation methods. See rocksdb/cache.h for more
        // information.
        // All methods should be thread-safe.
        class MemoryAllocator {
            public:
                static const char* Type() { return "MemoryAllocator"; }
                static Status CreateFromString(const ConfigOptions& options,
                                                const std::string& value,
                                                std::shared_ptr<MemoryAllocator>* result);

            // Allocate a block of at least size. Has to be thread-safe.
            virtual void* Allocate(size_t size) = 0;

            // Deallocate previously allocated block. Has to be thread-safe.
            virtual void Deallocate(void* p) = 0;

            // Returns the memory size of the block allocated at p. The default
            // implementation that just returns the original allocation_size is fine.
            virtual size_t UsableSize(void* /*p*/, size_t allocation_size) const {
                // default implementation just returns the allocation size
                return allocation_size;
            }

            // std::string GetId() const override { return GenerateIndividualId(); }
        };
    // } // namespace rocksdb
    
} // namespace latte



#endif