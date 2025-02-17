



#ifndef __LATTE_C_PLUS_IO_FS_SEQUENTIAL_FILE_H
#define __LATTE_C_PLUS_IO_FS_SEQUENTIAL_FILE_H

namespace latte
{
   
    class FSSequentialFile {
        public:
            FSSequentialFile() {}

            virtual ~FSSequentialFile() {}

            // 从文件中读取最多“n”个字节。“scratch[0..n-1]”可能由
            // 此例程写入。将“*result”设置为已读取的数据
            //（包括成功读取的字节数少于“n”的情况）。
            // 可以将“*result”设置为指向“scratch[0..n-1]”中的数据，因此
            // 使用“*result”时，“scratch[0..n-1]”必须处于活动状态。
            // 如果遇到错误，则返回非 OK 状态。
            //
            // 调用后，仅当已到达文件末尾（或非 OK 状态）时，result->size() < n。如果在第一次 result->size() < n 之后再次调用，读取可能会失败。
            //
            // 需要：外部同步
            virtual IOStatus Read(size_t n, const IOOptions& options, Slice* result,
                                    char* scratch, IODebugContext* dbg) = 0;

            // 从文件中跳过“n”个字节。这保证不会比读取相同数据慢，但可能会更快。
            //
            // 如果到达文件末尾，跳过将在文件末尾停止，并且 Skip 将返回 OK。
            //
            // 需要：外部同步
            virtual IOStatus Skip(uint64_t n) = 0;

            // 指示上层当前 SequentialFile 实现是否使用直接 IO。
            virtual bool use_direct_io() const { return false; }

            // 使用返回的对齐值来分配
            // 用于直接 I/O 的对齐缓冲区
            virtual size_t GetRequiredBufferAlignment() const { return kDefaultPageSize; }

            // 删除从偏移量到偏移量+文件长度的任何类型的数据缓存。如果长度为 0，则表示文件末尾。
            // 如果系统未缓存文件内容，则这是一个无操作。
            virtual IOStatus InvalidateCache(size_t /*offset*/, size_t /*length*/) {
                return IOStatus::NotSupported("InvalidateCache not supported.");
            }

            // 定位直接 I/O 读取
            // 如果启用直接 I/O，偏移量、n 和划痕应正确对齐
            virtual IOStatus PositionedRead(uint64_t /*offset*/, size_t /*n*/,
                                            const IOOptions& /*options*/,
                                            Slice* /*result*/, char* /*scratch*/,
                                            IODebugContext* /*dbg*/) {
                return IOStatus::NotSupported("PositionedRead");
            }

            // 实验性
            // 可用时，返回文件的实际温度。在某些外部进程将文件从一个层移动到另一个层的情况下，这很有用，
            // 尽管通常预计文件打开时温度不会改变。
            virtual Temperature GetTemperature() const { return Temperature::kUnknown; }

            // 如果您在这里添加方法，请记住也将它们添加到
            // SequentialFileWrapper 中。
    };

    class FSSequentialFileWrapper : public FSSequentialFile {
        public:
        // Creates a FileWrapper around the input File object and without
        // taking ownership of the object
        explicit FSSequentialFileWrapper(FSSequentialFile* t) : target_(t) {}

        FSSequentialFile* target() const { return target_; }

        IOStatus Read(size_t n, const IOOptions& options, Slice* result,
                        char* scratch, IODebugContext* dbg) override {
            return target_->Read(n, options, result, scratch, dbg);
        }
        IOStatus Skip(uint64_t n) override { return target_->Skip(n); }
        bool use_direct_io() const override { return target_->use_direct_io(); }
        size_t GetRequiredBufferAlignment() const override {
            return target_->GetRequiredBufferAlignment();
        }
        IOStatus InvalidateCache(size_t offset, size_t length) override {
            return target_->InvalidateCache(offset, length);
        }
        IOStatus PositionedRead(uint64_t offset, size_t n, const IOOptions& options,
                                Slice* result, char* scratch,
                                IODebugContext* dbg) override {
            return target_->PositionedRead(offset, n, options, result, scratch, dbg);
        }
        Temperature GetTemperature() const override {
            return target_->GetTemperature();
        }

        private:
            FSSequentialFile* target_;
    };

    class FSSequentialFileOwnerWrapper : public FSSequentialFileWrapper {
        public:
        // Creates a FileWrapper around the input File object and takes
        // ownership of the object
        explicit FSSequentialFileOwnerWrapper(std::unique_ptr<FSSequentialFile>&& t)
            : FSSequentialFileWrapper(t.get()), guard_(std::move(t)) {}

        private:
        std::unique_ptr<FSSequentialFile> guard_;
    };

    // FSSequentialFileTracingWrapper is a wrapper class above FSSequentialFile that
    // forwards the call to the underlying storage system. It then invokes IOTracer
    // to record file operations and other contextual information in a binary format
    // for tracing. It overrides methods we are interested in tracing and extends
    // FSSequentialFileWrapper, which forwards all methods that are not explicitly
    // overridden.
    class FSSequentialFileTracingWrapper : public FSSequentialFileOwnerWrapper 
    {
        public:
            FSSequentialFileTracingWrapper(std::unique_ptr<FSSequentialFile>&& t,
                                            std::shared_ptr<IOTracer> io_tracer,
                                            const std::string& file_name)
                : FSSequentialFileOwnerWrapper(std::move(t)),
                    io_tracer_(io_tracer),
                    clock_(SystemClock::Default().get()),
                    file_name_(file_name) {}

        ~FSSequentialFileTracingWrapper() override {}

        IOStatus Read(size_t n, const IOOptions& options, Slice* result,
                        char* scratch, IODebugContext* dbg) override;

        IOStatus InvalidateCache(size_t offset, size_t length) override;

        IOStatus PositionedRead(uint64_t offset, size_t n, const IOOptions& options,
                                Slice* result, char* scratch,
                                IODebugContext* dbg) override;

        private:
        std::shared_ptr<IOTracer> io_tracer_;
        SystemClock* clock_;
        std::string file_name_;
    };
   
    // The FSSequentialFilePtr is a wrapper class that takes pointer to storage
    // systems (such as posix filesystems). It overloads operator -> and returns a
    // pointer of either FSSequentialFile or FSSequentialFileTracingWrapper based on
    // whether tracing is enabled or not. It is added to bypass
    // FSSequentialFileTracingWrapper when tracing is disabled.
    class FSSequentialFilePtr {
        public:
            FSSequentialFilePtr() = delete;
            FSSequentialFilePtr(std::unique_ptr<FSSequentialFile>&& fs,
                                const std::shared_ptr<IOTracer>& io_tracer,
                                const std::string& file_name)
                : io_tracer_(io_tracer),
                    fs_tracer_(std::move(fs), io_tracer_,
                            file_name.substr(file_name.find_last_of("/\\") +
                                                1) /* pass file name */) {}

            FSSequentialFile* operator->() const {
                if (io_tracer_ && io_tracer_->is_tracing_enabled()) {
                return const_cast<FSSequentialFileTracingWrapper*>(&fs_tracer_);
                } else {
                return fs_tracer_.target();
                }
            }

            FSSequentialFile* get() const {
                if (io_tracer_ && io_tracer_->is_tracing_enabled()) {
                return const_cast<FSSequentialFileTracingWrapper*>(&fs_tracer_);
                } else {
                return fs_tracer_.target();
                }
            }

        private:
            std::shared_ptr<IOTracer> io_tracer_;
            FSSequentialFileTracingWrapper fs_tracer_;
    };
    
} // namespace latte



#endif