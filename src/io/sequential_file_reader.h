



#ifndef __LATTE_C_PLUS_SEQUENTIAL_FILE_READER_H
#define __LATTE_C_PLUS_SEQUENTIAL_FILE_READER_H


#include <vector>
#include <atomic>
#include "listener/event_listener.h"
namespace latte
{
    namespace rocksdb
    {
        class SequentialFileReader {
            public:
                explicit SequentialFileReader(
                    std::unique_ptr<FSSequentialFile>&& _file, const std::string& _file_name,
                    const std::shared_ptr<IOTracer>& io_tracer = nullptr,
                    const std::vector<std::shared_ptr<EventListener>>& listeners = {},
                    RateLimiter* rate_limiter =
                        nullptr,  // TODO: migrate call sites to provide rate limiter
                    bool verify_and_reconstruct_read = false)
                    : file_name_(_file_name),
                        file_(std::move(_file), io_tracer, _file_name),
                        listeners_(),
                        rate_limiter_(rate_limiter),
                        verify_and_reconstruct_read_(verify_and_reconstruct_read) {
                    AddFileIOListeners(listeners);
                }

                explicit SequentialFileReader(
                    std::unique_ptr<FSSequentialFile>&& _file, const std::string& _file_name,
                    size_t _readahead_size,
                    const std::shared_ptr<IOTracer>& io_tracer = nullptr,
                    const std::vector<std::shared_ptr<EventListener>>& listeners = {},
                    RateLimiter* rate_limiter =
                        nullptr,  // TODO: migrate call sites to provide rate limiter
                    bool verify_and_reconstruct_read = false)
                    : file_name_(_file_name),
                        file_(NewReadaheadSequentialFile(std::move(_file), _readahead_size),
                            io_tracer, _file_name),
                        listeners_(),
                        rate_limiter_(rate_limiter),
                        verify_and_reconstruct_read_(verify_and_reconstruct_read) {
                    AddFileIOListeners(listeners);
                }

                void AddFileIOListeners(
                    const std::vector<std::shared_ptr<EventListener>>& listeners) {
                    std::for_each(listeners.begin(), listeners.end(),
                                [this](const std::shared_ptr<EventListener>& e) {
                        if (e->ShouldBeNotifiedOnFileIO()) {
                        listeners_.emplace_back(e);
                        }
                    });
                }

            public:
                std::string file_name_;
                FSSequentialFilePtr file_;
                std::atomic<size_t> offset_{0};  // read offset
                std::vector<std::shared_ptr<EventListener>> listeners_{};
                RateLimiter* rate_limiter_;
                bool verify_and_reconstruct_read_;

            private:
            // NewReadaheadSequentialFile 为 SequentialFile 提供了一个包装器，以便
            // 每次读取时始终预取额外的数据。   
            static std::unique_ptr<FSSequentialFile> NewReadaheadSequentialFile(
                std::unique_ptr<FSSequentialFile>&& file, size_t readahead_size);
               
        };
    } // namespace rocksdb
    
} // namespace latte




#endif