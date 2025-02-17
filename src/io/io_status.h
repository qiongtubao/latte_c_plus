

#include "status/status.h"

namespace latte
{
    class IOStatus : public Status {
        public:
            using Code = Status::Code;
            using SubCode = Status::SubCode;

        enum IOErrorScope : unsigned char {
            kIOErrorScopeFileSystem,
            kIOErrorScopeFile,
            kIOErrorScopeRange,
            kIOErrorScopeMax,
        };

        // Create a success status.
        IOStatus() : IOStatus(kOk, kNone) {}
        ~IOStatus() {}

        static IOStatus OK() { return IOStatus(); }

        static IOStatus NotSupported(const Slice& msg, const Slice& msg2 = Slice()) {
            return IOStatus(kNotSupported, msg, msg2);
        }

        static IOStatus IOError(const Slice& msg, const Slice& msg2 = Slice()) {
            return IOStatus(kIOError, msg, msg2);
        }

        static IOStatus IOError(SubCode msg = kNone) {
            return IOStatus(kIOError, msg);
        }

        private:
            explicit IOStatus(Code _code, SubCode _subcode = kNone)
                : Status(_code, _subcode, false, false, kIOErrorScopeFileSystem) {}
            IOStatus(Code _code, SubCode _subcode, const Slice& msg, const Slice& msg2);
            IOStatus(Code _code, const Slice& msg, const Slice& msg2)
                : IOStatus(_code, kNone, msg, msg2) {}    
    };
} // namespace latte
