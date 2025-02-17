#include <string>
#include "slice/slice.h"
#include "io_status.h"
#include <map>

namespace latte
{

    // struct IOOptions {

    // };

    

    //latte_leveldb
    class RandomAccessFile {
        public:
            RandomAccessFile() = default;

            RandomAccessFile(const RandomAccessFile&) = delete;



    };


    // //latte_rocksdb
    // class FSRandomAccessFile {
    //     public:
    //         FSRandomAccessFile() {}
    //         virtual ~FSRandomAccessFile() {}

    //         virtual IOStatus Read(uint64_t offset, size_t n, const IOOptions& options,
    //                     Slice* result, char* scratch,
    //                     IODebugContext* dbg) const = 0;
    // };
} // namespace latte



