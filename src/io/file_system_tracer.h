

#ifndef __LATTE_C_PLUS_ENV_FILE_SYSTEM_TRACER_H
#define __LATTE_C_PLUS_ENV_FILE_SYSTEM_TRACER_H
#include <memory>
#include "file_system.h"
#include "file_system_tracing_wrapper.h"
#include "io_tracer.h"
#include "slice/slice.h"
namespace latte
{
    // A data structure brings the data verification information, which is
    // used together with data being written to a file.
    struct DataVerificationInfo {
        // checksum of the data being written.
        Slice checksum;
    };


    class FSWritableFileTracingWrapper : public FSWritableFileOwnerWrapper {
        public:
            FSWritableFileTracingWrapper(std::unique_ptr<FSWritableFile>&& t,
                                        std::shared_ptr<IOTracer> io_tracer,
                                        const std::string& file_name)
                : FSWritableFileOwnerWrapper(std::move(t)),
                    io_tracer_(io_tracer),
                    clock_(SystemClock::Default().get()),
                    file_name_(file_name) {}

            ~FSWritableFileTracingWrapper() override {}

            IOStatus Append(const Slice& data, const IOOptions& options,
                            IODebugContext* dbg) override;
            IOStatus Append(const Slice& data, const IOOptions& options,
                            const DataVerificationInfo& /*verification_info*/,
                            IODebugContext* dbg) override {
                return Append(data, options, dbg);
            }

            IOStatus PositionedAppend(const Slice& data, uint64_t offset,
                                        const IOOptions& options,
                                        IODebugContext* dbg) override;
            IOStatus PositionedAppend(const Slice& data, uint64_t offset,
                                        const IOOptions& options,
                                        const DataVerificationInfo& /*verification_info*/,
                                        IODebugContext* dbg) override {
                return PositionedAppend(data, offset, options, dbg);
            }

            IOStatus Truncate(uint64_t size, const IOOptions& options,
                                IODebugContext* dbg) override;

            IOStatus Close(const IOOptions& options, IODebugContext* dbg) override;

            uint64_t GetFileSize(const IOOptions& options, IODebugContext* dbg) override;

            IOStatus InvalidateCache(size_t offset, size_t length) override;

            private:
                std::shared_ptr<IOTracer> io_tracer_;
                SystemClock* clock_;
                // Stores file name instead of full path.
                std::string file_name_;
    };
    class FSWritableFilePtr {
        public:
            FSWritableFilePtr(std::unique_ptr<FSWritableFile>&& fs,
                                const std::shared_ptr<IOTracer>& io_tracer,
                                const std::string& file_name)
                : io_tracer_(io_tracer) {
                fs_tracer_.reset(new FSWritableFileTracingWrapper(
                    std::move(fs), io_tracer_,
                    file_name.substr(file_name.find_last_of("/\\") +
                                    1) /* pass file name */));
            }

            FSWritableFile* operator->() const {
                if (io_tracer_ && io_tracer_->is_tracing_enabled()) {
                    return fs_tracer_.get();
                } else {
                    return fs_tracer_->target();
                }
            }

            FSWritableFile* get() const {
                if (io_tracer_ && io_tracer_->is_tracing_enabled()) {
                    return fs_tracer_.get();
                } else if (fs_tracer_) {
                    return fs_tracer_->target();
                } else {
                    return nullptr;
                }
            }

            

            void reset() {
                fs_tracer_.reset();
                io_tracer_ = nullptr;
            }

            private:
                std::shared_ptr<IOTracer> io_tracer_;
                std::unique_ptr<FSWritableFileTracingWrapper> fs_tracer_;
    };
    // FileSystemPtr 是一个包装器类，它接受指向存储系统（例如 posix 文件系统）的指针。
    //它重载运算符 -> 并根据是否启用跟踪返回 FileSystem 或 FileSystemTracingWrapper 的指针。
    //添加它是为了在禁用跟踪时绕过 FileSystemTracingWrapper。
    class FileSystemPtr {
        public:
            // FileSystemPtr(std::shared_ptr<FileSystem> fs,
            // const std::shared_ptr<IOTracer>& io_tracer)
            //     : fs_(fs), io_tracer_(io_tracer) {
            //     fs_tracer_ = std::make_shared<FileSystemTracingWrapper>(fs_, io_tracer_);
            // }

            std::shared_ptr<FileSystem> operator->() const {
                if (io_tracer_ && io_tracer_->is_tracing_enabled()) {
                    return fs_tracer_;
                } else {
                    return fs_;
                }
            }

            // /* Returns the underlying File System pointer */
            FileSystem* get() const {
                if (io_tracer_ && io_tracer_->is_tracing_enabled()) {
                    return fs_tracer_.get();
                } else {
                    return fs_.get();
                }
            }

            // private:
                std::shared_ptr<FileSystem> fs_;
                std::shared_ptr<IOTracer> io_tracer_;
                std::shared_ptr<FileSystemTracingWrapper> fs_tracer_;
    };

    
} // namespace latte
#endif