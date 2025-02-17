


#ifndef __LATTE_C_PLUS_WRITE_THREAD_H
#define __LATTE_C_PLUS_WRITE_THREAD_H

#include <cassert>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>
#include "status/status.h"
#include "../types.h"

namespace latte
{
    namespace leveldb
    {
        
    } // namespace leveldb
    
    namespace rocksdb
    {
        class WriteThread {
            public:
                
            
            struct Writer;

            struct WriteGroup {
                Writer* leader = nullptr;
                Writer* last_writer = nullptr;
                SequenceNumber last_sequence;
                // before running goes to zero, status needs leader->StateMutex()
                Status status;
                std::atomic<size_t> running;
                size_t size = 0;

                struct Iterator {
                    Writer* writer;
                    Writer* last_writer;

                    explicit Iterator(Writer* w, Writer* last)
                        : writer(w), last_writer(last) {}

                    Writer* operator*() const { return writer; }

                    Iterator& operator++() {
                        assert(writer != nullptr);
                        if (writer == last_writer) {
                            writer = nullptr;
                        } else {
                            writer = writer->link_newer;
                        }
                        return *this;
                    }

                    bool operator!=(const Iterator& other) const {
                        return writer != other.writer;
                    }
                };

                Iterator begin() const { return Iterator(leader, last_writer); }
                Iterator end() const { return Iterator(nullptr, nullptr); }
            };
        };
    } // namespace rocksdb
    
} // namespace latte



#endif