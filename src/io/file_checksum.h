
#ifndef __LATTE_C_PLUS_IO_FILE_CHECKSUM_H
#define __LATTE_C_PLUS_IO_FILE_CHECKSUM_H

#include <string>

namespace latte
{
    class FileChecksumGenFactory {
        public:
            virtual std::unique_ptr<FileChecksumGenerator> CreateFileChecksumGenerator(
                const FileChecksumGenContext& context) = 0;
    };

    struct FileChecksumGenContext {
        std::string file_name;
        // The name of the requested checksum generator.
        // Checksum factories may use or ignore requested_checksum_func_name,
        // and checksum factories written before this field was available are still
        // compatible.
        std::string requested_checksum_func_name;
    };


    class FileChecksumGenerator {
        public:
            virtual ~FileChecksumGenerator() {}

            // Update the current result after process the data. For different checksum
            // functions, the temporal results may be stored and used in Update to
            // include the new data.
            virtual void Update(const char* data, size_t n) = 0;

            // Generate the final results if no further new data will be updated.
            virtual void Finalize() = 0;

            // Get the checksum. The result should not be the empty string and may
            // include arbitrary bytes, including non-printable characters.
            virtual std::string GetChecksum() const = 0;

            // Returns a name that identifies the current file checksum function.
            virtual const char* Name() const = 0;
    };
} // namespace latte


#endif

