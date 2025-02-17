#ifndef __LATTE_C_PLUS_FILE_SYSTEM_H
#define __LATTE_C_PLUS_FILE_SYSTEM_H


#pragma once
#include <string>
#include "slice/slice.h"
#include "io_status.h"
#include "options.h"
#include <map>
#include "customizable.h"
#include "io_debug_context.h"
#include "file_options.h"
#include "env/env.h"
#include "fs_sequential_file.h"
#include "./config_options.h"

namespace latte
{
    
    // DEPRECATED
    // Priority of an IO request. This is a hint and does not guarantee any
    // particular QoS.
    // IO_LOW - Typically background reads/writes such as compaction/flush
    // IO_HIGH - Typically user reads/synchronous WAL writes
    enum class IOPriority : uint8_t {
        kIOLow,
        kIOHigh,
        kIOTotal,
    };

    // Type of the data begin read/written. It can be passed down as a flag
    // for the FileSystem implementation to optionally handle different types in
    // different ways
    enum class IOType : uint8_t {
        kData,
        kFilter,
        kIndex,
        kMetadata,
        kWAL,
        kManifest,
        kLog,
        kUnknown,
        kInvalid,
    };

    

    // using FSAllocationPtr = std::unique_ptr<void, std::function<void(void*)>>;
    // struct FSReadRequest {
    //     uint64_t offset;

    //     size_t len;

    //     char* scratch;

    //     Slice result;

    //     IOStatus status;
        
    //     FSAllocationPtr fs_scratch;
    // };


    // //latte_rocksdb
    // class FSRandomAccessFile {
    //     public:
    //         FSRandomAccessFile() {}
    //         virtual ~FSRandomAccessFile() {}

    //         virtual IOStatus Read(uint64_t offset, size_t n, const IOOptions& options,
    //                     Slice* result, char* scratch,
    //                     IODebugContext* dbg) const = 0;
    // };

    


    // enum representing various operations supported by underlying FileSystem.
    // These need to be set in SupportedOps API for RocksDB to use them.
    enum FSSupportedOps {
        kAsyncIO,   // Supports async reads
        kFSBuffer,  // Supports handing off the file system allocated read buffer
                    // to the caller of Read/MultiRead
        kVerifyAndReconstructRead,  // Supports a higher level of data integrity. See
                                    // the verify_and_reconstruct_read flag in
                                    // IOOptions.
    };

    

    class FSWritableFile {
        public:
            FSWritableFile()
                : last_preallocated_block_(0),
                preallocation_block_size_(0),
                // io_priority_(Env::IO_TOTAL),
                // write_hint_(Env::WLTH_NOT_SET),
                strict_bytes_per_sync_(false) {}

            explicit FSWritableFile(const FileOptions& options)
                : last_preallocated_block_(0),
                    preallocation_block_size_(0),
                    io_priority_(Env::IO_TOTAL),
                    write_hint_(Env::WLTH_NOT_SET),
                    strict_bytes_per_sync_(options.strict_bytes_per_sync) {}
            /*
            * Get and set the default pre-allocation block size for writes to
            * this file.  If non-zero, then Allocate will be used to extend the
            * underlying storage of a file (generally via fallocate) if the Env
            * instance supports it.
            */
            virtual void SetPreallocationBlockSize(size_t size) {
                preallocation_block_size_ = size;
            }

            virtual size_t GetRequiredBufferAlignment() const { return kDefaultPageSize; }
            virtual bool use_direct_io() const { return false; }
        private:
            size_t last_preallocated_block_;
            size_t preallocation_block_size_;
            // No copying allowed
            FSWritableFile(const FSWritableFile&);
            void operator=(const FSWritableFile&);

        protected:
            Env::IOPriority io_priority_;
            Env::WriteLifeTimeHint write_hint_;
            const bool strict_bytes_per_sync_;
    };

    class FileSystem  : public Customizable {
        public:
            FileSystem();

            FileSystem(const FileSystem&) = delete;

            virtual ~FileSystem();

            static const char* Type() { return "FileSystem"; }

            static const char* kDefaultName() { return "DefaultFileSystem"; }

            static Status CreateFromString(const ConfigOptions& options,
                                        const std::string& value,
                                        std::shared_ptr<FileSystem>* result);
            
            static std::shared_ptr<FileSystem> Default();

            virtual Status RegisterDbPaths(const std::vector<std::string>& /*paths*/) {
                return Status::OK();
            }

            virtual Status UnregisterDbPaths(const std::vector<std::string>& /*path*/) {
                return Status::OK();
            }

            virtual IOStatus GetChildren(const std::string& dir, const IOOptions& options,
                               std::vector<std::string>* result,
                               IODebugContext* dbg) = 0;

            virtual IOStatus FileExists(const std::string& fname,
                              const IOOptions& options,
                              IODebugContext* dbg) = 0;
            virtual IOStatus DeleteFile(const std::string& fname,
                              const IOOptions& options,
                              IODebugContext* dbg) = 0;
            virtual FileOptions OptimizeForManifestWrite(
                const FileOptions& file_options) const;

            // 创建一个具有指定名称的全新顺序可读文件。
            // 成功时，将指向新文件的指针存储在 *result 中并返回 OK。
            // 失败时，将 nullptr 存储在 *result 中并返回非 OK。如果文件不存在，则返回非 OK 状态。
            //
            // 返回的文件一次只能由一个线程访问。
            virtual IOStatus NewSequentialFile(const std::string& fname,
                                                const FileOptions& file_opts,
                                                std::unique_ptr<FSSequentialFile>* result,
                                                IODebugContext* dbg) = 0;

            // 使用指定名称创建一个全新的随机访问只读文件。成功时，
            // 将指向新文件的指针存储在 *result 中并返回 OK。
            // 失败时将 nullptr 存储在 *result 中并返回非 OK。如果文件不存在，则返回非 OK 状态。 //
            // 返回的文件可能被多个线程同时访问。
            virtual IOStatus NewRandomAccessFile(
                const std::string& fname, const FileOptions& file_opts,
                std::unique_ptr<FSRandomAccessFile>* result, IODebugContext* dbg) = 0;

            // OptimizeForManifestRead 将创建一个新的 FileOptions 对象，该对象是参数中的 FileOptions 的副本
            //，但针对读取清单
            // 文件进行了优化。
            virtual FileOptions OptimizeForManifestRead(
                const FileOptions& file_options) const;

            virtual void SupportedOps(int64_t& supported_ops) {
                supported_ops = 0;
                supported_ops |= (1 << FSSupportedOps::kAsyncIO);
            }

            // If you're adding methods here, remember to add them to EnvWrapper too.

            private:
                void operator=(const FileSystem&);
    };
    
     // A utility routine: write "data" to the named file.
    IOStatus WriteStringToFile(FileSystem* fs, const Slice& data,
                            const std::string& fname, bool should_sync = false,
                            const IOOptions& io_options = IOOptions(),
                            const FileOptions& file_options = FileOptions());

    // A utility routine: read contents of named file into *data
    IOStatus ReadFileToString(FileSystem* fs, const std::string& fname,
                            std::string* data);

    // A utility routine: read contents of named file into *data
    IOStatus ReadFileToString(FileSystem* fs, const std::string& fname,
                                const IOOptions& opts, std::string* data);

    

    class FSWritableFileWrapper : public FSWritableFile {
        public:
        // Creates a FileWrapper around the input File object and without
        // taking ownership of the object
        explicit FSWritableFileWrapper(FSWritableFile* t) : target_(t) {}

        FSWritableFile* target() const { return target_; }

        // IOStatus Append(const Slice& data, const IOOptions& options,
        //                 IODebugContext* dbg) override {
        //     return target_->Append(data, options, dbg);
        // }
        // IOStatus Append(const Slice& data, const IOOptions& options,
        //                 const DataVerificationInfo& verification_info,
        //                 IODebugContext* dbg) override {
        //     return target_->Append(data, options, verification_info, dbg);
        // }
        // IOStatus PositionedAppend(const Slice& data, uint64_t offset,
        //                             const IOOptions& options,
        //                             IODebugContext* dbg) override {
        //     return target_->PositionedAppend(data, offset, options, dbg);
        // }
        // IOStatus PositionedAppend(const Slice& data, uint64_t offset,
        //                             const IOOptions& options,
        //                             const DataVerificationInfo& verification_info,
        //                             IODebugContext* dbg) override {
        //     return target_->PositionedAppend(data, offset, options, verification_info,
        //                                     dbg);
        // }
        // IOStatus Truncate(uint64_t size, const IOOptions& options,
        //                     IODebugContext* dbg) override {
        //     return target_->Truncate(size, options, dbg);
        // }
        // IOStatus Close(const IOOptions& options, IODebugContext* dbg) override {
        //     return target_->Close(options, dbg);
        // }
        // IOStatus Flush(const IOOptions& options, IODebugContext* dbg) override {
        //     return target_->Flush(options, dbg);
        // }
        // IOStatus Sync(const IOOptions& options, IODebugContext* dbg) override {
        //     return target_->Sync(options, dbg);
        // }
        // IOStatus Fsync(const IOOptions& options, IODebugContext* dbg) override {
        //     return target_->Fsync(options, dbg);
        // }
        // bool IsSyncThreadSafe() const override { return target_->IsSyncThreadSafe(); }

        bool use_direct_io() const override { return target_->use_direct_io(); }

        // size_t GetRequiredBufferAlignment() const override {
        //     return target_->GetRequiredBufferAlignment();
        // }

        // void SetWriteLifeTimeHint(Env::WriteLifeTimeHint hint) override {
        //     target_->SetWriteLifeTimeHint(hint);
        // }

        // Env::WriteLifeTimeHint GetWriteLifeTimeHint() override {
        //     return target_->GetWriteLifeTimeHint();
        // }

        // uint64_t GetFileSize(const IOOptions& options, IODebugContext* dbg) override {
        //     return target_->GetFileSize(options, dbg);
        // }

        // void SetPreallocationBlockSize(size_t size) override {
        //     target_->SetPreallocationBlockSize(size);
        // }

        // void GetPreallocationStatus(size_t* block_size,
        //                             size_t* last_allocated_block) override {
        //     target_->GetPreallocationStatus(block_size, last_allocated_block);
        // }

        // size_t GetUniqueId(char* id, size_t max_size) const override {
        //     return target_->GetUniqueId(id, max_size);
        // }

        // IOStatus InvalidateCache(size_t offset, size_t length) override {
        //     return target_->InvalidateCache(offset, length);
        // }

        // IOStatus RangeSync(uint64_t offset, uint64_t nbytes, const IOOptions& options,
        //                     IODebugContext* dbg) override {
        //     return target_->RangeSync(offset, nbytes, options, dbg);
        // }

        // void PrepareWrite(size_t offset, size_t len, const IOOptions& options,
        //                     IODebugContext* dbg) override {
        //     target_->PrepareWrite(offset, len, options, dbg);
        // }

        // IOStatus Allocate(uint64_t offset, uint64_t len, const IOOptions& options,
        //                     IODebugContext* dbg) override {
        //     return target_->Allocate(offset, len, options, dbg);
        // }

        private:
            FSWritableFile* target_;
    };

    class FSWritableFileOwnerWrapper : public FSWritableFileWrapper {
        public:
            // Creates a FileWrapper around the input File object and takes
            // ownership of the object
            explicit FSWritableFileOwnerWrapper(std::unique_ptr<FSWritableFile>&& t)
                : FSWritableFileWrapper(t.get()), guard_(std::move(t)) {}

        private:
            std::unique_ptr<FSWritableFile> guard_;
    };
} // namespace latte


#endif