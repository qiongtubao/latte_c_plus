
#ifndef __LATTE_C_PLUS_IO_TYPES_H
#define __LATTE_C_PLUS_IO_TYPES_H

#include "stdint.h"
namespace latte
{

    // Types of checksums to use for checking integrity of logical blocks within
    // files. All checksums currently use 32 bits of checking power (1 in 4B
    // chance of failing to detect random corruption). Traditionally, the actual
    // checking power can be far from ideal if the corruption is due to misplaced
    // data (e.g. physical blocks out of order in a file, or from another file),
    // which is fixed in format_version=6 (see below).
    enum ChecksumType : char {
        kNoChecksum = 0x0,
        kCRC32c = 0x1,
        kxxHash = 0x2,
        kxxHash64 = 0x3,
        kXXH3 = 0x4,  // Supported since RocksDB 6.27
    };
    // The types of files RocksDB uses in a DB directory. (Available for
    // advanced options.)
    enum FileType {
        kWalFile,
        kDBLockFile,
        kTableFile,
        kDescriptorFile,
        kCurrentFile,
        kTempFile,
        kInfoLogFile,  // Either the current one, or an old one
        kMetaDatabase,
        kIdentityFile,
        kOptionsFile,
        kBlobFile
    };
    // Temperature of a file. Used to pass to FileSystem for a different
    // placement and/or coding.
    // Reserve some numbers in the middle, in case we need to insert new tier
    // there.
    enum class Temperature : uint8_t {
        kUnknown = 0,
        kHot = 0x04,
        kWarm = 0x08,
        kCold = 0x0C,
        kLastTemperature,
    };
} // namespace latte


#endif
