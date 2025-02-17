
#ifndef __LATTE_C_PLUS_IO_FILE_OPTIONS
#define __LATTE_C_PLUS_IO_FILE_OPTIONS


#include "env/env_options.h"
#include "types.h"

namespace latte
{
    // namespace rocksdb
    // {
        struct FileOptions : public EnvOptions {
            public:
            // Embedded IOOptions to control the parameters for any IOs that need
            // to be issued for the file open/creation
            IOOptions io_options;

            // EXPERIMENTAL
            // The feature is in development and is subject to change.
            // When creating a new file, set the temperature of the file so that
            // underlying file systems can put it with appropriate storage media and/or
            // coding.
            Temperature temperature = Temperature::kUnknown;

            // The checksum type that is used to calculate the checksum value for
            // handoff during file writes.
            ChecksumType handoff_checksum_type;

            FileOptions() : EnvOptions()
            , handoff_checksum_type(ChecksumType::kCRC32c) 
            {}

            // FileOptions(const DBOptions& opts)
            //     : EnvOptions(opts)
            //     , temperature(opts.metadata_write_temperature)
            //     // ,
            //     //     handoff_checksum_type(ChecksumType::kCRC32c) 
            //     {}

            FileOptions(const EnvOptions& opts)
                : EnvOptions(opts)
                , handoff_checksum_type(ChecksumType::kCRC32c) 
                {}

            FileOptions(const FileOptions& opts)
                : EnvOptions(opts),
                    io_options(opts.io_options)
                    , temperature(opts.temperature)
                    ,handoff_checksum_type(opts.handoff_checksum_type) 
                {}

            FileOptions& operator=(const FileOptions&) = default;
        };
    // } // namespace rocksdb
    
} // namespace latte

#endif