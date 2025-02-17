
#ifndef __LATTE_C_PLUS_IO_OPTIONS_H
#define __LATTE_C_PLUS_IO_OPTIONS_H

namespace latte
{
   
    // Per-request options that can be passed down to the FileSystem
    // implementation. These are hints and are not necessarily guaranteed to be
    // honored. More hints can be added here in the future to indicate things like
    // storage media (HDD/SSD) to be used, replication level etc.
    struct IOOptions {
        // Timeout for the operation in microseconds
        std::chrono::microseconds timeout;

        // DEPRECATED
        // Priority - high or low
        IOPriority prio;

        // Priority used to charge rate limiter configured in file system level (if
        // any)
        // Limitation: right now RocksDB internal does not consider this
        // rate_limiter_priority
        Env::IOPriority rate_limiter_priority;

        // Type of data being read/written
        // IOType type;

        // EXPERIMENTAL
        // An option map that's opaque to RocksDB. It can be used to implement a
        // custom contract between a FileSystem user and the provider. This is only
        // useful in cases where a RocksDB user directly uses the FileSystem or file
        // object for their own purposes, and wants to pass extra options to APIs
        // such as NewRandomAccessFile and NewWritableFile.
        std::unordered_map<std::string, std::string> property_bag;

        // Force directory fsync, some file systems like btrfs may skip directory
        // fsync, set this to force the fsync
        bool force_dir_fsync;

        // Can be used by underlying file systems to skip recursing through sub
        // directories and list only files in GetChildren API.
        bool do_not_recurse;

        // Setting this flag indicates a corruption was detected by a previous read,
        // so the caller wants to re-read the data with much stronger data integrity
        // checking and correction, i.e requests the file system to reconstruct the
        // data from redundant copies and verify checksums, if available, in order
        // to have a better chance of success. It is expected that this will have a
        // much higher overhead than a normal read.
        // This is a hint. At a minimum, the file system should implement this flag in
        // FSRandomAccessFile::Read and FSSequentialFile::Read
        // NOTE: The file system must support kVerifyAndReconstructRead in
        // FSSupportedOps, otherwise this feature will not be used.
        bool verify_and_reconstruct_read;

        // EXPERIMENTAL
        // Env::IOActivity io_activity = Env::IOActivity::kUnknown;

        IOOptions() : IOOptions(false) {}

        explicit IOOptions(bool force_dir_fsync_)
            : timeout(std::chrono::microseconds::zero()),
                prio(IOPriority::kIOLow),
                rate_limiter_priority(Env::IO_TOTAL),
                type(IOType::kUnknown),
                force_dir_fsync(force_dir_fsync_),
                do_not_recurse(false),
                verify_and_reconstruct_read(false) {}
    };

    struct DBOptions {
        DBOptions* OldDefaults(int rocksdb_major_version = 4,
                            int rocksdb_minor_version = 6);
        

        // Env* env = Env::Default();

    };
} // namespace latte


#endif